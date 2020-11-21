/* P_main.c */

#include "doomdef.h"
#include "p_local.h"

// text for debug
#include "graph.h"

int			numvertexes;	//80077E44|uGp00000a34
vertex_t	*vertexes;		//8007800C|puGp00000bfc

int			numsegs;		//80077ECC
seg_t		*segs;			//8007805C

int			numsectors;		//80077D80
sector_t	*sectors;		//80077ED0

int			numsubsectors;	//80078048
subsector_t	*subsectors;	//80077D6C

int			numnodes;		//80077FE0
node_t		*nodes;			//80077CD0

int			numlines;		//80077FF0
line_t		*lines;			//80077CDC

int			numsides;		//80077FDC
side_t		*sides;			//80077CCC

int			numleafs;		//80077D90
leaf_t		*leafs;			//80077F34

int         numlights;      // 800A5EFC
light_t     *lights;        // 800A5E9C
maplights_t *maplights;     // 800A5EA4

int         nummacros;      // 800A5F00
macro_t     **macros;       // 800A5EA0

short		*blockmaplump;			//80077EEC /* offsets in blockmap are from here */
short		*blockmap;
int			bmapwidth, bmapheight;	/* in mapblocks */ //800780A8, 80077CE4
fixed_t		bmaporgx, bmaporgy;		/* origin of block map */ //80077FB4,80077FBC
mobj_t		**blocklinks;			/* for thing chains */ //80077D08

byte		*rejectmatrix;			/* for fast sight rejection */

mapthing_t  *spawnlist;     // 800A5D74
int         spawncount;     // 800A5D78

//mapthing_t	deathmatchstarts[10], *deathmatch_p;//80097e4c, 80077E8C
//mapthing_t	playerstarts[MAXPLAYERS];//800a8c60

/*
=================
=
= P_LoadVertexes
=
=================
*/

void P_LoadVertexes (void) // 8001CF20
{
	int			i, lumpSize;
	mapvertex_t	*ml;
	vertex_t	*li;

	numvertexes = W_MapLumpLength(ML_VERTEXES) / sizeof(mapvertex_t);
	vertexes = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0);
	D_memset (vertexes, 0, numvertexes*sizeof(vertex_t));

	ml = (mapvertex_t *)W_GetMapLump(ML_VERTEXES);
	li = vertexes;
	for (i=0 ; i<numvertexes ; i++, li++, ml++)
	{
		li->x = LONGSWAP(ml->x);
		li->y = LONGSWAP(ml->y);
		//li->validcount = 0;

		//PRINTF_D2(WHITE, 0, 25, "vertexes(%i,%i,%i)     \n", li->x>>16, li->y>>16,li->validcount);
		//WAIT();
	}
}

/*
=================
=
= P_LoadSegs
=
=================
*/

void P_LoadSegs (void) // 8001D020
{
	int			i;
	mapseg_t	*ml;
	seg_t		*li;
	line_t		*ldef;
	int			linedef, side;
    float       x, y;

	numsegs = W_MapLumpLength(ML_SEGS) / sizeof(mapseg_t);
	segs = Z_Malloc (numsegs*sizeof(seg_t),PU_LEVEL,0);
	D_memset (segs, 0, numsegs*sizeof(seg_t));

	ml = (mapseg_t *)W_GetMapLump(ML_SEGS);
	li = segs;
	for (i=0 ; i<numsegs ; i++, li++, ml++)
	{
		li->v1 = &vertexes[LITTLESHORT(ml->v1)];
		li->v2 = &vertexes[LITTLESHORT(ml->v2)];

		li->angle = (BIGSHORT(ml->angle)) << FRACBITS;
		li->offset = (LITTLESHORT(ml->offset)) << FRACBITS;

		linedef = LITTLESHORT(ml->linedef);
		ldef = &lines[linedef];
		li->linedef = ldef;

		side = LITTLESHORT(ml->side);
		li->sidedef = &sides[ldef->sidenum[side]];

		li->frontsector = sides[ldef->sidenum[side]].sector;

		if (ldef-> flags & ML_TWOSIDED)
			li->backsector = sides[ldef->sidenum[side^1]].sector;
		else
			li->backsector = 0;

		if (ldef->v1 == li->v1)
			ldef->fineangle = li->angle >> ANGLETOFINESHIFT;

        x = (float) ((double) (li->v2->x - li->v1->x) / 65536.0);
        y = (float) ((double) (li->v2->y - li->v1->y) / 65536.0);

        li->length = (short)((int)((double)sqrtf((x * x) + (y * y)) * 16.0));

        //PRINTF_D2(WHITE, 0, 25, "segs(length %i)     ", li->length);
		//WAIT();
	}
}

