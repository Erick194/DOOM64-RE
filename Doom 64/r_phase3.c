
//Renderer phase 3 - World Rendering Routines

#include "doomdef.h"
#include "r_local.h"

//-----------------------------------//
void R_RenderWorld(subsector_t *sub);

void R_WallPrep(seg_t *seg);
void R_RenderWall(seg_t *seg, int flags, int texture, int topHeight, int bottomHeight, int topOffset, int bottomOffset, int topColor, int bottomColor);
void R_RenderSwitch(seg_t *seg, int texture, int topOffset, int color);

void R_RenderPlane(leaf_t *leaf, int numverts, int zpos, int texture, int xpos, int ypos, int color);

void R_RenderThings(subsector_t *sub);
void R_RenderLaser(mobj_t *thing);
void R_RenderPSprites(void);
//-----------------------------------//

void R_RenderAll(void) // 80026590
{
    subsector_t *sub;

    while (endsubsector--, (endsubsector >= solidsubsectors))
    {
        sub = *endsubsector;
        frontsector = sub->sector;
        R_RenderWorld(sub);

        sub->drawindex = 0x7fff;
    }
}

void R_RenderWorld(subsector_t *sub) // 80026638
{
    leaf_t *lf;
    seg_t *seg;

    fixed_t xoffset;
    fixed_t yoffset;
    int numverts;
    int i;

    I_CheckGFX();

    gDPSetPrimColor(GFX1++, 0, frontsector->lightlevel, 0, 0, 0, 255);

    numverts = sub->numverts;

    /* */
    /* Render Walls */
    /* */
    lf = &leafs[sub->leaf];
    for (i = 0; i < numverts; i++)
    {
        seg = lf->seg;

        if (seg && (seg->flags & 1))
        {
            R_WallPrep(seg);
        }

        lf++;
    }

    /* */
    /* Render Ceilings */
    /* */
    if ((frontsector->ceilingpic != -1) && (viewz < frontsector->ceilingheight))
    {
        if (frontsector->flags & MS_SCROLLCEILING)
        {
            xoffset = frontsector->xoffset;
            yoffset = frontsector->yoffset;
        }
        else
        {
            xoffset = 0;
            yoffset = 0;
        }

        lf = &leafs[sub->leaf];
        R_RenderPlane(lf, numverts, frontsector->ceilingheight >> FRACBITS,
                        textures[frontsector->ceilingpic],
                        xoffset, yoffset,
                        lights[frontsector->colors[0]].rgba);
    }

    /* */
    /* Render Floors */
    /* */
    if ((frontsector->floorpic != -1) && (frontsector->floorheight < viewz))
    {
        if (!(frontsector->flags & MS_LIQUIDFLOOR))
        {
            if (frontsector->flags & MS_SCROLLFLOOR)
            {
                xoffset = frontsector->xoffset;
                yoffset = frontsector->yoffset;
            }
            else
            {
                xoffset = 0;
                yoffset = 0;
            }

            lf = &leafs[sub->leaf];
            R_RenderPlane(lf, numverts, frontsector->floorheight >> FRACBITS,
                            textures[frontsector->floorpic],
                            xoffset, yoffset,
                            lights[frontsector->colors[1]].rgba);
        }
        else
        {
            gDPPipeSync(GFX1++);
            gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_XLU_SURF2);

            if (frontsector->flags & MS_SCROLLFLOOR)
            {
                xoffset = frontsector->xoffset;
                yoffset = frontsector->yoffset;
            }
            else
            {
                xoffset = scrollfrac;
                yoffset = 0;
            }

            //--------------------------------------------------------------
            lf = &leafs[sub->leaf];
            R_RenderPlane(lf, numverts, frontsector->floorheight >> FRACBITS,
                            textures[frontsector->floorpic + 1],
                            xoffset, yoffset,
                            lights[frontsector->colors[1]].rgba);

            //--------------------------------------------------------------
            gDPSetPrimColor(GFX1++, 0, frontsector->lightlevel, 0, 0, 0, 160);

            lf = &leafs[sub->leaf];
            R_RenderPlane(lf, numverts, frontsector->floorheight >> FRACBITS,
                            textures[frontsector->floorpic],
                            -yoffset, xoffset,
                            lights[frontsector->colors[1]].rgba);

            gDPPipeSync(GFX1++);
            gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_TEX_EDGE2);
        }
    }

    /* */
    /* Render Things */
    /* */
    R_RenderThings(sub);
}

