

/*****************************************************************************/
/*                                                                           */
/*  CROBOTS                                                                  */
/*                                                                           */
/*  (C) Copyright Tom Poindexter, 1985, all rights reserved.                 */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define YY_NO_INPUT
/* common defines */

/* note-the INIT flag (or lack of it) causes extern for all but one module */

#define ILEN      8		/* length of identifiers, also in lexanal.l */
#define MAXROBOTS 4		/* maximum number of robots */
#define CODESPACE 1000		/* maximum number of machine instructions */
#define DATASPACE 500		/* maximum number of data stack entries */
#define UPDATE_CYCLES 30  	/* number of cycles before screen update */
#define MOTION_CYCLES 15 	/* number of cycles before motion update */
#define CYCLE_LIMIT 500000L	/* limit of background cycles */
#define ROBOT_SPEED 7		/* multiplicative speed factor */
#define TURN_SPEED 50		/* maximum speed for direction change */
#define ACCEL  10		/* acceleration factor per motion cycle */

#define ACTIVE    1		/* robot active & running */
#define DEAD      0		/* dead robot */

struct robot {			/* robot context */
  int status;			/* status of robot, active or dead */
  char name[14];		/* name of robot */
  int x;			/* current x location * 100 */
  int y;			/* current y location * 100 */
  int org_x;			/* orgin x location * 100 */
  int org_y;			/* orgin y location * 100 */
  int range;			/* distance traveled on this heading * 100 */
  int last_x;			/* last plotted physical x */
  int last_y;			/* last plotted physical y */
  int speed;			/* current speed, 0 - 100 */
  int last_speed;		/* last speed, 0 - 100 */
  int accel;			/* linear acceleration / de-acceleration lag */
  int d_speed;			/* desired speed */
  int heading;			/* current heading, 0 - 359 */
  int last_heading;		/* last heading, 0 - 359 */
  int d_heading;		/* desired heading, 0 - 359 */
  int damage;			/* damage sustained, percent */
  int last_damage;		/* last damage */
  int scan;			/* current scan direction */
  int last_scan;		/* last scan direction */
  int reload;			/* number of cycles between reloading */
  int ext_count;		/* size of external pool needed */
  long *external;		/* external variable pool */
  long *local;			/* current local variables on stack */
  long *stackbase;		/* base of local & expression stack */
  long *stackend;		/* end of stack */
  long *stackptr;		/* current stack pointer, grows up */
  long *retptr;			/* return frame pointers, grow down */
  char *funcs;			/* table of function names by offset */
  struct func *code_list;	/* list of function headers */
  struct instr *code;		/* machine instructions, actually instr */
  struct instr *ip; 		/* instruction pointer */
};

struct func {			/* function header */
  struct func *nextfunc;	/* next function header in chain */
  char func_name[ILEN];		/* function name */
  struct instr *first;		/* first instruction pointer */
  int var_count;		/* number of pool variables needed */
  int par_count;		/* number of parameters expected */
};

struct instr {			/* robot machine instruction */
  char ins_type;		/* instruction type */
  union {
    long k;			/* constant value */
    short int var1;		/* variable offset, function offset, operator */
    struct instr *br;		/* false branch */
    struct {
      short int var2;		/* assignment variable offset */
      short int a_op;		/* assignment operator */
    } a;
  } u;
};


/* missile constants */
#define MIS_SPEED 500		/* how far in one motion cycle (in clicks) */
#define MIS_RANGE 700 		/* maximum missile range (see MAX_X, MAX_Y */
#define MIS_ROBOT 2		/* number of active missiles per robot */
#define AVAIL  0		/* missile available for use */
#define FLYING 1		/* missile in air */
#define EXPLODING 2		/* missile exploding on ground */
#define RELOAD 15		/* motion cycles before reload */
#define EXP_COUNT 5 		/* motion cycles for exploding missile */

struct missile {			/* active missiles */
  int stat;			/* missile status */
  int beg_x;			/* beginning x * 100 */
  int beg_y;			/* beginning y * 100 */
  int cur_x;			/* current x * 100 */
  int cur_y;			/* current y * 100 */
  int last_xx;			/* last plotted x */
  int last_yy;			/* last plotted y */
  int head;			/* heading, 0 - 359 */
  int count;			/* cycle count for exploding missiles */
  int rang;			/* range of missile */
  int curr_dist;		/* current distance from orgin * 100 */
};

