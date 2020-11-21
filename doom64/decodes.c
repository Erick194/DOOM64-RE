/* decodes.c */

#include "doomdef.h"

#include "graph.h"

/*=======*/
/* TYPES */
/*=======*/

typedef struct {
    int var0;
    int var1;
    int var2;
    int var3;
    byte *write;
    byte *writePos;
    byte *read;
    byte *readPos;
} decoder_t;

/*=========*/
/* GLOBALS */
/*=========*/

static short ShiftTable[6] = {4, 6, 8, 10, 12, 14}; // 8005D8A0

static int tableVar01[18];      // 800B2250

static short *PtrEvenTbl;       // 800B2298
static short *PtrOddTbl;        // 800B229C
static short *PtrNumTbl1;       // 800B22A0
static short *PtrNumTbl2;       // 800B22A4

//static short EvenTable[0x275]; // DecodeTable[0]
//static short OddTable[0x275];  // DecodeTable[0x278]
//static short NumTable1[0x4EA]; // DecodeTable[0x4F0]
//static short NumTable2[0x4EA]; // array01[0]

static short DecodeTable[2524]; // 800B22A8
static short array01[1258];     // 800B3660

static decoder_t decoder;       // 800B4034
static byte *allocPtr;          // 800B4054

static int OVERFLOW_READ;       // 800B4058
static int OVERFLOW_WRITE;      // 800B405C

/*
============================================================================

DECODE BASED ROUTINES

============================================================================
*/

/*
========================
=
= GetDecodeByte
=
========================
*/

static byte GetDecodeByte(void) // 8002D1D0
{
    if ((int)(decoder.readPos - decoder.read) >= OVERFLOW_READ)
        return -1;

    return *decoder.readPos++;
}

/*
========================
=
= WriteOutput
=
========================
*/

static void WriteOutput(byte outByte) // 8002D214
{
    if ((int)(decoder.writePos - decoder.write) >= OVERFLOW_WRITE)
        I_Error("Overflowed output buffer");

    *decoder.writePos++ = outByte;
}

/*
========================
=
= WriteBinary
= routine required for encoding
=
========================
*/

static void WriteBinary(int binary) // 8002D288
{
    decoder.var3 = (decoder.var3 << 1);

    if (binary != 0)
        decoder.var3 = (decoder.var3 | 1);

    decoder.var2 = (decoder.var2 + 1);
    if (decoder.var2 == 8)
    {
        WriteOutput((byte)decoder.var3);
        decoder.var2 = 0;
    }
}

/*
========================
=
= DecodeScan
=
========================
*/

static int DecodeScan(void) // 8002D2F4
{
    int resultbyte;

    resultbyte = decoder.var0;

    decoder.var0 = (resultbyte - 1);
    if ((resultbyte < 1))
    {
        resultbyte = GetDecodeByte();

        decoder.var1 = resultbyte;
        decoder.var0 = 7;
    }

    resultbyte = (0 < (decoder.var1 & 0x80));
    decoder.var1 = (decoder.var1 << 1);

    return resultbyte;
}

/*
========================
=
= MakeExtraBinary
= routine required for encoding
=
========================
*/

static void MakeExtraBinary(int binary, int shift) // 8002D364
{
    int i;

    i = 0;
    if (shift > 0)
    {
        do
        {
            WriteBinary(binary & 1);
            binary = (binary >> 1);
        } while (++i != shift);
    }
}

/*
========================
=
= RescanByte
=
========================
*/

static int RescanByte(int byte) // 8002D3B8
{
    int shift;
    int i;
    int resultbyte;

    resultbyte = 0;
    i = 0;
    shift = 1;

    if(byte <= 0)
        return resultbyte;

    do
    {
        if (DecodeScan() != 0)
            resultbyte |= shift;

        i++;
        shift = (shift << 1);
    } while (i != byte);

    return resultbyte;
}

/*
========================
=
= WriteEndCode
= routine required for encoding
=
========================
*/