/*
=================
=
= P_LoadSubSectors
=
=================
*/

void P_LoadSubSectors (void) // 8001D34C
{
	int				i;
	mapsubsector_t	*ms;
	subsector_t		*ss;

	numsubsectors = W_MapLumpLength (ML_SSECTORS) / sizeof(mapsubsector_t);
	subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t),PU_LEVEL,0);
	D_memset (subsectors, 0, numsubsectors*sizeof(subsector_t));

	ms = (mapsubsector_t *)W_GetMapLump(ML_SSECTORS);
	ss = subsectors;
	for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
	{
		ss->numlines = LITTLESHORT(ms->numsegs);
		ss->firstline = LITTLESHORT(ms->firstseg);

		//ss->numverts = 0;
        //ss->leaf = 0;
        //ss->drawindex = 0;
	}
}


/*
=================
=
= P_LoadSectors
=
=================
*/

void P_LoadSectors (void) // 8001D43C
{
	int				i;
	mapsector_t		*ms;
	sector_t		*ss;
	int				skyname;

	skytexture = 0;
	skyname = W_GetNumForName("F_SKYA") - firsttex;

	numsectors = W_MapLumpLength(ML_SECTORS) / sizeof(mapsector_t);
	sectors = Z_Malloc (numsectors*sizeof(sector_t),PU_LEVEL,0);
	D_memset (sectors, 0, numsectors*sizeof(sector_t));

	ms = (mapsector_t *)W_GetMapLump(ML_SECTORS);
	ss = sectors;
	for (i=0 ; i<numsectors ; i++, ss++, ms++)
	{
		ss->floorheight = LITTLESHORT(ms->floorheight)<<FRACBITS;
		ss->ceilingheight = LITTLESHORT(ms->ceilingheight)<<FRACBITS;
		ss->floorpic = LITTLESHORT(ms->floorpic);
        ss->ceilingpic = LITTLESHORT(ms->ceilingpic);

		ss->colors[0] = LITTLESHORT(ms->colors[1]);
		ss->colors[1] = LITTLESHORT(ms->colors[0]);
		ss->colors[2] = LITTLESHORT(ms->colors[2]);
		ss->colors[3] = LITTLESHORT(ms->colors[3]);
		ss->colors[4] = LITTLESHORT(ms->colors[4]);

		ss->special = LITTLESHORT(ms->special);
		ss->thinglist = NULL;
		ss->tag = LITTLESHORT(ms->tag);
		ss->flags = LITTLESHORT(ms->flags);

        if (skyname <= ss->ceilingpic)
        {
            skytexture = (ss->ceilingpic - skyname) + 1;
            ss->ceilingpic = -1;
        }
        if (skyname <= ss->floorpic)
            ss->floorpic = -1;

	}
}

/*
=================
=
= P_LoadNodes
=
=================
*/

void P_LoadNodes (void) // 8001D64C
{
	int			i,j,k;
	mapnode_t	*mn;
	node_t		*no;

	numnodes = W_MapLumpLength(ML_NODES) / sizeof(mapnode_t);
	nodes = Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);
	D_memset (nodes, 0, numnodes*sizeof(node_t));

	mn = (mapnode_t *)W_GetMapLump(ML_NODES);
	no = nodes;
	for (i=0 ; i<numnodes ; i++, no++, mn++)
	{
		no->line.x = LITTLESHORT(mn->x) << FRACBITS;
		no->line.y = LITTLESHORT(mn->y) << FRACBITS;
		no->line.dx = LITTLESHORT(mn->dx) << FRACBITS;
		no->line.dy = LITTLESHORT(mn->dy) << FRACBITS;
		for (j=0 ; j<2 ; j++)
		{
			no->children[j] = (unsigned short)LITTLESHORT(mn->children[j]);
			for (k=0 ; k<4 ; k++)
				no->bbox[j][k] = LITTLESHORT(mn->bbox[j][k]) << FRACBITS;
		}
	}
}

