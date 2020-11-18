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

/*------------------------------------------*/
/* fixed_t	FixedMul (fixed_t a, fixed_t b) */
/*------------------------------------------*/

.globl FixedMul
.ent FixedMul

FixedMul:   /* 800044D0 */

    dmult   $4, $5
    mflo    $2
    dsra    $2, $2, 16
    jr      $31
    nop

.end FixedMul

/*-------------------------------------------*/
/* fixed_t	FixedDiv2 (fixed_t a, fixed_t b) */
/*-------------------------------------------*/

.globl FixedDiv2
.ent FixedDiv2

FixedDiv2:   /* 800044E4 */

    dsll    $4, $4, 16
    ddiv    $4, $5

/*---------------------------------------------------*/
/* automatically generated with opcode (ddiv $4, $5) */
/* and -mips3 on command line                        */
/*---------------------------------------------------*/
/*
    bne     $5, $0, loc_800044F8
    nop
    break   0x1C00

loc_800044F8:
    daddiu  $1, $0, 0xffff
    bne     $5, $1, loc_80004514
    daddiu  $1, $0, 0x0001
    dsll32  $1, $1, 0x1f
    bne     $4, $1, loc_80004514
    nop
    break   0x1800

loc_80004514:
    mflo    $4
*/
/*--------------------------------------------------*/

    mflo    $2
    jr      $31
    nop

.end FixedDiv2
