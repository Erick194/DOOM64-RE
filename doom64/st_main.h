
#define	FLASHDELAY	8		/* # of tics delay (1/30 sec) */
#define FLASHTIMES	6		/* # of times to flash new frag amount (EVEN!) */

typedef struct
{
    int active;
    int doDraw;
    int delay;
    int times;
} sbflash_t;

extern sbflash_t flashCards[6];    // 800A8180
extern boolean tryopen[6]; // 800A81E0

extern byte *sfontlump;     // 800A81F8
extern byte *statuslump;   // 800A81FC
extern int sumbolslump;    // 800A8204

extern int err_text_x;     // 800A8208
extern int err_text_y;     // 800A820C

#define FIRST_SYMBOL   0x80
#define LAST_SYMBOL    0x90 // 0x91 for Right arrow

typedef struct
{
    int      x;
    int      y;
    int      w;
    int      h;
} symboldata_t;

extern symboldata_t symboldata[];// 8005B260

void ST_Init(void); // 80029BA0
void ST_InitEveryLevel(void); // 80029C00
void ST_Ticker (void); // 80029C88
void ST_Drawer (void); // 80029DC0
void ST_Message(int x,int y,char *text,int color); // 8002A36C
void ST_DrawNumber(int x, int y, int val, int mode, int color); // 8002A79C
void ST_DrawString(int x, int y, char *text, int color); // 8002A930
int ST_GetCenterTextX(byte *text); // 8002AAF4
void ST_UpdateFlash(void); // 8002AC30
void ST_DrawSymbol(int xpos, int ypos, int index, int color); // 8002ADEC

// Debug
void ST_DebugSetPrintPos(int x, int y);
void ST_DebugPrint(const char *text, ...);