/*
=================
=
= P_LoadThings
=
=================
*/

void P_LoadThings (void) // 8001D864
{
	int				i;
	mapthing_t		*mt, *mts;
	int				numthings;
	int				spawncnt;

	numthings = W_MapLumpLength(ML_THINGS) / sizeof(mapthing_t);

	mts = (mapthing_t *)W_GetMapLump(ML_THINGS);
	for (i=0, spawncnt=0; i<numthings ; i++, mts++)
    {
        if(LITTLESHORT(mts->options) & MTF_SPAWN)
        {
            spawncnt++;
        }
    }

    if (spawncnt != 0)
        spawnlist = (mapthing_t *)Z_Malloc(spawncnt * sizeof(mapthing_t),PU_LEVEL,0);

    mt = (mapthing_t *)W_GetMapLump(ML_THINGS);
	for (i=0 ; i<numthings ; i++, mt++)
	{
		mt->x = LITTLESHORT(mt->x);
		mt->y = LITTLESHORT(mt->y);
		mt->z = LITTLESHORT(mt->z);
		mt->angle = LITTLESHORT(mt->angle);
		mt->type = LITTLESHORT(mt->type);
		mt->options = LITTLESHORT(mt->options);
		mt->tid = LITTLESHORT(mt->tid);
		P_SpawnMapThing (mt);

		if (mt->type >= 4096)
			I_Error("P_LoadThings: doomednum:%d >= 4096", mt->type);
	}
}

/*
=================
=
= P_LoadLineDefs
=
= Also counts secret lines for intermissions
=================
*/

void P_LoadLineDefs (void) // 8001D9B8
{
	int				i;
	maplinedef_t	*mld;
	line_t			*ld;
	vertex_t		*v1, *v2;
	unsigned int    special;

	numlines = W_MapLumpLength(ML_LINEDEFS) / sizeof(maplinedef_t);
	lines = Z_Malloc (numlines*sizeof(line_t),PU_LEVEL,0);
	D_memset (lines, 0, numlines*sizeof(line_t));

	mld = (maplinedef_t *)W_GetMapLump(ML_LINEDEFS);
	ld = lines;
	for (i=0 ; i<numlines ; i++, mld++, ld++)
	{
		ld->flags = LONGSWAP(mld->flags);
		ld->special = BIGSHORT(mld->special);
		ld->tag = LITTLESHORT(mld->tag);

		v1 = ld->v1 = &vertexes[LITTLESHORT(mld->v1)];
		v2 = ld->v2 = &vertexes[LITTLESHORT(mld->v2)];

		ld->dx = (v2->x - v1->x);
		ld->dy = (v2->y - v1->y);

		if (!ld->dx)
			ld->slopetype = ST_VERTICAL;
		else if (!ld->dy)
			ld->slopetype = ST_HORIZONTAL;
		else
		{
			if (FixedDiv (ld->dy , ld->dx) > 0)
				ld->slopetype = ST_POSITIVE;
			else
				ld->slopetype = ST_NEGATIVE;
		}

		if (v1->x < v2->x)
		{
			ld->bbox[BOXLEFT] = v1->x;
			ld->bbox[BOXRIGHT] = v2->x;
		}
		else
		{
			ld->bbox[BOXLEFT] = v2->x;
			ld->bbox[BOXRIGHT] = v1->x;
		}

		if (v1->y < v2->y)
		{
			ld->bbox[BOXBOTTOM] = v1->y;
			ld->bbox[BOXTOP] = v2->y;
		}
		else
		{
			ld->bbox[BOXBOTTOM] = v2->y;
			ld->bbox[BOXTOP] = v1->y;
		}

		ld->sidenum[0] = LITTLESHORT(mld->sidenum[0]);
		ld->sidenum[1] = LITTLESHORT(mld->sidenum[1]);

		if (ld->sidenum[0] != -1)
			ld->frontsector = sides[ld->sidenum[0]].sector;
		else
			ld->frontsector = 0;

		if (ld->sidenum[1] != -1)
			ld->backsector = sides[ld->sidenum[1]].sector;
		else
			ld->backsector = 0;

        special = SPECIALMASK(ld->special);

        if(special >= 256)
        {
            if(special >= (nummacros + 256))
            {
                I_Error("P_LoadLineDefs: linedef %d has unknown macro", i);
            }
        }
	}
}