void R_WallPrep(seg_t *seg) // 80026A44
{
    sector_t *backsector;
	line_t *li;
	side_t *side;
	fixed_t f_ceilingheight;
	fixed_t f_floorheight;
	fixed_t b_ceilingheight;
	fixed_t b_floorheight;
	fixed_t m_top;
	fixed_t m_bottom;
	fixed_t height;
	fixed_t rowoffs;
	int pic;

	unsigned int height2;
	unsigned int r1, g1, b1;
	unsigned int r2, g2, b2;
	unsigned int thingcolor;
	unsigned int upcolor;
	unsigned int lowcolor;
	unsigned int topcolor;
	unsigned int bottomcolor;
	unsigned int tmp_upcolor;
	unsigned int tmp_lowcolor;

    li = seg->linedef;
    side = seg->sidedef;

    f_ceilingheight = frontsector->ceilingheight >> 16;
    f_floorheight = frontsector->floorheight >> 16;

    thingcolor = lights[frontsector->colors[2]].rgba;
    upcolor = lights[frontsector->colors[3]].rgba;
    lowcolor = lights[frontsector->colors[4]].rgba;

    if (li->flags & ML_BLENDING)
    {
        r1 = upcolor  >> 24;
        g1 = upcolor  >> 16 & 0xff;
        b1 = upcolor  >> 8 & 0xff;
        r2 = lowcolor >> 24;
        g2 = lowcolor >> 16 & 0xff;
        b2 = lowcolor >> 8 & 0xff;

        tmp_upcolor = upcolor;
        tmp_lowcolor = lowcolor;
    }
    else
    {
        topcolor = thingcolor;
        bottomcolor = thingcolor;
    }

    m_bottom = f_floorheight; // set middle bottom
    m_top = f_ceilingheight;  // set middle top

    backsector = seg->backsector;
    if (backsector)
    {
        b_floorheight = backsector->floorheight >> 16;
        b_ceilingheight = backsector->ceilingheight >> 16;

        if ((backsector->ceilingheight < frontsector->ceilingheight) && (backsector->ceilingpic != -1))
        {
            if (li->flags & ML_DONTPEGTOP)
            {
                height = (f_ceilingheight - b_ceilingheight);
                rowoffs = (side->rowoffset >> 16) + height;
            }
            else
            {
                height = (f_ceilingheight - b_ceilingheight);
                rowoffs = (height + 127 & -128) + (side->rowoffset >> 16);
            }

            if (li->flags & ML_BLENDING)
            {
                if (!(li->flags & ML_BLENDFULLTOP))
                {
                    if (f_floorheight < f_ceilingheight)
                    {
                        height2 = ((height << 16) / (f_ceilingheight - f_floorheight));
                    }
                    else
                    {
                        height2 = 0;
                    }

                    tmp_lowcolor = (((((r2 - r1) * height2) >> 16) + r1) << 24) |
                                   (((((g2 - g1) * height2) >> 16) + g1) << 16) |
                                   (((((b2 - b1) * height2) >> 16) + b1) << 8)  | 0xff;
                }

                if (li->flags & ML_INVERSEBLEND)
                {
                    bottomcolor = tmp_upcolor;
                    topcolor = tmp_lowcolor;
                }
                else
                {
                    topcolor = tmp_upcolor;
                    bottomcolor = tmp_lowcolor;
                }

                // clip middle color upper
                upcolor = tmp_lowcolor;
            }

            R_RenderWall(seg, li->flags, textures[side->toptexture],
                         f_ceilingheight, b_ceilingheight,
                         rowoffs - height, rowoffs,
                         topcolor, bottomcolor);

            m_top = b_ceilingheight; // clip middle top height
            if ((li->flags & (ML_CHECKFLOORHEIGHT|ML_SWITCHX08)) == ML_SWITCHX08)
            {
                if (SWITCHMASK(li->flags) == ML_SWITCHX04)
                {
                    pic = side->bottomtexture;
                    rowoffs = side->rowoffset >> 16;
                }
                else
                {
                    pic = side->midtexture;
                    rowoffs = side->rowoffset >> 16;
                }

                R_RenderSwitch(seg, pic, b_ceilingheight + rowoffs + 48, thingcolor);
            }
        }

        if (frontsector->floorheight < backsector->floorheight)
        {
            height = (f_ceilingheight - b_floorheight);

            if ((li->flags & ML_DONTPEGBOTTOM) == 0)
            {
                rowoffs = side->rowoffset >> 16;
            }
            else
            {
                rowoffs = height + (side->rowoffset >> 16);
            }

            if (li->flags & ML_BLENDING)
            {
                if (!(li->flags & ML_BLENDFULLBOTTOM))
                {
                    if (f_floorheight < f_ceilingheight)
                    {
                        height2 = ((height << 16) / (f_ceilingheight - f_floorheight));
                    }
                    else
                    {
                        height2 = 0;
                    }

                    tmp_upcolor = (((((r2 - r1) * height2) >> 16) + r1) << 24) |
                                  (((((g2 - g1) * height2) >> 16) + g1) << 16) |
                                  (((((b2 - b1) * height2) >> 16) + b1) << 8)  | 0xff;
                }

                topcolor = tmp_upcolor;
                bottomcolor = lowcolor;

                // clip middle color lower
                lowcolor = tmp_upcolor;
            }

            R_RenderWall(seg, li->flags, textures[side->bottomtexture],
                         b_floorheight, f_floorheight,
                         rowoffs, rowoffs + (b_floorheight - f_floorheight),
                         topcolor, bottomcolor);

            m_bottom = b_floorheight; // clip middle bottom height
            if ((li->flags & (ML_CHECKFLOORHEIGHT|ML_SWITCHX08)) == ML_CHECKFLOORHEIGHT)
            {
                if (SWITCHMASK(li->flags) == ML_SWITCHX02)
                {
                    pic = side->toptexture;
                    rowoffs = side->rowoffset >> 16;
                }
                else
                {
                    pic = side->midtexture;
                    rowoffs = side->rowoffset >> 16;
                }

                R_RenderSwitch(seg, pic, b_floorheight + rowoffs - 16, thingcolor);
            }
        }

        if (!(li->flags & ML_DRAWMASKED))
        {
            return;
        }
    }

    if (li->flags & ML_DONTPEGBOTTOM)
    {
        height = m_top - m_bottom;
        rowoffs = (height + 127 & -128) + (side->rowoffset >> 16);
    }
    else if (li->flags & ML_DONTPEGTOP)
    {
        rowoffs = (side->rowoffset >> 16) - m_bottom;
        height = m_top - m_bottom;
    }
    else
    {
        height = m_top - m_bottom;
        rowoffs = (side->rowoffset >> 16) + height;
    }

    if (li->flags & ML_BLENDING)
    {
        topcolor = upcolor;
        bottomcolor = lowcolor;
    }

    R_RenderWall(seg, li->flags, textures[side->midtexture],
                 m_top, m_bottom,
                 rowoffs - height, rowoffs,
                 topcolor, bottomcolor);

    if ((li->flags & (ML_CHECKFLOORHEIGHT|ML_SWITCHX08)) == (ML_CHECKFLOORHEIGHT|ML_SWITCHX08))
    {
        if (SWITCHMASK(li->flags) == ML_SWITCHX02)
        {
            pic = side->toptexture;
            rowoffs = side->rowoffset >> 16;
        }
        else
        {
            pic = side->bottomtexture;
            rowoffs = side->rowoffset >> 16;
        }

        R_RenderSwitch(seg, pic, m_bottom + rowoffs + 48, thingcolor);
    }
}