static void WriteEndCode(void) // 8002D424
{
    if (decoder.var2 > 0) {
        WriteOutput((byte)(decoder.var3 << (8 - decoder.var2)) & 0xff);
    }
}

/*
========================
=
= InitDecodeTable
=
========================
*/

static void InitDecodeTable(void) // 8002D468
{
    int evenVal, oddVal, incrVal;

    short *curArray;
    short *incrTbl;
    short *evenTbl;
    short *oddTbl;

	tableVar01[15] = 3;
    tableVar01[16] = 0;
    tableVar01[17] = 0;

    decoder.var0 = 0;
    decoder.var1 = 0;
    decoder.var2 = 0;
    decoder.var3 = 0;

    curArray = &array01[2];
    incrTbl = &DecodeTable[0x4F2];

    incrVal = 2;

    do
    {
        if(incrVal < 0) {
            *incrTbl = (short)((incrVal + 1) >> 1);
        }
        else {
            *incrTbl = (short)(incrVal >> 1);
        }

        *curArray++ = 1;
        incrTbl++;
    } while(++incrVal < 1258);

    oddTbl  = &DecodeTable[0x279];
    evenTbl = &DecodeTable[1];

    evenVal = 2;
    oddVal = 3;

    do
    {
        *oddTbl++ = (short)oddVal;
        oddVal += 2;

        *evenTbl++ = (short)evenVal;
        evenVal += 2;

    } while(oddVal < 1259);

    tableVar01[0] = 0;

    incrVal = (1 << ShiftTable[0]);
    tableVar01[6] = (incrVal - 1);
    tableVar01[1] = incrVal;

    incrVal += (1 << ShiftTable[1]);
    tableVar01[7] = (incrVal - 1);
    tableVar01[2] = incrVal;

    incrVal += (1 << ShiftTable[2]);
	tableVar01[8] = (incrVal - 1);
    tableVar01[3] = incrVal;

    incrVal += (1 << ShiftTable[3]);
	tableVar01[9] = (incrVal - 1);
    tableVar01[4] = incrVal;

    incrVal += (1 << ShiftTable[4]);
	tableVar01[10] = (incrVal - 1);
    tableVar01[5] = incrVal;

    incrVal += (1 << ShiftTable[5]);
	tableVar01[11] = (incrVal - 1);
    tableVar01[12] = (incrVal - 1);

    tableVar01[13] = tableVar01[12] + 64;
}

/*
========================
=
= CheckTable
=
========================
*/

static void CheckTable(int a0,int a1,int a2) // 8002D624
{
    int i;
    int idByte1;
    int idByte2;
    short *curArray;
    short *evenTbl;
    short *oddTbl;
    short *incrTbl;

    i = 0;
    evenTbl = &DecodeTable[0];
    oddTbl  = &DecodeTable[0x278];
    incrTbl = &DecodeTable[0x4F0];

    idByte1 = a0;

    do {
        idByte2 = incrTbl[idByte1];

        array01[idByte2] = (array01[a1] + array01[a0]);

        a0 = idByte2;

        if(idByte2 != 1) {
            idByte1 = incrTbl[idByte2];
            idByte2 = evenTbl[idByte1];

            a1 = idByte2;

            if(a0 == idByte2) {
                a1 = oddTbl[idByte1];
            }
        }

        idByte1 = a0;
    }while(a0 != 1);

    if(array01[1] != 0x7D0) {
        return;
    }

    array01[1] >>= 1;

    curArray = &array01[2];
    do
    {
        curArray[3] >>= 1;
        curArray[2] >>= 1;
        curArray[1] >>= 1;
        curArray[0] >>= 1;
        curArray += 4;
        i += 4;
    } while(i != 1256);
}

/*
========================
=
= DecodeByte
=
========================
*/