/*
=================
=
= P_LoadSideDefs
=
=================
*/

void P_LoadSideDefs (void) // 8001DCC8
{
	int				i;
	mapsidedef_t	*msd;
	side_t			*sd;

	numsides = W_MapLumpLength(ML_SIDEDEFS) / sizeof(mapsidedef_t);
	sides = Z_Malloc (numsides*sizeof(side_t),PU_LEVEL,0);
	D_memset (sides, 0, numsides*sizeof(side_t));

	msd = (mapsidedef_t *)W_GetMapLump(ML_SIDEDEFS);
	sd = sides;
	for (i=0 ; i<numsides ; i++, msd++, sd++)
	{
		sd->textureoffset = LITTLESHORT(msd->textureoffset)<<FRACBITS;
		sd->rowoffset = LITTLESHORT(msd->rowoffset)<<FRACBITS;
		sd->sector = &sectors[LITTLESHORT(msd->sector)];

		sd->toptexture = LITTLESHORT(msd->toptexture);
		sd->midtexture = LITTLESHORT(msd->midtexture);
		sd->bottomtexture = LITTLESHORT(msd->bottomtexture);
	}
}

/*
=================
=
= P_LoadBlockMap
=
=================
*/

void P_LoadBlockMap (void) // 8001DE38
{
	int		count;
	int		i;
	int     length;
	byte    *src;

	length = W_MapLumpLength(ML_BLOCKMAP);
	blockmaplump = Z_Malloc(length, PU_LEVEL, 0);
	src = (byte *)W_GetMapLump(ML_BLOCKMAP);
	D_memcpy(blockmaplump,src,length);

	blockmap = blockmaplump+4;//skip blockmap header
	count = length/2;
	for (i=0 ; i<count ; i++)
		blockmaplump[i] = LITTLESHORT(blockmaplump[i]);

	bmapwidth = blockmaplump[2];
	bmapheight = blockmaplump[3];
	bmaporgx = blockmaplump[0]<<FRACBITS;
	bmaporgy = blockmaplump[1]<<FRACBITS;

	/* clear out mobj chains */
	count = sizeof(*blocklinks)* bmapwidth*bmapheight;
	blocklinks = Z_Malloc (count,PU_LEVEL, 0);
	D_memset (blocklinks, 0, count);
}

/*
=================
=
= P_LoadReject
= Include On Psx Doom / Doom64
=
=================
*/

void P_LoadReject(void) // 8001DF98
{
	int     length;
	byte    *src;

	length = W_MapLumpLength(ML_REJECT);
	rejectmatrix = (byte*)Z_Malloc(length, PU_LEVEL, NULL);

    src = (byte *)W_GetMapLump(ML_REJECT);
    D_memcpy(rejectmatrix,src,length);
}

/*
=================
=
= P_LoadLeafs
= Exclusive Psx Doom / Doom64
=
=================
*/