void R_RenderWall(seg_t *seg, int flags, int texture, int topHeight, int bottomHeight,
                  int topOffset, int bottomOffset, int topColor, int bottomColor) // 80027138
{
    byte *data;
    byte *src;
    vertex_t *v1;
    vertex_t *v2;
    int cms, cmt;
    int wshift, hshift;

    if (texture != 16)
    {
        if (flags & ML_HMIRROR) {
            cms = G_TX_MIRROR;
        }
        else {
            cms = G_TX_NOMIRROR;
        }

        if (flags & ML_VMIRROR) {
            cmt = G_TX_MIRROR;
        }
        else {
            cmt = G_TX_NOMIRROR;
        }

        if ((texture != globallump) || (globalcm != (cms | cmt)))
        {
            /*
            In Doom 64 all textures are compressed with the second method (dec_d64),
            in the original line it was declared that if a texture was not stored,
            it would be stored from the DOOM64.WAD and decompressed with the Jaguar Doom
            method (dec_jag) which is wrong since all the textures are previously
            loaded from the P_Init function with the second decompression method (dec_d64)
            */
            //data = W_CacheLumpNum(texture >> 4, PU_CACHE, dec_jag); // error decomp mode
            data = W_CacheLumpNum(texture >> 4, PU_CACHE, dec_d64); /* [GEC] FIXED */

            wshift = ((textureN64_t*)data)->wshift;
            hshift = ((textureN64_t*)data)->hshift;

            src = data + sizeof(textureN64_t);

            // Load Image Data
            gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
            gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

            gDPLoadSync(GFX1++);
            gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0,
                         (((1 << wshift) * (1 << hshift)) >> 2) - 1, 0);

            gDPPipeSync(GFX1++);
            gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_4b,
                       (((1 << wshift) >> 1) + 7) >> 3, 0, G_TX_RENDERTILE, 0,
                       cmt, hshift, 0,
                       cms, wshift, 0);

            gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0,
                           ((1 << wshift) - 1) << 2,
                           ((1 << hshift) - 1) << 2);

            // Load Palette Data
            gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1,
                               src + (1 << ((wshift + hshift + 31) & 31)) + ((texture & 15) << 5));

            gDPTileSync(GFX1++);
            gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

            gDPLoadSync(GFX1++);
            gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 15);

            gDPPipeSync(GFX1++);

            globallump = texture;
            globalcm = (cms | cmt);
        }

        gSPTexture(GFX1++, (1024 << 6)-1, (1024 << 6)-1, 0, G_TX_RENDERTILE, G_ON);

        gSPVertex(GFX1++, VTX1, 4, 0);
        gSP1Quadrangle(GFX1++, 0, 1, 2, 3, 1);

        v1 = seg->v1;
        v2 = seg->v2;

        // x coordinates
        VTX1[0].v.ob[0] = VTX1[3].v.ob[0] = (signed short)(v1->x >> 16);
        VTX1[1].v.ob[0] = VTX1[2].v.ob[0] = (signed short)(v2->x >> 16);

        // y coordinates
        VTX1[0].v.ob[1] = VTX1[1].v.ob[1] = topHeight;
        VTX1[3].v.ob[1] = VTX1[2].v.ob[1] = bottomHeight;

        // z coordinates
        VTX1[0].v.ob[2] = VTX1[3].v.ob[2] = (signed short)-(v1->y >> 16);
        VTX1[1].v.ob[2] = VTX1[2].v.ob[2] = (signed short)-(v2->y >> 16);

        // texture s coordinates
        VTX1[0].v.tc[0] = VTX1[3].v.tc[0] = ((seg->sidedef->textureoffset + seg->offset) >> 11);
        VTX1[1].v.tc[0] = VTX1[2].v.tc[0] = VTX1[0].v.tc[0] + (seg->length << 1);

        // texture t coordinates
        VTX1[0].v.tc[1] = VTX1[1].v.tc[1] = (topOffset << 5);
        VTX1[2].v.tc[1] = VTX1[3].v.tc[1] = (bottomOffset << 5);

        // vertex color
        *(int*)VTX1[0].v.cn = *(int*)VTX1[1].v.cn = topColor;
        *(int*)VTX1[2].v.cn = *(int*)VTX1[3].v.cn = bottomColor;

        VTX1 += 4;
    }
}

