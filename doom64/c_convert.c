/* c_convert.c  */

/*-----------------------------------*/
/* Color Converter RGB2HSV & HSV2RGB */
/*-----------------------------------*/

#include "doomdef.h"

/*
===================
=
= LightGetHSV
= Set HSV values based on given RGB
=
===================
*/

void LightGetHSV(int r,int g,int b,int *h,int *s,int *v) // 800020BC
{
    register u32 fpstat, fpstatset;

    int min;
    int max;
    float deltamin;
    float deltamax;
    float j;
    float x;
    float xr;
    float xg;
    float xb;
    float sum;

    max = MAXINT;

    if(r < max) {
        max = r;
    }
    if(g < max) {
        max = g;
    }
    if(b < max) {
        max = b;
    }

    min = MININT;

    if(r > min) {
        min = r;
    }
    if(g > min) {
        min = g;
    }
    if(b > min) {
        min = b;
    }

    deltamin = (float)((double)min / (double)255.0);
    deltamax = deltamin - (float)((double)max / (double)255.0);

    if((double)deltamin == 0.0) {
        j = 0.0;
    }
    else {
        j = deltamax / deltamin;
    }

    if((double)j != 0.0)
    {
        xr = (float)((double)r / (double)255.0);
        xg = (float)((double)g / (double)255.0);
        xb = (float)((double)b / (double)255.0);

        if(xr != deltamin)
        {
            if(xg != deltamin)
            {
                if(xb == deltamin)
                {
                    sum = ((deltamin - xg) / deltamax + 4.0) -
                          ((deltamin - xr) / deltamax);
                }
            }
            else
            {
                sum = ((deltamin - xr) / deltamax + 2.0) -
                      ((deltamin - xb) / deltamax);
            }
        }
        else
        {
            sum = ((deltamin - xb) / deltamax) -
                  ((deltamin - xg) / deltamax);
        }

        x = (sum * 60.0);

        if(x < 0.0)
        {
            x = (float)((double)x + (double)360.0);
        }
    }
    else
    {
        j = 0.0;
    }

    // fetch the current floating-point control/status register
	fpstat = __osGetFpcCsr();
	// enable round to negative infinity for floating point
	fpstatset = (fpstat | FPCSR_RM_RM) ^ 2;
	// _Disable_ unimplemented operation exception for floating point.
	__osSetFpcCsr(fpstatset);

    *h = (int)(((double)x / (double)360.0) * (double)255.0);

    // fetch the current floating-point control/status register
    fpstat = __osGetFpcCsr();
    // enable round to negative infinity for floating point
    fpstatset = (fpstat | FPCSR_RM_RM) ^ 2;
    // _Disable_ unimplemented operation exception for floating point.
    __osSetFpcCsr(fpstatset);
    *s = (int)((double)j * (double)255.0);

    // fetch the current floating-point control/status register
    fpstat = __osGetFpcCsr();
    // enable round to negative infinity for floating point
    fpstatset = (fpstat | FPCSR_RM_RM) ^ 2;
    // _Disable_ unimplemented operation exception for floating point.
    __osSetFpcCsr(fpstatset);
    *v = (int)((double)deltamin * (double)255.0);
}

/*
===================
=
= LightGetRGB
= Set RGB values based on given HSV
=
===================
*/

void LightGetRGB(int h,int s,int v,int *r,int *g,int *b) // 8000248C
{
    register u32 fpstat, fpstatset;

    float x;
    float j;
    float i;
    float t;
    int table;
    float xr;
    float xg;
    float xb;

    j = (float)(((double)h / (double)255.0) * (double)360.0);

    if((double)360.0 <= (double)j) {
        j = (float)((double)j - (double)360.0);
    }

    x = ((double)s / (double)255.0);
    i = ((double)v / (double)255.0);

    if(x != 0.0)
    {
        // fetch the current floating-point control/status register
        fpstat = __osGetFpcCsr();
        // enable round to negative infinity for floating point
        fpstatset = (fpstat | FPCSR_RM_RM) ^ 2;
        // _Disable_ unimplemented operation exception for floating point.
        __osSetFpcCsr(fpstatset);

        table = (int)(j / 60.0);
        if(table < 6)
        {
            t = (j / 60.0);
            switch(table) {
            case 0:
                xr = i;
                xg = ((1.0 - ((1.0 - (t - (float)table)) * x)) * i);
                xb = ((1.0 - x) * i);
                break;
            case 1:
                xr = ((1.0 - (x * (t - (float)table))) * i);
                xg = i;
                xb = ((1.0 - x) * i);
                break;
            case 2:
                xr = ((1.0 - x) * i);
                xg = i;
                xb = ((1.0 - ((1.0 - (t - (float)table)) * x)) * i);
                break;
            case 3:
                xr = ((1.0 - x) * i);
                xg = ((1.0 - (x * (t - (float)table))) * i);
                xb = i;
                break;
            case 4:
                xr = ((1.0 - ((1.0 - (t - (float)table)) * x)) * i);
                xg = ((1.0 - x) * i);
                xb = i;
                break;
            case 5:
                xr = i;
                xg = ((1.0 - x) * i);
                xb = ((1.0 - (x * (t - (float)table))) * i);
                break;
            }
        }
    }
    else
    {
        xr = xg = xb = i;
    }

    // fetch the current floating-point control/status register
	fpstat = __osGetFpcCsr();
	// enable round to negative infinity for floating point
	fpstatset = (fpstat | FPCSR_RM_RM) ^ 2;
	// _Disable_ unimplemented operation exception for floating point.
	__osSetFpcCsr(fpstatset);

    *r = (int)((double)xr * (double)255.0);

    // fetch the current floating-point control/status register
    fpstat = __osGetFpcCsr();
    // enable round to negative infinity for floating point
    fpstatset = (fpstat | FPCSR_RM_RM) ^ 2;
    // _Disable_ unimplemented operation exception for floating point.
    __osSetFpcCsr(fpstatset);

    *g = (int)((double)xg * (double)255.0);

    // fetch the current floating-point control/status register
    fpstat = __osGetFpcCsr();
    // enable round to negative infinity for floating point
    fpstatset = (fpstat | FPCSR_RM_RM) ^ 2;
    // _Disable_ unimplemented operation exception for floating point.
    __osSetFpcCsr(fpstatset);

    *b = (int)((double)xb * (double)255.0);
}