void P_LoadLeafs(void) // 8001DFF8
{
	int         i, j;
	int         length, size, count;
	int         vertex, seg;
	subsector_t *ss;
	leaf_t      *lf;
	byte		*data;
	short       *mlf;

    data = W_GetMapLump(ML_LEAFS);

    size = 0;
    count = 0;
    mlf = (short *)data;
    length = W_MapLumpLength(ML_LEAFS);
    while (mlf < (short *)(data + length))
    {
        count += 1;
        size += (int)LITTLESHORT(*mlf);
        mlf += (int)(LITTLESHORT(*mlf) << 1) + 1;
    }

    if (count != numsubsectors)
		I_Error("P_LoadLeafs: leaf/subsector inconsistancy\n");

	leafs = Z_Malloc(size * sizeof(leaf_t), PU_LEVEL, 0);

    lf = leafs;
    ss = subsectors;

	numleafs = 0;
    mlf = (short *)data;
	for (i = 0; i < count; i++, ss++)
	{
        ss->numverts = LITTLESHORT(*mlf++);
		ss->leaf = (short)numleafs;

		for (j = 0; j < (int)ss->numverts; j++, lf++)
        {
            vertex = LITTLESHORT(*mlf++);

			if (vertex >= numvertexes)
				I_Error("P_LoadLeafs: vertex out of range\n");

			lf->vertex = &vertexes[vertex];

			seg = LITTLESHORT(*mlf++);
			if (seg != -1)
			{
				if (seg >= numsegs)
					I_Error("P_LoadLeafs: seg out of range\n");

				lf->seg = &segs[seg];
			}
			else
			{
				lf->seg = NULL;
			}
        }
        numleafs += (int)j;
	}
}

/*
=================
=
= P_LoadLights
= Exclusive Doom64
=
=================
*/

void P_LoadLights(void) // 8001E29C
{
    int         i;
    int         length;
    byte        *data;
    maplights_t *ml;
    light_t     *l;

    length = W_MapLumpLength(ML_LIGHTS);
    maplights = (maplights_t *)Z_Malloc(length, PU_LEVEL, 0);

    data = (byte *)W_GetMapLump(ML_LIGHTS);
    D_memcpy(maplights,data,length);

    numlights = (length / sizeof(maplights_t)) + 256;

    lights = (light_t *)Z_Malloc(numlights*sizeof(light_t), PU_LEVEL, 0);
    D_memset(lights,0,numlights*sizeof(light_t));

    ml = maplights;
    l = lights;

    /* Default light color (0 to 255) */
    for (i = 0; i < 256; i++, l++)
    {
        l->rgba = ((i << 24) | (i << 16) | (i << 8) | 255);
    }

    /* Copy custom light colors */
    for (i = i; i < numlights; i++, l++, ml++)
    {
        l->rgba = ((ml->r << 24) | (ml->g << 16) | (ml->b << 8) | ml->a);
        l->tag = LITTLESHORT(ml->tag);
    }

    P_SetLightFactor(0);
}

/*
=================
=
= P_LoadMacros
= Exclusive Doom64
=
=================
*/

void P_LoadMacros(void) // 8001E478
{
    short *data;
    int specialCount;
    byte *macroData;
    macro_t *pMacro;
    int headerSize;
    int i, j;

    data = (short *)W_GetMapLump(ML_MACROS);

    nummacros = LITTLESHORT(*data++);
    specialCount = LITTLESHORT(*data++);
    headerSize = sizeof(void*) * nummacros;

    macroData = (byte *)Z_Malloc(((nummacros + specialCount) * sizeof(macro_t)) + headerSize, PU_LEVEL, 0);
    macros = (macro_t**)macroData;
    pMacro = (macro_t*)(macroData + headerSize);

    for(i = 0; i < nummacros; i++)
    {
        macros[i] = pMacro;
        specialCount = LITTLESHORT(*data++);

        for(j = 0; j < specialCount+1; j++)
        {
            pMacro->id = LITTLESHORT(*data++);
            pMacro->tag = LITTLESHORT(*data++);
            pMacro->special = LITTLESHORT(*data++);

            if(j == specialCount)
                pMacro->id = 0;

            pMacro++;
        }
    }
}

/*
=================
=
= P_GroupLines
=
= Builds sector line lists and subsector sector numbers
= Finds block bounding boxes for sectors
=================
*/