void R_RenderSwitch(seg_t *seg, int texture, int topOffset, int color) // 80027654
{
    byte *data;
    byte *src;
    vertex_t *v1;
    vertex_t *v2;
    fixed_t x, y;
    fixed_t sin, cos;
    int wshift, hshift;

    if (texture != globallump)
    {
        /*
        In Doom 64 all textures are compressed with the second method (dec_d64),
        in the original line it was declared that if a texture was not stored,
        it would be stored from the DOOM64.WAD and decompressed with the Jaguar Doom
        method (dec_jag) which is wrong since all the textures are previously
        loaded from the P_Init function with the second decompression method (dec_d64)
        */
        //data = W_CacheLumpNum(firsttex + texture, PU_CACHE, dec_jag); // error decomp mode
        data = W_CacheLumpNum(firsttex + texture, PU_CACHE, dec_d64); /* [GEC] FIXED */

        wshift = ((textureN64_t*)data)->wshift;
        hshift = ((textureN64_t*)data)->hshift;

        src = data + sizeof(textureN64_t);

        // Load Image Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0,
                         (((1 << wshift) * (1 << hshift)) >> 2) - 1, 0);

        gDPPipeSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_4b,
                       (((1 << wshift) >> 1) + 7) >> 3, 0, G_TX_RENDERTILE, 0,
                       0, 0, 0,
                       0, 0, 0);

        gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0,
                           ((1 << wshift) - 1) << 2,
                           ((1 << hshift) - 1) << 2);

        // Load Palette Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1,
                            src + (1 << ((wshift + hshift + 31) & 31)));

        gDPTileSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 15);

        gDPPipeSync(GFX1++);

        globallump = texture;
    }

    gSPTexture(GFX1++, (512 << 6), (512 << 6), 0, 0, 1);

    gSPVertex(GFX1++, VTX1, 4, 0);
    gSP1Quadrangle(GFX1++, 0, 1, 2, 3, 1);

    v1 = seg->linedef->v1;
    v2 = seg->linedef->v2;

    x = (v1->x + v2->x);
    if (x < 0) {x = x + 1;}

    y = (v1->y + v2->y);
    if (y < 0) {y = y + 1;}

    x >>= 1;
    y >>= 1;

    cos = finecosine[seg->angle >> ANGLETOFINESHIFT] << 1;
    sin = finesine  [seg->angle >> ANGLETOFINESHIFT] << 1;

    // x coordinates
    VTX1[0].v.ob[0] = VTX1[3].v.ob[0] = ((x) - (cos << 3) + sin) >> 16;
    VTX1[1].v.ob[0] = VTX1[2].v.ob[0] = ((x) + (cos << 3) + sin) >> 16;

    // y coordinates
    VTX1[0].v.ob[1] = VTX1[1].v.ob[1] = topOffset;
    VTX1[3].v.ob[1] = VTX1[2].v.ob[1] = topOffset - 32;

    // z coordinates
    VTX1[0].v.ob[2] = VTX1[3].v.ob[2] = ((-y) + (sin << 3) + cos) >> 16;
    VTX1[1].v.ob[2] = VTX1[2].v.ob[2] = ((-y) - (sin << 3) + cos) >> 16;

    // texture s coordinates
    VTX1[0].v.tc[0] = VTX1[3].v.tc[0] = (0 << 6);
    VTX1[1].v.tc[0] = VTX1[2].v.tc[0] = (32 << 6);

    // texture t coordinates
    VTX1[0].v.tc[1] = VTX1[1].v.tc[1] = (0 << 6);
    VTX1[2].v.tc[1] = VTX1[3].v.tc[1] = (32 << 6);

    // vertex color
    *(int*)VTX1[0].v.cn = *(int*)VTX1[1].v.cn = *(int*)VTX1[2].v.cn = *(int*)VTX1[3].v.cn = color;

    VTX1 += 4;
}