static void DecodeByte(int tblpos) // 8002D72C
{
    int incrIdx;
    int evenVal;
    int idByte1;
    int idByte2;
    int idByte3;
    int idByte4;

    short *evenTbl;
    short *oddTbl;
    short *incrTbl;
    short *tmpIncrTbl;

    evenTbl = &DecodeTable[0];
    oddTbl  = &DecodeTable[0x278];
    incrTbl = &DecodeTable[0x4F0];

    idByte1 = (tblpos + 0x275);
    array01[idByte1] += 1;

    if (incrTbl[idByte1] != 1)
    {
        tmpIncrTbl = &incrTbl[idByte1];
        idByte2 = *tmpIncrTbl;

        if (idByte1 == evenTbl[idByte2]) {
            CheckTable(idByte1, oddTbl[idByte2], idByte1);
        }
        else {
            CheckTable(idByte1, evenTbl[idByte2], idByte1);
        }

        do
        {
            incrIdx = incrTbl[idByte2];
            evenVal = evenTbl[incrIdx];

            if (idByte2 == evenVal) {
                idByte3 = oddTbl[incrIdx];
            }
            else {
                idByte3 = evenVal;
            }

            if (array01[idByte3] < array01[idByte1])
            {
                if (idByte2 == evenVal) {
                    oddTbl[incrIdx] = (short)idByte1;
                }
                else {
                    evenTbl[incrIdx] = (short)idByte1;
                }

                evenVal = evenTbl[idByte2];

                if (idByte1 == evenVal) {
                    idByte4 = oddTbl[idByte2];
                    evenTbl[idByte2] = (short)idByte3;
                }
                else {
                    idByte4 = evenVal;
                    oddTbl[idByte2] = (short)idByte3;
                }

                incrTbl[idByte3] = (short)idByte2;

                *tmpIncrTbl = (short)incrIdx;
                CheckTable(idByte3, idByte4, idByte4);

                tmpIncrTbl = &incrTbl[idByte3];
            }

            idByte1 = *tmpIncrTbl;
            tmpIncrTbl = &incrTbl[idByte1];

            idByte2 = *tmpIncrTbl;
        } while (idByte2 != 1);
    }
}

/*
========================
=
= StartDecodeByte
=
========================
*/

static int StartDecodeByte(void) // 8002D904
{
    int lookup;
    short *evenTbl;
    short *oddTbl;

    lookup = 1;

    evenTbl = &DecodeTable[0];
    oddTbl  = &DecodeTable[0x278];

    while(lookup < 0x275)
    {
        if(DecodeScan() == 0) {
            lookup = evenTbl[lookup];
        }
        else {
            lookup = oddTbl[lookup];
        }
    }

    lookup = (lookup + -0x275);
    DecodeByte(lookup);

    return lookup;
}

/*
========================
=
= L8002d990
= unknown Function
=
========================
*/

void L8002d990(int arg0) // 8002D990
{
	int val;

	val = ((allocPtr[(arg0 + 2) % tableVar01[13]] << 8) ^ (allocPtr[arg0] ^ (allocPtr[(arg0+1) % tableVar01[13]] << 4))) & 0x3fff;

	if (PtrEvenTbl[val] == -1)
	{
		PtrOddTbl[val] = arg0;
		PtrNumTbl1[arg0] = -1;
	}
	else
	{
		PtrNumTbl1[arg0] = PtrEvenTbl[val];
		PtrNumTbl2[PtrEvenTbl[val]] = arg0;
	}

	PtrEvenTbl[val] = arg0;
	PtrNumTbl2[arg0] = -1;
}

/*
========================
=
= FUN_8002dad0
= unknown Function
=
========================
*/

void FUN_8002dad0(int arg0) // 8002DAD0
{
	int val;

	val = ((allocPtr[(arg0 + 2) % tableVar01[13]] << 8) ^ (allocPtr[arg0] ^ (allocPtr[(arg0+1) % tableVar01[13]] << 4))) & 0x3fff;

	if (PtrEvenTbl[val] == PtrOddTbl[val])
	{
		PtrEvenTbl[val] = -1;
	}
	else
	{
		PtrNumTbl1[PtrNumTbl2[PtrOddTbl[val]]] = -1;
		PtrOddTbl[val] = PtrNumTbl2[PtrOddTbl[val]];
	}
}

/*
========================
=
= FUN_8002dc0c
= unknown Function
=
========================
*/

