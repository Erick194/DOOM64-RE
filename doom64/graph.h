/*====================================================================
  File  : graph.h

  Created   by Koji Mitsunari.    Mar,14 1996.
  Copyright by Nintendo, Co., Ltd.       1997.
  =====================================================================*/
#ifndef	_graph_h_
#define	_graph_h_

#define	BMP_DATA	0

#define WHITE		0xffffffff
#define BLACK		0x0000
#define GRAY		GPACK_RGBA5551(127,127,127,1)
#define BLUE		GPACK_RGBA5551(0,0,255,1)
#define GREEN		GPACK_RGBA5551(0,255,0,1)
#define RED		    GPACK_RGBA5551(255,0,0,1)
#define YELLOW		GPACK_RGBA5551(255,255,0,1)
#define DARK		GPACK_RGBA5551(50,50,128,1)
#define MARKCOLOR	GPACK_RGBA5551(50,250,50,1)
#define BGCOLOR		0x0000

#define CHAR_WD		8
#define CHAR_HT		16

#define TEXT_WD		SCREEN_WD/CHAR_WD
#define TEXT_HT		SCREEN_HT/CHAR_HT

extern	void	putchar(u32, int, int, char);
extern	void	revchar(u32, int, int, char);
extern	void	putint_h(u32, int, int, int, char);
extern	void	putint_d(u32, int, int, int);
extern	void	printstr(u32, int, int, char*);
extern	void	circle(int, int, int, u32);
extern	void	pset(int, int, u32);
extern	void	line(int, int, int, int, u32);
extern	void	box(int, int, int, int, u32);
extern	void	gcls(void);
extern	void	bigputc(u32, int, int, char, int, int);
extern	void	printbig(u32, int, int, char*, int, int);
extern	void	WriteBG(u8 *, u16 *);
extern	void	SetBG(u8 *, u16 *);
extern	void	gputchar(u32, int, int, char, u16 *);
extern	void	gprintstr(u32, int, int, char*, u16 *);
extern	void	putpattern(u32, int, int, char);

extern	u16	*__cursor;
extern	u32	__color;

extern	void	PRINTF_D(u32 color_, char *c_, ...);
extern	void	PRINTF_D2(u32 color_, int x, int y, char *c_, ...);
extern	void	WAIT(void);
#endif