void R_RenderPlane(leaf_t *leaf, int numverts, int zpos, int texture, int xpos, int ypos, int color) // 80027B68
{
    byte *data;
    byte *src;
    vertex_t *vrt;
    fixed_t x;
    fixed_t y;
    int idx, i;
    int v00, v01, v02;

    if (texture != globallump)
    {
        /*
        In Doom 64 all textures are compressed with the second method (dec_d64),
        in the original line it was declared that if a texture was not stored,
        it would be stored from the DOOM64.WAD and decompressed with the Jaguar Doom
        method (dec_jag) which is wrong since all the textures are previously
        loaded from the P_Init function with the second decompression method (dec_d64)
        */
        //data = W_CacheLumpNum(texture >> 4, PU_CACHE, dec_jag); // error decomp mode
        data = W_CacheLumpNum(texture >> 4, PU_CACHE, dec_d64); /* [GEC] FIXED */

        src = data + sizeof(textureN64_t);

        // Load Image Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0,
                         (((1 << 6) * (1 << 6)) >> 2) - 1, 0);

        gDPPipeSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_4b,
                       (((1 << 6) >> 1) + 7) >> 3, 0, G_TX_RENDERTILE, 0,
                       0, 6, 0,
                       0, 6, 0);

        gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0,
                           ((1 << 6) - 1) << 2,
                           ((1 << 6) - 1) << 2);

        // Load Palette Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1,
                               src + (1 << (6 + 6 - 1)) + ((texture & 15) << 5));

        gDPTileSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 15);

        gDPPipeSync(GFX1++);

        globallump = texture;
    }

    vrt = leaf->vertex;
    VTX1[0].v.ob[0] = (vrt->x >> 16);
    VTX1[0].v.ob[1] = zpos;
    VTX1[0].v.ob[2] =-(vrt->y >> 16);
    VTX1[0].v.tc[0] = (((vrt->x + xpos & 0x3f0000U) >> 16) << 5);
    VTX1[0].v.tc[1] =-(((vrt->y + ypos & 0x3f0000U) >> 16) << 5);
    *(int *)VTX1[0].v.cn = color;

    x = ((vrt->x + xpos) >> 16) & -64;
    y = ((vrt->y + ypos) >> 16) & -64;

    gSPTexture(GFX1++, (1024 << 6)-1, (1024 << 6)-1, 0, G_TX_RENDERTILE, G_ON);

    if (numverts >= 32)
        numverts = 32;

    gSPVertex(GFX1++, VTX1, numverts, 0);
    VTX1++;

    if (numverts & 1)
    {
        idx = 2;
        gSP1Triangle(GFX1++, 0, 1, 2, 0);
    }
    else
    {
        idx = 1;
    }

    leaf++;
    numverts--;

    if (idx < numverts)
    {
        v00 = idx + 0;
        v01 = idx + 1;
        v02 = idx + 2;
        do
        {
            gSP2Triangles(GFX1++,
                          v00, v01, v02, 0, // 0, 1, 2
                          v00, v02, 0, 0);  // 0, 2, 0

            v00 += 2;
            v01 += 2;
            v02 += 2;
        } while (v02 < (numverts + 2));
    }

    /*i = 0;
    if (numverts > 0)
    {
        if ((numverts & 3))
        {
            while(i != (numverts & 3))
            {
                vrt = leaf->vertex;
                VTX1[0].v.ob[0] = (vrt->x >> 16);
                VTX1[0].v.ob[1] = zpos;
                VTX1[0].v.ob[2] =-(vrt->y >> 16);
                VTX1[0].v.tc[0] = ((((vrt->x + xpos) >> 16) - x) << 5);
                VTX1[0].v.tc[1] =-((((vrt->y + ypos) >> 16) - y) << 5);
                *(int *)VTX1[0].v.cn = color;
                VTX1++;
                leaf++;
                i++;
            }
        }

        while(i != numverts)
        {
            vrt = leaf->vertex;
            VTX1[0].v.ob[0] = (vrt->x >> 16);
            VTX1[0].v.ob[1] = zpos;
            VTX1[0].v.ob[2] =-(vrt->y >> 16);
            VTX1[0].v.tc[0] = ((((vrt->x + xpos) >> 16) - x) << 5);
            VTX1[0].v.tc[1] =-((((vrt->y + ypos) >> 16) - y) << 5);
            *(int *)VTX1[0].v.cn = color;
            VTX1++;
            leaf++;

            vrt = leaf->vertex;
            VTX1[0].v.ob[0] = (vrt->x >> 16);
            VTX1[0].v.ob[1] = zpos;
            VTX1[0].v.ob[2] =-(vrt->y >> 16);
            VTX1[0].v.tc[0] = ((((vrt->x + xpos) >> 16) - x) << 5);
            VTX1[0].v.tc[1] =-((((vrt->y + ypos) >> 16) - y) << 5);
            *(int *)VTX1[0].v.cn = color;
            VTX1++;
            leaf++;

            vrt = leaf->vertex;
            VTX1[0].v.ob[0] = (vrt->x >> 16);
            VTX1[0].v.ob[1] = zpos;
            VTX1[0].v.ob[2] =-(vrt->y >> 16);
            VTX1[0].v.tc[0] = ((((vrt->x + xpos) >> 16) - x) << 5);
            VTX1[0].v.tc[1] =-((((vrt->y + ypos) >> 16) - y) << 5);
            *(int *)VTX1[0].v.cn = color;
            VTX1++;
            leaf++;

            vrt = leaf->vertex;
            VTX1[0].v.ob[0] = (vrt->x >> 16);
            VTX1[0].v.ob[1] = zpos;
            VTX1[0].v.ob[2] =-(vrt->y >> 16);
            VTX1[0].v.tc[0] = ((((vrt->x + xpos) >> 16) - x) << 5);
            VTX1[0].v.tc[1] =-((((vrt->y + ypos) >> 16) - y) << 5);
            *(int *)VTX1[0].v.cn = color;
            VTX1++;
            leaf++;

            i += 4;
        }
    }*/

    for(i = 0; i < numverts; i++)
    {
        vrt = leaf->vertex;
        VTX1[0].v.ob[0] = (vrt->x >> 16);
        VTX1[0].v.ob[1] = zpos;
        VTX1[0].v.ob[2] =-(vrt->y >> 16);
        VTX1[0].v.tc[0] = ((((vrt->x + xpos) >> 16) - x) << 5);
        VTX1[0].v.tc[1] =-((((vrt->y + ypos) >> 16) - y) << 5);
        *(int *)VTX1[0].v.cn = color;
        VTX1++;
        leaf++;
    }
}