int FUN_8002dc0c(int start, int count) // 8002DC0C
{
    short sVar1;
    int iVar2;
    int iVar4;
    int iVar5;
    int iVar6;

    int cnt;
    int curr, next;

    iVar4 = 0;
    if (start == tableVar01[13]) {
        start = 0;
    }

    sVar1 = PtrEvenTbl[(allocPtr[(start + 2) % tableVar01[13]] << 8 ^ allocPtr[start] ^ allocPtr[(start + 1) % tableVar01[13]] << 4) & 0x3fff];

    iVar5 = 1;
    do
    {
        iVar2 = (int)sVar1;
        if ((iVar2 == -1) || (count < iVar5)) {
            return iVar4;
        }

        if ((allocPtr[(start + iVar4) % tableVar01[13]]) ==
            (allocPtr[(iVar2 + iVar4) % tableVar01[13]]))
        {
            cnt = 0;
            if (allocPtr[start] == allocPtr[iVar2])
            {
                curr = start;
                next = iVar2;

                if(next != start)
                {
                    while (curr != tableVar01[15])
                    {
                        curr++;
                        if (curr == tableVar01[13]) {
                            curr = 0;
                        }

                        next++;
                        if (next == tableVar01[13]) {
                            next = 0;
                        }

                        cnt++;

                        if (allocPtr[curr] != allocPtr[next])
                            break;

                        if (cnt >= 64)
                            break;

                        if (next == start)
                            break;
                    }
                }
            }

            iVar6 = start - iVar2;
            if (iVar6 < 0) {
                iVar6 += tableVar01[13];
            }

            iVar6 -= cnt;
            if (tableVar01[16] && (tableVar01[6]/*15*/ < iVar6)) {
                return iVar4;
            }

            //if (((iVar4 < cnt) && (iVar6 <= tableVar01[12])) &&
            //((3 < cnt || (iVar6 <= tableVar01[tableVar01[17] + 9]))))
            if(iVar4 < cnt)
            {
                if(iVar6 <= tableVar01[12])
                {
                    if((cnt > 3) || (iVar6 <= tableVar01[tableVar01[17] + 9]))
                    {
                        iVar4 = cnt;
                        tableVar01[14] = iVar6;
                    }
                }
            }
        }

        sVar1 = PtrNumTbl1[iVar2];
        iVar5++;
    } while( true );
}

/*
========================
=
= FUN_8002df14
= unknown Function
=
========================
*/

void FUN_8002df14(void) // 8002DF14
{
    byte byte_val;

    int i, j, k;
    byte *curPtr;
    byte *nextPtr;
    byte *next2Ptr;

    curPtr = &allocPtr[0];

    k = 0;
    j = 0;
    i = 1;
    do
    {
        nextPtr = &allocPtr[j];
        if (curPtr[0] == 10)
        {
            j = i;
            if(nextPtr[0] == curPtr[1])
            {
                next2Ptr = &allocPtr[i+1];
                do
                {
                    nextPtr++;
                    byte_val = *next2Ptr++;
                    k++;
                } while (*nextPtr == byte_val);
            }
        }
        curPtr++;
        i++;
    } while (i != 67);

    if (k >= 16)
        tableVar01[16] = 1;
}

/*
========================
=
= DecodeD64
=
= Exclusive Doom 64
=
========================
*/

