#include <asm.h>
#include <PR/R4300.h>
#include <PR/ultratypes.h>

#define zero	$0
#define at	$1
#define v0	$2
#define v1	$3
#define a0	$4
#define a1	$5
#define a2	$6
#define a3	$7
#define	t0	$8
#define	t1	$9
#define	t2	$10
#define	t3	$11
#define	t4	$12
#define	t5	$13
#define	t6	$14
#define	t7	$15
#define s0	$16
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define t8	$24
#define t9	$25
#define k0	$26
#define k1	$27
#define gp	$28
#define sp	$29
#define fp	$30
#define ra	$31
#define return	s7

    .text
	.set noreorder
	.option pic0

/* Original Code From Mortal Kombat Trilogy N64 */

/*------------------------------------------*/
/* unsigned long wesssys_disable_ints(void) */
/*------------------------------------------*/

.globl wesssys_disable_ints
.ent wesssys_disable_ints

wesssys_disable_ints:   /* 8003A3E0 */

    mfc0    $8, $12     /* t0 = special register with IE bit */
    and     $9, $8, ~1  /* t1 = same register with IE bit cleared */
    mtc0    $9, $12     /* disable R4300 CPU interrupts */
    andi    $2, $8, 1   /* return the prior state of just the IE bit */
    nop
    jr      $31
    nop

.end wesssys_disable_ints

/*------------------------------------------------*/
/* void wesssys_restore_ints(unsigned long state) */
/*------------------------------------------------*/

.globl wesssys_restore_ints
.ent wesssys_restore_ints

wesssys_restore_ints:   /* 8003A400 */

    mfc0	$8,$12      /* t0 = special register with IE bit */
	nop
	or      $8, $4      /* restore IE bit from passed-in parameter */
	mtc0	$8, $12     /* restore R4300 CPU interrupts */
	nop
	nop
	j		$31         /* return nothing */
	nop

.end wesssys_restore_ints
