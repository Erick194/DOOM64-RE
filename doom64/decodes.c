/* decodes.c */

#include "doomdef.h"

#include "graph.h"

/*=======*/
/* TYPES */
/*=======*/

typedef struct {
    int dec_bit_count;
    int dec_bit_buffer;
    int enc_bit_count;
    int enc_bit_buffer;
    byte* ostart;
    byte* output;
    byte* istart;
    byte* input;
} buffers_t;

typedef struct {
    int offset;
    int incrBit;
    int unk1;
    int type;
} encodeArgs_t;

/*=========*/
/* GLOBALS */
/*=========*/

static short ShiftTable[6] = { 4, 6, 8, 10, 12, 14 }; // 8005D8A0

static int offsetTable[12];      // 800B2250
static int offsetMaxSize, windowSize; // 800b2280 , 800b2284
static encodeArgs_t encArgs;    // 800b2288

#define HASH_SIZE (1 << 14)
static short* hashtable;    // 800B2298
static short* hashtarget;   // 800B229C
static short* hashNext;     // 800B22A0
static short* hashPrev;     // 800B22A4

static short DecodeTable[2516]; // 800B22A8
static short array01[1258];     // 800B3660

static buffers_t buffers;       // 800B4034
static byte* window;            // 800B4054

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
= GetOutputSize
=
========================
*/
int GetOutputSize(void) // [GEC] New
{
    return (int)((int)buffers.output - (int)buffers.ostart);
}

/*
========================
=
= GetReadSize
=
========================
*/

int GetReadSize(void) // [GEC] New
{
    return (int)((int)buffers.input - (int)buffers.istart);
}

/*
========================
=
= ReadByte -> Old GetDecodeByte
=
========================
*/

static int ReadByte(void) // 8002D1D0
{
    if ((int)(buffers.input - buffers.istart) >= OVERFLOW_READ)
        return -1;

    return *buffers.input++;
}

/*
========================
=
= WriteByte -> Old WriteOutput
=
========================
*/

static void WriteByte(byte outByte) // 8002D214
{
    if ((int)(buffers.output - buffers.ostart) >= OVERFLOW_WRITE) {
        I_Error("Overflowed output buffer");
    }

    *buffers.output++ = outByte;
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
    buffers.enc_bit_buffer = (buffers.enc_bit_buffer << 1);

    if (binary != 0)
        buffers.enc_bit_buffer = (buffers.enc_bit_buffer | 1);

    buffers.enc_bit_count = (buffers.enc_bit_count + 1);
    if (buffers.enc_bit_count == 8)
    {
        WriteByte((byte)buffers.enc_bit_buffer);
        buffers.enc_bit_count = 0;
    }
}

/*
========================
=
= ReadBinary -> old DecodeScan
=
========================
*/

static int ReadBinary(void) // 8002D2F4
{
    int resultbyte;

    resultbyte = buffers.dec_bit_count;

    buffers.dec_bit_count = (resultbyte - 1);
    if ((resultbyte < 1))
    {
        resultbyte = ReadByte();

        buffers.dec_bit_buffer = resultbyte;
        buffers.dec_bit_count = 7;
    }

    resultbyte = (0 < (buffers.dec_bit_buffer & 0x80));
    buffers.dec_bit_buffer = (buffers.dec_bit_buffer << 1);

    return resultbyte;
}

/*
========================
=
= WriteCodeBinary
= routine required for encoding
=
========================
*/

static void WriteCodeBinary(int binary, int shift) // 8002D364
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
= ReadCodeBinary -> old RescanByte
=
========================
*/

static int ReadCodeBinary(int byte) // 8002D3B8
{
    int shift;
    int i;
    int resultbyte;

    resultbyte = 0;
    i = 0;
    shift = 1;

    if (byte <= 0)
        return resultbyte;

    do
    {
        if (ReadBinary() != 0)
            resultbyte |= shift;

        i++;
        shift = (shift << 1);
    } while (i != byte);

    return resultbyte;
}

/*
========================
=
= FlushBitBuffer
= routine required for encoding
=
========================
*/

static void FlushBitBuffer(void) // 8002D424
{
    if (buffers.enc_bit_count > 0) {
        WriteByte((byte)(buffers.enc_bit_buffer << (8 - buffers.enc_bit_count)) & 0xff);
    }
}

/*
========================
=
= InitTables -> old InitDecodeTable
=
========================
*/