void P_GroupLines (void) // 8001E614
{
	line_t		**linebuffer;
	int			i, j, total;
	sector_t	*sector;
	subsector_t	*ss;
	seg_t		*seg;
	int			block;
	line_t		*li;
	fixed_t		bbox[4];

/* look up sector number for each subsector */
	ss = subsectors;
	for (i=0 ; i<numsubsectors ; i++, ss++)
	{
		seg = &segs[ss->firstline];
		ss->sector = seg->sidedef->sector;
	}

/* count number of lines in each sector */
	li = lines;
	total = 0;
	for (i=0 ; i<numlines ; i++, li++)
	{
		total++;
		li->frontsector->linecount++;
		if (li->backsector && li->backsector != li->frontsector)
		{
			li->backsector->linecount++;
			total++;
		}
	}

/* build line tables for each sector	 */
	linebuffer = Z_Malloc (total*4, PU_LEVEL, 0);
	sector = sectors;
	for (i=0 ; i<numsectors ; i++, sector++)
	{
		M_ClearBox (bbox);
		sector->lines = linebuffer;
		li = lines;
		for (j=0 ; j<numlines ; j++, li++)
		{
			if (li->frontsector == sector || li->backsector == sector)
			{
				*linebuffer++ = li;
				M_AddToBox (bbox, li->v1->x, li->v1->y);
				M_AddToBox (bbox, li->v2->x, li->v2->y);
			}
		}
		if (linebuffer - sector->lines != sector->linecount)
			I_Error ("P_GroupLines: miscounted");

		/* set the degenmobj_t to the middle of the bounding box */
		sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
		sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;
		//sector->soundorg.z = (sector->floorheight + sector->ceilingheight) / 2;

		/* link into subsector */
		sector->soundorg.subsec = R_PointInSubsector(sector->soundorg.x, sector->soundorg.y);

		/* adjust bounding box to map blocks */
		block = (bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
		block = (block >= bmapheight) ? bmapheight-1 : block;
		sector->blockbox[BOXTOP]=block;

		block = (bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
		block = (block < 0) ? 0 : block;
		sector->blockbox[BOXBOTTOM]=block;

		block = (bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;
		block = (block >= bmapwidth) ? bmapwidth-1 : block;
		sector->blockbox[BOXRIGHT]=block;

		block = (bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
		block = (block < 0) ? 0 : block;
		sector->blockbox[BOXLEFT]=block;
	}
}

/*============================================================================= */

/*
=================
=
= P_SetupLevel
=
=================
*/

void P_SetupLevel(int map, skill_t skill) // 8001E974
{
	int		i, memory;
	int		lumpnum;
	mobj_t	*mobj;
	char	lumpname[16];

	/* free all tags except the PU_STATIC tag */
	Z_FreeTags(mainzone, ~PU_STATIC); // (PU_LEVEL | PU_LEVSPEC | PU_CACHE)

	Z_CheckZone(mainzone);//Z_CheckHeap
	M_ClearRandom();

	//printf("P_SetupLevel(%i,%i)\n", map, skill);
	//PRINTF_D2(WHITE, 0, 24, "P_SetupLevel(%i,%i)\n", map, skill);

	totalkills = totalitems = totalsecret = 0;

	//P_InitThinkers();
	thinkercap.prev = thinkercap.next = &thinkercap;
	mobjhead.next = mobjhead.prev = &mobjhead;

	spawncount = 0;

	W_OpenMapWad(map);

	/* note: most of this ordering is important	 */
    P_LoadMacros();
    P_LoadBlockMap();
    P_LoadVertexes();
    P_LoadSectors();
    P_LoadSideDefs();
    P_LoadLineDefs();
    P_LoadSubSectors();
    P_LoadNodes();
    P_LoadSegs();
    P_LoadLeafs();
    P_LoadReject();
    P_LoadLights();
    P_GroupLines();
	P_LoadThings();
	W_FreeMapLump();

	P_Init();

	/* set up world state */
	P_SpawnSpecials();
	R_SetupSky();

    Z_SetAllocBase(mainzone);
    Z_CheckZone(mainzone);

	memory = Z_FreeMemory(mainzone);
	if (memory < 0x10000)
	{
		Z_DumpHeap(mainzone);
		I_Error("P_SetupLevel: not enough free memory %d", memory);
	}

    P_SpawnPlayer();

    //PRINTF_D2(WHITE, 0, 27, "P_SetupLevel DONE\n");
}