void R_RenderThings(subsector_t *sub) // 80028248
{
    byte *data;
    byte *src;
    byte *paldata;
    vissprite_t *vissprite_p;

    mobj_t *thing;
    boolean flip;
    int lump;

    int compressed;
    int tileh;
    int tilew;
    int height;
    int width;
    int tiles;
    int color;

    fixed_t xx, yy;
    int xpos1, xpos2;
    int ypos;
    int zpos1, zpos2;
    int spos, tpos;
    int v00, v01, v02, v03;

    vissprite_p = sub->vissprite;
    if (vissprite_p)
    {
        gDPPipeSync(GFX1++);

        if (vissprite_p->thing->flags & MF_RENDERLASER)
        {
            gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_RA_OPA_SURF2);
            gDPSetCombineMode(GFX1++, G_CC_D64COMB15, G_CC_D64COMB16);

            do
            {
                I_CheckGFX();
                R_RenderLaser(vissprite_p->thing);

                vissprite_p = vissprite_p->next;
                if(vissprite_p == NULL) {
                    break;
                }

            } while(vissprite_p->thing->flags & MF_RENDERLASER);

            gDPPipeSync(GFX1++);
            gDPSetCombineMode(GFX1++, G_CC_D64COMB07, G_CC_D64COMB08);

            if (vissprite_p == NULL)
            {
                gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_TEX_EDGE2);
                return;
            }
        }

        gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_XLU_SURF2_CLAMP);

        while (vissprite_p)
        {
            I_CheckGFX();

            thing = vissprite_p->thing;
            lump = vissprite_p->lump;
            flip = vissprite_p->flip;

            if (thing->frame & FF_FULLBRIGHT)
            {
                color = PACKRGBA(255, 255, 255, 255);//0xffffffff;
            }
            else
            {
                color = lights[vissprite_p->sector->colors[2]].rgba;
            }

            gDPSetPrimColorD64(GFX1++, 0, vissprite_p->sector->lightlevel, thing->alpha);

            data = W_CacheLumpNum(lump, PU_CACHE, dec_jag);

            compressed = ((spriteN64_t*)data)->compressed;
            tileh = ((spriteN64_t*)data)->tileheight;
            width = ((spriteN64_t*)data)->width;
            height = ((spriteN64_t*)data)->height;
            tiles = ((spriteN64_t*)data)->tiles << 1;

            spos = width;
            tpos = 0;

            src = data + sizeof(spriteN64_t);

            if (flip)
            {
                xx = thing->x + (((spriteN64_t*)data)->xoffs * viewsin);
                xpos1 = (xx - (width * viewsin)) >> 16;
                xpos2 = (xx) >> 16;

                yy = thing->y - (((spriteN64_t*)data)->xoffs * viewcos);
                zpos1 = -(yy + (width * viewcos)) >> 16;
                zpos2 = -(yy) >> 16;
            }
            else
            {
                xx = thing->x - (((spriteN64_t*)data)->xoffs * viewsin);
                xpos2 = (xx + (width * viewsin)) >> 16;
                xpos1 = (xx) >> 16;

                yy = thing->y + (((spriteN64_t*)data)->xoffs * viewcos);
                zpos2 = -(yy - (width * viewcos)) >> 16;
                zpos1 = -(yy) >> 16;
            }

            gSPTexture(GFX1++, (512 << 6), (512 << 6), 0, 0, 1);

            gSPVertex(GFX1++, VTX1, (tiles+2), 0);

            if (compressed < 0)
            {
                width = ((spriteN64_t*)data)->width + 7 & ~7;
                tilew = tileh * width;

                if (((spriteN64_t*)data)->cmpsize & 1)
                {
                    paldata = W_CacheLumpNum((lump - (((spriteN64_t*)data)->cmpsize >> 1)) +
                                             thing->info->palette, PU_CACHE, dec_jag) + 8;
                }
                else
                {
                    paldata = (src + ((spriteN64_t*)data)->cmpsize);
                }

                // Load Palette Data (256 colors)
                gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1, paldata);

                gDPTileSync(GFX1++);
                gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

                gDPLoadSync(GFX1++);
                gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 255);

                gDPPipeSync(GFX1++);
            }
            else
            {
                width = ((spriteN64_t*)data)->width + 15 & ~15;
                tilew = tileh * width;

                if (tilew < 0) {
                    tilew = tilew + 1;
                }

                tilew >>= 1;

                // Load Palette Data (16 colors)
                gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1, (src + ((spriteN64_t*)data)->cmpsize));

                gDPTileSync(GFX1++);
                gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

                gDPLoadSync(GFX1++);
                gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 15);

                gDPPipeSync(GFX1++);
            }

            ypos = (thing->z >> 16) + ((spriteN64_t*)data)->yoffs;

            VTX1[0].v.ob[0] = xpos1;
            VTX1[0].v.ob[1] = ypos;
            VTX1[0].v.ob[2] = zpos1;

            VTX1[1].v.ob[0] = xpos2;
            VTX1[1].v.ob[1] = ypos;
            VTX1[1].v.ob[2] = zpos2;

            VTX1[flip].v.tc[0] = 0;
            VTX1[flip^1].v.tc[0] = (spos << 6);

            VTX1[0].v.tc[1] = VTX1[1].v.tc[1] = (tpos << 6);

            *(int *)VTX1[0].v.cn = *(int *)VTX1[1].v.cn = color;
            VTX1 += 2;

            v03 = 0;
            v00 = 1;
            v01 = 3;
            v02 = 2;

            if (tiles > 0)
            {
                do
                {
                    if (compressed < 0)
                    {
                        // Load Image Data (8bit)
                        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
                        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

                        gDPLoadSync(GFX1++);
                        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, (tilew >> 1) - 1, 0);

                        gDPPipeSync(GFX1++);
                        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b, (width >> 3), 0,
                                   G_TX_RENDERTILE , 0, 0, 0, 0, 0, 0, 0);

                        gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, tpos << 2, ((width - 1) << 2), (tpos + tileh - 1) << 2);
                    }
                    else
                    {
                        // Load Image Data (4bit)
                        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
                        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

                        gDPLoadSync(GFX1++);
                        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, (tilew >> 1) - 1, 0);

                        gDPPipeSync(GFX1++);
                        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_4b, (width >> 4), 0,
                                   G_TX_RENDERTILE , 0, 0, 0, 0, 0, 0, 0);

                        gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, tpos << 2, ((width - 1) << 2), (tpos + tileh - 1) << 2);
                    }

                    tpos += tileh;
                    ypos -= tileh;

                    gSP2Triangles(GFX1++, v00, v01, v02, 0, // 1, 3, 2
                                          v00, v02, v03, 0); // 1, 2, 0

                    VTX1[0].v.ob[0] = xpos1;
                    VTX1[0].v.ob[1] = ypos;
                    VTX1[0].v.ob[2] = zpos1;

                    VTX1[1].v.ob[0] = xpos2;
                    VTX1[1].v.ob[1] = ypos;
                    VTX1[1].v.ob[2] = zpos2;

                    VTX1[flip].v.tc[0] = 0;
                    VTX1[flip^1].v.tc[0] = (spos << 6);

                    VTX1[0].v.tc[1] = VTX1[1].v.tc[1] = (tpos << 6);

                    *(int *)VTX1[0].v.cn = *(int *)VTX1[1].v.cn = color;
                    VTX1 += 2;

                    src += tilew;

                    height -= tileh;
                    if (height < tileh) {
                        tileh = height;
                    }

                    v00 += 2;
                    v01 += 2;
                    v02 += 2;
                    v03 += 2;
                } while (v02 < (tiles+2));
            }

            vissprite_p = vissprite_p->next;
        }

        globallump = -1;

        gDPPipeSync(GFX1++);
        gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_TEX_EDGE2);
    }
}