#ifndef INIT
extern
#endif
struct missile missiles[MAXROBOTS][MIS_ROBOT];

#ifndef INIT
extern
#endif
struct robot *cur_robot,	/* current robot */
             robots[MAXROBOTS];	/* all robots */

#ifndef INIT
extern
#endif
int r_debug,			/* debug switch */
    r_flag;			/* global flag for push/pop errors */

/* instruction types */
#define NOP    0		/* end of code marker */
#define FETCH  1		/* push(varpool(offset)) */
#define STORE  2		/* push(pop op pop) --> varpool(offset) */
#define CONST  3		/* push(constant) */
#define BINOP  4		/* pop -->y, pop -->x, push(x op y) */
#define FCALL  5		/* pop --> parmn..parm1, save ip, call */
#define RETSUB 6		/* push(returnval), restore ip */
#define BRANCH 7		/* if (pop == 0) branch --> ip*/
#define CHOP   8		/* pop --> bit bucket */
#define FRAME  9		/* frame stack pointer for call */

/* external variable flag (or'ed in or and'ed out) , also in grammar.y */
#define EXTERNAL 0x8000

/* size of battlefield, see MIS_RANGE also. note - x and y coordinates */
/* of robots and missiles are measured in CLICKS * MAX_? */
#define CLICK 10		/* 10 clicks per meter */
#define MAX_X 1000		/* 1000 meters x axis */
#define MAX_Y 1000		/* 1000 meters y axis */

/* damage factors, percent */
#define DIRECT_HIT 10
#define NEAR_HIT   5
#define FAR_HIT    3
#define COLLISION  2
/* distance of damage factors */
#define DIRECT_RANGE 5
#define NEAR_RANGE   20
#define FAR_RANGE    40
  
void c_scan( void );
void c_cannon( void );
void c_drive( void );
void c_damage( void );
void c_speed( void );
void c_loc_x( void );
void c_loc_y( void );
void c_rand( void );
void c_sin( void );
void c_cos( void );
void c_tan( void );
void c_atan( void );
void c_sqrt( void );

/* declare instrinsic function table */
#ifndef INIT
extern
#endif
struct intrin {
  char *n;
  void (*f)(void);
} intrinsics[20]

#ifdef INIT
 = {
  {"*dummy*",	(void (*)()) 0},
  {"scan",	c_scan},
  {"cannon",	c_cannon},
  {"drive",	c_drive},
  {"damage",	c_damage},
  {"speed",	c_speed},
  {"loc_x",	c_loc_x},
  {"loc_y",	c_loc_y},
  {"rand",	c_rand},
  {"sin",	c_sin},
  {"cos",	c_cos},
  {"tan",	c_tan},
  {"atan",	c_atan},
  {"sqrt",	c_sqrt},
  {"",		(void (*)()) 0} 
 }
#endif
;

/* end of crobots.h header */

void robot_stat( int n );
void decinstr(struct instr *code);
void binaryop(int op);
void dumpvar(long *pool,int size);
void robot_go(struct robot *r);
void rand_pos( int n );
void end_disp( void );
void draw_field( void );
void plot_robot( int n );
void plot_miss( int r, int n);
void plot_exp(int r, int n);
void show_cycle( long l );
void init_disp( void );
void update_disp( void );
void count_miss( int i, int j ) ;
void init_comp( void ) ;
int reset_comp( void ) ;
int new_func( void );
void end_func( void ) ;
int allocvar(char *s, char *pool);
int efetch( int offset );
int estore( int offset, int operator );
int econst( long c );
void dumpoff( char *pool );
int findvar(char *s, char *pool);
int stackid( char *id, char *stack, int *ptr );
int popid( char *id, char *stack, int *ptr );
int poolsize( char *pool );
int ebinop( int c );
int efcall( int c );
int eretsub( void );
int ebranch( void );
int echop( void );
int eframe( void );
int new_if( void );
int else_part( void );
void close_if( void );
int new_while( void );
int while_expr( void );
int close_while( void );
void decompile( struct instr *code );
void printop( int op );
void yyerror( char *s );
int yywrap( void );
void count( void );
void allprint( void );
void sprint( void );

void cpu_trace( char *f );
void catch_int( void );

void comp( char **f, int n );
void play( char **f, int n );
void match( int m, long l, char **f, int n );
