#!smake
include $(ROOT)/usr/include/make/PRdefs

FINAL = YES

ifeq ($(FINAL), YES)
	OPTIMIZER       = -O2
	LCDEFS          = -DNDEBUG -D_FINALROM -DF3DEX_GBI
ifeq ($(PLATFORM), BB)
	N64LIB          = -lultra_rom
else
	N64LIB          = -lgultra_rom
endif
else
	OPTIMIZER       = -g3
	LCDEFS          = -DF3DEX_GBI -DDEBUG
ifeq ($(PLATFORM), BB)
	N64LIB          = -lultra_d
else
	N64LIB          = -lgultra_d
endif
endif

APP	=	doom64.out

TARGETS	=	doom64.n64

ASMFILES   =	wessint_s.s
#m_fixed_s.s

CODEFILES   =	graph.c asci.c \
				doominfo.c \
				sprinfo.c \
				m_bbox.c \
				am_main.c \
				doomlib.c \
				vsprintf.c \
				c_convert.c \
				d_main.c \
				f_main.c \
				g_game.c \
				in_main.c \
				m_fixed.c \
				i_main.c \
				m_main.c \
				m_password.c \
				p_base.c \
				p_misc.c \
				p_telept.c \
				p_ceilng.c \
				p_change.c \
				p_doors.c \
				p_enemy.c \
				p_floor.c \
				p_inter.c \
				p_lights.c \
				p_map.c \
				p_slide.c \
				p_shoot.c \
				p_maputl.c \
				p_mobj.c \
				p_move.c \
				p_plats.c \
				p_pspr.c \
				p_setup.c \
				p_sight.c \
				p_spec.c \
				p_macros.c \
				p_switch.c \
				p_tick.c \
				p_user.c \
				r_data.c \
				r_main.c \
				r_phase1.c \
				r_phase2.c \
				r_phase3.c \
				s_sound.c \
				st_main.c \
				d_screens.c \
				w_wad.c \
				z_zone.c \
				decodes.c \
				tables.c \
				audio.c \
				wessshell.c \
				wesstwek.c \
				wessapi.c \
				wessapic.c \
				wessapim.c \
				wessapip.c \
				wessapit.c \
				wesshand.c \
				wesstrak.c \
				wessedit.c \
				wessarc.c \
				funqueue.c \
				wessfade.c \
				wessseq.c \
				n64cmd.c \
				seqload.c \
				seqloadl.c \
				seqloadr.c

WESSLIB   =	#wesslib.o

CODEOBJECTS =	$(CODEFILES:.c=.o) $(ASMFILES:.s=.o) $(WESSLIB)

DATAFILES   =	mem_heap.c audio_heap.c cfb.c

DATAOBJECTS =	$(DATAFILES:.c=.o)

CODESEGMENT =	codesegment.o

OBJECTS =	$(CODESEGMENT) $(DATAOBJECTS)

LCINCS =	-I. -I$(ROOT)/usr/include/PR

LDIRT  =	$(APP)

PATH_PRINC =	$(ROOT)/usr/include/PR
PATH_PRLIB =	$(ROOT)/usr/lib/PR

LCOPTS	=	-G 0
LDFLAGS	=	-L$(ROOT)/usr/lib $(N64LIB)
ASFLAGS	=	-non_shared -g -I$(PATH_PRINC) -I$(PATH_INC)

ifeq ($(PLATFORM), BB)
LCOPTS	+=	-mips2
LDFLAGS	+=	-lgcc
ASFLAGS	+=	-mips2
else
LDFLAGS	+=	-L$(GCCDIR)/mipse/lib -lkmc
ASFLAGS	+=	-mips3
endif

default:	$(TARGETS)

include $(COMMONRULES)

$(CODESEGMENT):	$(CODEOBJECTS)
		$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS)

$(TARGETS) $(APP):	spec $(OBJECTS)
ifeq ($(FINAL), YES)
	$(MAKEROM) -s 64 -r $(TARGETS) spec
	makemask $(TARGETS)
else
	$(MAKEROM) -r $(TARGETS) spec
endif