static void InitTables(void) // 8002D468
{
    int evenVal, oddVal, incrVal, i;

    short* curArray;
    short* incrTbl;
    short* evenTbl;
    short* oddTbl;

    int* Tbl1, * Tbl2;

    encArgs.incrBit = 3;
    encArgs.unk1 = 0;
    encArgs.type = 0;

    buffers.dec_bit_count = 0;
    buffers.dec_bit_buffer = 0;
    buffers.enc_bit_count = 0;
    buffers.enc_bit_buffer = 0;

    curArray = &array01[(0 + 2)];
    incrTbl = &DecodeTable[(1258 + 2)];

    incrVal = 2;

    do {
        *incrTbl++ = (short)(incrVal / 2);
        *curArray++ = 1;
    } while (++incrVal < 1258);

    oddTbl = &DecodeTable[(629 + 1)];
    evenTbl = &DecodeTable[(0 + 1)];

    evenVal = 1;
    oddVal = 3;

    do
    {
        *oddTbl++ = (short)oddVal;
        oddVal += 2;

        *evenTbl++ = (short)(evenVal * 2);
        evenVal++;
    } while (evenVal < 629);

    incrVal = 0;
    i = 0;
    Tbl2 = &offsetTable[6];
    Tbl1 = &offsetTable[0];
    do {
        *Tbl1++ = incrVal;
        incrVal += (1 << (ShiftTable[i] & 0x1f));
        *Tbl2++ = incrVal - 1;
    } while (++i <= 5);

    offsetMaxSize = incrVal - 1;
    windowSize = offsetMaxSize + (64 - 1);
}

/*
========================
=
= CheckTable
=
========================
*/

static void CheckTable(int a0, int a1, int a2) // 8002D624
{
    int i;
    int idByte1;
    int idByte2;
    short* curArray;
    short* evenTbl;
    short* oddTbl;
    short* incrTbl;

    i = 0;
    evenTbl = &DecodeTable[0];
    oddTbl  = &DecodeTable[629];
    incrTbl = &DecodeTable[1258];

    idByte1 = a0;

    do {
        idByte2 = incrTbl[idByte1];

        array01[idByte2] = (array01[a1] + array01[a0]);

        a0 = idByte2;

        if (idByte2 != 1) {
            idByte1 = incrTbl[idByte2];
            idByte2 = evenTbl[idByte1];

            a1 = idByte2;

            if (a0 == idByte2) {
                a1 = oddTbl[idByte1];
            }
        }

        idByte1 = a0;
    } while (a0 != 1);

    if (array01[1] != 0x7D0) {
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
    } while (i != 1256);
}

/*
========================
=
= UpdateTables -> old DecodeByte
=
========================
*/

static void UpdateTables(int tblpos) // 8002D72C
{
    int incrIdx;
    int evenVal;
    int idByte1;
    int idByte2;
    int idByte3;
    int idByte4;

    short* evenTbl;
    short* oddTbl;
    short* incrTbl;
    short* tmpIncrTbl;

    evenTbl = &DecodeTable[0];
    oddTbl  = &DecodeTable[629];
    incrTbl = &DecodeTable[1258];

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
    short* evenTbl;
    short* oddTbl;

    lookup = 1;

    evenTbl = &DecodeTable[0];
    oddTbl  = &DecodeTable[629];

    while (lookup < 0x275) {
        if (ReadBinary() == 0) {
            lookup = evenTbl[lookup];
        }
        else {
            lookup = oddTbl[lookup];
        }
    }

    lookup = (lookup + -0x275);
    UpdateTables(lookup);

    return lookup;
}

/*
========================
=
= InsertNodeDirectory
= routine required for encoding
=
========================
*/

void InsertNodeDirectory(int start) // 8002D990
{
    int hashKey = ((window[start % windowSize] ^ (window[(start + 1) % windowSize] << 4)) ^ (window[(start + 2) % windowSize] << 8)) & (HASH_SIZE - 1);

    if (hashtable[hashKey] == -1) {
        hashtarget[hashKey] = start;
        hashNext[start] = -1;
    }
    else {
        hashNext[start] = hashtable[hashKey];
        hashPrev[hashtable[hashKey]] = start;
    }

    hashtable[hashKey] = start;
    hashPrev[start] = -1;
}

/*
========================
=
= DeleteNodeDirectory
= routine required for encoding
=
========================
*/