void R_RenderLaser(mobj_t *thing) // 80028CCC
{
    laserdata_t *laserdata;

    laserdata = (laserdata_t *)thing->extradata;

    gSPVertex(GFX1++, (VTX1), 6, 0);

    gSP2Triangles(GFX1++, 0, 2, 3, 1/*flag1*/,
						  0, 1, 2, 2/*flag2*/);

    gSP2Triangles(GFX1++, 0, 3, 5, 2/*flag1*/,
						  3, 4, 5, 2/*flag2*/);

    VTX1[0].v.ob[0] = (laserdata->x1 >> 16);
    VTX1[0].v.ob[1] = (laserdata->z1 >> 16);
    VTX1[0].v.ob[2] = -(laserdata->y1 >> 16);

    VTX1[1].v.ob[0] = ((laserdata->x1 - laserdata->slopey) >> 16);
    VTX1[1].v.ob[1] = (laserdata->z1 >> 16);
    VTX1[1].v.ob[2] = (-(laserdata->y1 + laserdata->slopex) >> 16);

    VTX1[2].v.ob[0] = ((laserdata->x2 - laserdata->slopey) >> 16);
    VTX1[2].v.ob[1] = (laserdata->z2 >> 16);
    VTX1[2].v.ob[2] = (-(laserdata->y2 + laserdata->slopex) >> 16);

    VTX1[3].v.ob[0] = (laserdata->x2 >> 16);
    VTX1[3].v.ob[1] = (laserdata->z2 >> 16);
    VTX1[3].v.ob[2] = -(laserdata->y2 >> 16);

    VTX1[4].v.ob[0] = ((laserdata->x2 + laserdata->slopey) >> 16);
    VTX1[4].v.ob[1] = (laserdata->z2 >> 16);
    VTX1[4].v.ob[2] = (-(laserdata->y2 - laserdata->slopex) >> 16);

    VTX1[5].v.ob[0] = ((laserdata->x1 + laserdata->slopey) >> 16);
    VTX1[5].v.ob[1] = (laserdata->z1 >> 16);
    VTX1[5].v.ob[2] = (-(laserdata->y1 - laserdata->slopex) >> 16);

    *(int *)VTX1[0].v.cn = PACKRGBA(255,0,0,255); // 0xff0000ff;
    *(int *)VTX1[1].v.cn = PACKRGBA(0,0,0,255);   // 0xff;
    *(int *)VTX1[2].v.cn = PACKRGBA(0,0,0,255);   // 0xff;
    *(int *)VTX1[3].v.cn = PACKRGBA(255,0,0,255); // 0xff0000ff;
    *(int *)VTX1[4].v.cn = PACKRGBA(0,0,0,255);   // 0xff;
    *(int *)VTX1[5].v.cn = PACKRGBA(0,0,0,255);   // 0xff;

    VTX1 += 6;
}