void DecodeD64(unsigned char *input, unsigned char *output) // 8002DFA0
{
    int copyPos, storePos;
	int dec_byte, resc_byte;
	int incrBit, copyCnt, shiftPos, j;

	//PRINTF_D2(WHITE, 0, 15, "DecodeD64");

	InitDecodeTable();

	OVERFLOW_READ = MAXINT;
    OVERFLOW_WRITE = MAXINT;

	incrBit = 0;

	decoder.read = input;
	decoder.readPos = input;
	decoder.write = output;
	decoder.writePos = output;

	allocPtr = (byte *)Z_Alloc(tableVar01[13], PU_STATIC, NULL);

    dec_byte = StartDecodeByte();

    while(dec_byte != 256)
    {
        if(dec_byte < 256)
        {
            /* Decode the data directly using binary data code */

            WriteOutput((byte)(dec_byte & 0xff));
            allocPtr[incrBit] = (byte)dec_byte;

            /* Resets the count once the memory limit is exceeded in allocPtr,
                so to speak resets it at startup for reuse */
            incrBit += 1;
            if(incrBit == tableVar01[13]) {
                incrBit = 0;
            }
        }
        else
        {
            /* Decode the data using binary data code,
                a count is obtained for the repeated data,
                positioning itself in the root that is being stored in allocPtr previously. */

            /*  A number is obtained from a range from 0 to 5,
                necessary to obtain a shift value in the ShiftTable*/
            shiftPos = (dec_byte + -257) / 62;

            /*  get a count number for data to copy */
            copyCnt  = (dec_byte - (shiftPos * 62)) + -254;

            /*  To start copying data, you receive a position number
                that you must sum with the position of table tableVar01 */
            resc_byte = RescanByte(ShiftTable[shiftPos]);

            /*  with this formula the exact position is obtained
                to start copying previously stored data */
            copyPos = incrBit - ((tableVar01[shiftPos] + resc_byte) + copyCnt);

            if(copyPos < 0) {
                copyPos += tableVar01[13];
            }

            storePos = incrBit;

            for(j = 0; j < copyCnt; j++)
            {
                /* write the copied data */
                WriteOutput(allocPtr[copyPos]);

                /* save copied data at current position in memory allocPtr */
                allocPtr[storePos] = allocPtr[copyPos];

                storePos++; /* advance to next allocPtr memory block to store */
                copyPos++;  /* advance to next allocPtr memory block to copy */

                /* reset the position of storePos once the memory limit is exceeded */
                if(storePos == tableVar01[13]) {
                    storePos = 0;
                }

                /* reset the position of copyPos once the memory limit is exceeded */
                if(copyPos == tableVar01[13]) {
                    copyPos = 0;
                }
            }

            /* Resets the count once the memory limit is exceeded in allocPtr,
                so to speak resets it at startup for reuse */
            incrBit += copyCnt;
            if (incrBit >= tableVar01[13]) {
                incrBit -= tableVar01[13];
            }
        }

        dec_byte = StartDecodeByte();
    }

	Z_Free(allocPtr);

	//PRINTF_D2(WHITE, 0, 21, "DecodeD64:End");
}

/*
== == == == == == == == == ==
=
= DecodeJaguar (decode original name)
=
= Exclusive Psx Doom / Doom 64 from Jaguar Doom
=
== == == == == == == == == ==
*/

#define WINDOW_SIZE	4096
#define LOOKAHEAD_SIZE	16

#define LENSHIFT 4		/* this must be log2(LOOKAHEAD_SIZE) */

void DecodeJaguar(unsigned char *input, unsigned char *output) // 8002E1f4
{
    int getidbyte = 0;
	int len;
	int pos;
	int i;
	unsigned char *source;
	int idbyte = 0;

	while (1)
	{
		/* get a new idbyte if necessary */
		if (!getidbyte) idbyte = *input++;
		getidbyte = (getidbyte + 1) & 7;

		if (idbyte & 1)
		{
			/* decompress */
			pos = *input++ << LENSHIFT;
			pos = pos | (*input >> LENSHIFT);
			source = output - pos - 1;
			len = (*input++ & 0xf) + 1;
			if (len == 1) break;

			//for (i = 0; i<len; i++)
				//*output++ = *source++;

            i = 0;
            if (len > 0)
            {
                if ((len & 3))
                {
                    while(i != (len & 3))
                    {
                        *output++ = *source++;
                        i++;
                    }
                }
                while(i != len)
                {
                    output[0] = source[0];
                    output[1] = source[1];
                    output[2] = source[2];
                    output[3] = source[3];
                    output += 4;
                    source += 4;
                    i += 4;
                }
            }
		}
		else
        {
			*output++ = *input++;
		}

		idbyte = idbyte >> 1;
	}
}