void DeleteNodeDirectory(int start) // 8002DAD0
{
    int hashKey = ((window[start % windowSize] ^ (window[(start + 1) % windowSize] << 4)) ^ (window[(start + 2) % windowSize] << 8)) & (HASH_SIZE - 1);

    if (hashtable[hashKey] == hashtarget[hashKey]) {
        hashtable[hashKey] = -1;
    }
    else {
        hashNext[hashPrev[hashtarget[hashKey]]] = -1;
        hashtarget[hashKey] = hashPrev[hashtarget[hashKey]];
    }
}

/*
========================
=
= FindMatch
= routine required for encoding
=
========================
*/


int FindMatch(int start, int count) // 8002DC0C
{
    int encodedlen;
    int offset;
    int i;
    int samelen;
    int next;
    int curr;
    int encodedpos;
    int hashKey;

    encodedlen = 0;
    if (start == windowSize) {
        start = 0;
    }

    hashKey = ((window[start % windowSize] ^ (window[(start + 1) % windowSize] << 4)) ^ (window[(start + 2) % windowSize] << 8)) & (HASH_SIZE - 1);

    offset = hashtable[hashKey];

    i = 0;
    while (offset != -1)
    {
        if (++i > count) {
            break;
        }

        if ((window[(start + encodedlen) % windowSize]) == (window[(offset + encodedlen) % windowSize]))
        {
            samelen = 0;
            curr = start;
            next = offset;

            while (window[curr] == window[next]) {
                if (samelen >= 64) {
                    break;
                }
                if (next == start) {
                    break;
                }
                if (curr == encArgs.incrBit) {
                    break;
                }
                ++samelen;
                if (++curr == windowSize) {
                    curr = 0;
                }
                if (++next == windowSize) {
                    next = 0;
                }
            }

            encodedpos = start - offset;
            if (encodedpos < 0) {
                encodedpos += windowSize;
            }
            encodedpos -= samelen;
            if ((encArgs.unk1) && (encodedpos > offsetTable[6])) {
                break;
            }

            if (encodedlen < samelen && encodedpos <= offsetMaxSize && (samelen > 3 || offsetTable[6 + (encArgs.type + 3)] >= encodedpos)) {
                encodedlen = samelen;
                encArgs.offset = encodedpos;
            }
        }

        offset = hashNext[offset]; // try next in list
    }
    return encodedlen;
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
    byte* curPtr;
    byte* nextPtr;
    byte* next2Ptr;

    curPtr = &window[0];

    k = 0;
    j = 0;
    i = 1;
    do
    {
        nextPtr = &window[j];
        if (curPtr[0] == 10)
        {
            j = i;
            if (nextPtr[0] == curPtr[1])
            {
                next2Ptr = &window[i + 1];
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

    if (k >= 16) {
        encArgs.unk1 = 1;
    }
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

void DecodeD64(unsigned char* input, unsigned char* output) // 8002DFA0
{
    int copyPos, storePos;
    int dec_byte, resc_byte;
    int incrBit, copyCnt, shiftPos, j;

    //PRINTF_D2(WHITE, 0, 15, "DecodeD64");

    InitTables();

    OVERFLOW_READ = MAXINT;
    OVERFLOW_WRITE = MAXINT;

    incrBit = 0;

    buffers.input = buffers.istart = input;
    buffers.output = buffers.ostart = output;

    window = (byte *)Z_Alloc(windowSize, PU_STATIC, NULL);

    dec_byte = StartDecodeByte();

    while (dec_byte != 256)
    {
        if (dec_byte < 256)
        {
            /*  Decode the data directly using binary data code */

            WriteByte((byte)(dec_byte & 0xff));
            window[incrBit] = (byte)dec_byte;

            /*  Resets the count once the memory limit is exceeded in allocPtr,
                so to speak resets it at startup for reuse */
            incrBit += 1;
            if (incrBit == windowSize) {
                incrBit = 0;
            }
        }
        else
        {
            /*  Decode the data using binary data code,
                a count is obtained for the repeated data,
                positioning itself in the root that is being stored in allocPtr previously. */

            /*  A number is obtained from a range from 0 to 5,
                necessary to obtain a shift value in the ShiftTable*/
            shiftPos = (dec_byte + -257) / 62;

            /*  Get a count number for data to copy */
            copyCnt = (dec_byte - (shiftPos * 62)) + -254;

            /*  To start copying data, you receive a position number
                that you must sum with the position of table tableVar01 */
            resc_byte = ReadCodeBinary(ShiftTable[shiftPos]);

            /*  with this formula the exact position is obtained
                to start copying previously stored data */
            copyPos = incrBit - ((offsetTable[shiftPos] + resc_byte) + copyCnt);

            if (copyPos < 0) {
                copyPos += windowSize;
            }

            storePos = incrBit;

            for (j = 0; j < copyCnt; j++)
            {
                /* write the copied data */
                WriteByte(window[copyPos]);

                /* save copied data at current position in memory allocPtr */
                window[storePos] = window[copyPos];

                storePos++; /* advance to next allocPtr memory block to store */
                copyPos++;  /* advance to next allocPtr memory block to copy */

                /* reset the position of storePos once the memory limit is exceeded */
                if (storePos == windowSize) {
                    storePos = 0;
                }

                /* reset the position of copyPos once the memory limit is exceeded */
                if (copyPos == windowSize) {
                    copyPos = 0;
                }
            }

            /*  Resets the count once the memory limit is exceeded in allocPtr,
                so to speak resets it at startup for reuse */
            incrBit += copyCnt;
            if (incrBit >= windowSize) {
                incrBit -= windowSize;
            }
        }

        dec_byte = StartDecodeByte();
    }

    Z_Free(window);

    //PRINTF_D2(WHITE, 0, 21, "DecodeD64:End");
}

// ENCODE MODE BY GEC Erick Vasquez Garcia (ERICK194)
// Updated version, compresses too fast and exact.

void StartEncodeCode(int lookup)
{
    // Encodes data from a table
    int lookupCode, lookupCheck;
    short* oddTbl;
    short* incrTbl;
    int binCnt = 0;
    byte binary[64];
    //static std::vector<byte> binary;

    //binary.clear();
    oddTbl = &DecodeTable[629];
    incrTbl = &DecodeTable[1258];

    lookupCode = lookup + 0x275;
    while (1) {

        if (lookupCode <= 1) { break; }

        lookupCheck = oddTbl[incrTbl[lookupCode]];

        //binary.push_back((lookupCheck == lookupCode) ? 1 : 0);
        binary[binCnt++] = (lookupCheck == lookupCode) ? 1 : 0;

        lookupCode = incrTbl[lookupCode];
    };

    // Write bits in bytes
    while (binCnt) {
        WriteBinary(binary[--binCnt]);
    }

    /*auto it = binary.rbegin();
    for (int k = 0; k < (int)binary.size(); k++) {
        WriteBinary(*it++);
    }*/

    // Update table
    UpdateTables(lookup);
}

unsigned char* EncodeD64(unsigned char* input, int inputlen, int* size) {
    int i, readPos, nodePos, looKupCode, byteData;
    int cpyCountNext, cpyCount, cntMin, cntMax;
    boolean deleteNode, skipCopy;

    unsigned char* output = (unsigned char*)malloc(inputlen * 2);

    InitTables();

    OVERFLOW_READ = inputlen;
    OVERFLOW_WRITE = inputlen * 2;

    deleteNode = false;
    skipCopy = false;
    readPos = encArgs.incrBit;
    nodePos = 0;

    buffers.input = buffers.istart = input;
    buffers.output = buffers.ostart = output;

    hashtable = (short*)Z_Alloc(HASH_SIZE * sizeof(short), PU_STATIC, NULL);
    hashtarget = (short*)Z_Alloc(HASH_SIZE * sizeof(short), PU_STATIC, NULL);
    hashNext = (short*)Z_Alloc(windowSize * sizeof(short), PU_STATIC, NULL);
    hashPrev = (short*)Z_Alloc(windowSize * sizeof(short), PU_STATIC, NULL);
    D_memset(hashNext, 0, windowSize * sizeof(short));
    D_memset(hashPrev, 0, windowSize * sizeof(short));

    window = (byte*)malloc(windowSize * sizeof(byte));
    D_memset(window, 0, windowSize * sizeof(byte));

    // Initializes hash encoding tables
    for (i = 0; i < HASH_SIZE; i++) {
        hashtable[i] = -1;
        hashtarget[i] = -1;
    }

    // Copy the first 3 bytes in the data window
    for (i = 0; i < encArgs.incrBit; i++) {
        byteData = ReadByte();
        if (byteData != -1) {
            // Encode a simple byte data.
            StartEncodeCode(byteData);
            window[i] = byteData;
        }
    }

    if (GetReadSize() < OVERFLOW_READ) { // Does not copy any data if the input data reached the limit
        // Copy the first 64 bytes in the data window
        for (i = 0; i < 64; i++) {
            byteData = ReadByte();

            // For some unknown reason if a byte is greater than or equal to 128 the form of the encoding changes
            // This happens depending on the file an example the BLANK and SPACEO texture results in 0
            // Textures such as C101, result in 1
            // MAP01, result in 1
            if (byteData >= 128) {
                encArgs.type = 1;
            }

            if (byteData != -1) {
                window[encArgs.incrBit++] = byteData;
            }
        }
    }

    // Determines the number of bytes to compare
    cntMin = (encArgs.type == 1) ? 20 : 50;
    cntMax = (encArgs.type == 1) ? 200 : 1000;

    if (GetReadSize() < OVERFLOW_READ) { // Does not encode any data if the input data reached the limit

        while (readPos != encArgs.incrBit) {
            // Add the data in the node directory table
            // update the hash table
            InsertNodeDirectory(nodePos);

            if (!skipCopy) {

                cpyCountNext = FindMatch(readPos + 1, cntMin);
                cpyCount = FindMatch(readPos, cntMax);

                //printf("cpyCount %d\n", cpyCount);
                //printf("cpyCountNext %d\n", cpyCountNext);

                // Check if there is a possibility to copy the data by routes
                if (cpyCount >= 3 && cpyCount >= cpyCountNext) {

                    int count = cpyCount;
                    int ValExtra = encArgs.offset;
                    int Shift = 0x04;

                    if ((ValExtra >= offsetTable[1]) && (ValExtra < offsetTable[2]))
                    {
                        Shift = 0x06;
                        ValExtra -= offsetTable[1];
                    }
                    if ((ValExtra >= offsetTable[2]) && (ValExtra < offsetTable[3]))
                    {
                        Shift = 0x08;
                        ValExtra -= offsetTable[2];
                    }
                    if ((ValExtra >= offsetTable[3]) && (ValExtra < offsetTable[4]))
                    {
                        Shift = 0x0A;
                        ValExtra -= offsetTable[3];
                    }
                    if ((ValExtra >= offsetTable[4]) && (ValExtra < offsetTable[5]))
                    {
                        Shift = 0x0C;
                        ValExtra -= offsetTable[4];
                    }
                    if ((ValExtra >= offsetTable[5]))
                    {
                        Shift = 0x0E;
                        ValExtra -= offsetTable[5];
                    }

                    if (Shift == 0x04) { looKupCode = (0x0101 + (count - 3)); }
                    else if (Shift == 0x06) { looKupCode = (0x013F + (count - 3)); }
                    else if (Shift == 0x08) { looKupCode = (0x017D + (count - 3)); }
                    else if (Shift == 0x0A) { looKupCode = (0x01BB + (count - 3)); }
                    else if (Shift == 0x0C) { looKupCode = (0x01F9 + (count - 3)); }
                    else if (Shift == 0x0E) { looKupCode = (0x0237 + (count - 3)); }

                    //printf("Code 0x%04X ValExtra 0x%04X\n", LooKupCode2, ValExtra);

                    // Writes the startup code and address to copy data from the data window.
                    StartEncodeCode(looKupCode);
                    // Writes the amount of data to copy from the data window.
                    WriteCodeBinary(ValExtra, Shift);

                    skipCopy = true;
                }
                else { // Encode a simple byte data.
                    StartEncodeCode(window[readPos]);
                }
            }

            // Skip copy data
            if (--cpyCount == 0) {
                skipCopy = false;
            }

            // Advance to the next position to check in matching
            if (++readPos == windowSize) {
                readPos = 0;
            }

            // Advance to the next position to add to the nodes of the directory
            if (++nodePos == windowSize) {
                nodePos = 0;
            }

            // Copy input data to the data window
            if (GetReadSize() < OVERFLOW_READ) { // Does not copy any data if the input data reached the limit
                byteData = ReadByte();
                if (byteData != -1) {
                    window[encArgs.incrBit++] = byteData;
                }

                // Enables the deletion of nodes in the directory once the first block of the data window is reached
                if (encArgs.incrBit == windowSize) {
                    encArgs.incrBit = 0;
                    deleteNode = true;
                }
            }

            // Delete the data in the node directory table
            if (deleteNode && GetReadSize() < OVERFLOW_READ) {
                DeleteNodeDirectory(encArgs.incrBit);
            }
        }
    }

    // Write the code to finish the encoding
    StartEncodeCode(0x100);
    FlushBitBuffer();

    *size = GetOutputSize();

    Z_Free(hashtable);
    Z_Free(hashtarget);
    Z_Free(hashNext);
    Z_Free(hashPrev);
    Z_Free(window);

    return output;
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