void R_RenderPSprites(void) // 80028f20
{
    int				i;
	pspdef_t		*psp, *psptmp;
	state_t			*state;
	spritedef_t		*sprdef;
	spriteframe_t	*sprframe;
	int				lump;
	int				flagtranslucent;

	boolean         palloaded;
	byte		    *data;
	byte		    *paldata;
	byte		    *src;

	int             tilecnt;
	int             tiles;
	int             tileh;
	int             tilew;
	int             width;
	int             height;
	int             width2;
	int             yh;
	int             x, y;

	I_CheckGFX();

	gDPPipeSync(GFX1++);
	gDPSetTexturePersp(GFX1++, G_TP_NONE);
	gDPSetCombineMode(GFX1++, G_CC_D64COMB17, G_CC_D64COMB18);

    psp = &viewplayer->psprites[0];

    flagtranslucent = (viewplayer->mo->flags & MF_SHADOW) != 0;

    psptmp = psp;
    for (i = 0; i < NUMPSPRITES; i++, psptmp++)
    {
        if(flagtranslucent || ((psptmp->state != 0) && (psptmp->alpha < 255)))
        {
            gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_XLU_SURF2_CLAMP);
            break;
        }
    }

    palloaded = false;

    for (i = 0; i < NUMPSPRITES; i++, psp++)
	{
		if ((state = psp->state) != 0) /* a null state means not active */
		{
			sprdef = &sprites[state->sprite];
			sprframe = &sprdef->spriteframes[state->frame & FF_FRAMEMASK];
			lump = sprframe->lump[0];

            data = W_CacheLumpNum(lump, PU_CACHE, dec_jag);

            tilecnt = 0;
            tiles = ((spriteN64_t*)data)->tiles;
            width = ((spriteN64_t*)data)->width;
            tileh = ((spriteN64_t*)data)->tileheight;
            width2 = width + 7 & ~7;
            tilew = tileh * width2;
            height = ((spriteN64_t*)data)->height;
            src = data + sizeof(spriteN64_t);

            x = (((psp->sx >> 16) - ((spriteN64_t*)data)->xoffs) + 160) << 2;
            y = (((psp->sy >> 16) - ((spriteN64_t*)data)->yoffs) + 239) << 2;
            if (viewplayer->onground)
            {
                x += (quakeviewx >> 20);
                y += (quakeviewy >> 14);
            }

			if (psp->state->frame & FF_FULLBRIGHT)
			{
			    gDPSetPrimColorD64(GFX1, 0, 0, PACKRGBA(255,255,255,0));//0xffffff00
			}
			else
			{
			    gDPSetPrimColorD64(GFX1, 0, frontsector->lightlevel,
                          lights[frontsector->colors[2]].rgba & ~255); // remove alpha value
			}

			// apply alpha value
			if (flagtranslucent)
			{
				GFX1->words.w1 |= 144;
			}
			else
            {
                GFX1->words.w1 |= psp->alpha;
            }
            GFX1++; // continue to next GFX1

            if (!palloaded)
            {
                palloaded = true;

                if (((spriteN64_t*)data)->cmpsize & 1)
                {
                /* Loads the palette from the first frame of the animation,  */
                /* which uses an odd number to get to the lump */
                    paldata = W_CacheLumpNum((lump - (((spriteN64_t*)data)->cmpsize >> 1)),
                                             PU_CACHE, dec_jag);

                    paldata += (((spriteN64_t*)paldata)->cmpsize + sizeof(spriteN64_t));
                }
                else
                {
                /* Loads the palette if it is included in the image data */
                    paldata = (src + ((spriteN64_t*)data)->cmpsize);
                }

                /* Load Palette Data (256 colors) */
                gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1, paldata);

                gDPTileSync(GFX1++);
                gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

                gDPLoadSync(GFX1++);
                gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 255);

                gDPPipeSync(GFX1++);
            }

            if (tiles > 0)
            {
                do
                {
                    /* Load Image Data (8bit) */
                    gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
                    gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

                    gDPLoadSync(GFX1++);
                    gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, ((width2 * tileh + 1) >> 1) - 1, 0);

                    gDPPipeSync(GFX1++);
                    gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b, ((width2 + 7) >> 3), 0,
                                   G_TX_RENDERTILE , 0, 0, 0, 0, 0, 0, 0);

                    gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0, ((width2 - 1) << 2), (tileh - 1) << 2);

                    yh = (tileh << 2) + y;

                    gSPTextureRectangle(GFX1++,
                                    x, y,
                                    (width << 2) + x, yh,
                                    0,
                                    0, 0,
                                    (1 << 10), (1 << 10));

                    height -= tileh;
                    if (height < tileh) {
                        tileh = height;
                    }

                    y = yh;

                    src += tilew;
                    tilecnt += 1;
                } while (tilecnt != (int)tiles);
            }
		}
	}
}
