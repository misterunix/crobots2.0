D			[0-9]
L			[a-zA-Z_]


%{

/*****************************************************************************/
/*                                                                           */
/*  CROBOTS                                                                  */
/*                                                                           */
/*  (C) Copyright Tom Poindexter, 1985, all rights reserved.                 */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

/* lexical analyzer for crobots */

#include "tokens.h"
#include "crobots.h"
#include "compiler.h"

/* redefine i/o */
#undef ECHO
#define ECHO fprintf(f_out,"%s",yytext)

long atol();

%}

%x C_COMMENT


%%
"/*"            { BEGIN(C_COMMENT); }
<C_COMMENT>"*/" { BEGIN(INITIAL); }
<C_COMMENT>.    { }


"auto"			{ count(); return(AUTO); }
"break"			{ count(); return(BREAK); }
"else"			{ count(); return(ELSE); }
"extern"		{ count(); return(EXTERN); }
"for"			{ count(); return(FOR); }
"if"			{ count(); return(IF); }
"int"			{ count(); return(INT); }
"long"			{ count(); return(LONG); }
"register"		{ count(); return(REGISTER); }
"return"		{ count(); return(RETURN); }
"while"			{ count(); return(WHILE); }

{L}({L}|{D})*		{ count(); 
				strncpy(last_ident,yytext,ILEN-1);
				last_ident[ILEN-1] = '\0';
				return(IDENTIFIER); }

{D}+     		{ count(); 
				kk = atol(yytext);
				return(CONSTANT); }

">>="			{ count(); return(RIGHT_ASSIGN); }
"<<="			{ count(); return(LEFT_ASSIGN); }
"+="			{ count(); return(ADD_ASSIGN); }
"-="			{ count(); return(SUB_ASSIGN); }
"*="			{ count(); return(MUL_ASSIGN); }
"/="			{ count(); return(DIV_ASSIGN); }
"%="			{ count(); return(MOD_ASSIGN); }
"&="			{ count(); return(AND_ASSIGN); }
"^="			{ count(); return(XOR_ASSIGN); }
"|="			{ count(); return(OR_ASSIGN); }
">>"			{ count(); return(RIGHT_OP); }
"<<"			{ count(); return(LEFT_OP); }
"++"			{ count(); return(INC_OP); }
"--"			{ count(); return(DEC_OP); }
"&&"			{ count(); return(AND_OP); }
"||"			{ count(); return(OR_OP); }
"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
";"			{ count(); return(';'); }
"{"			{ count(); return('{'); }
"}"			{ count(); return('}'); }
","			{ count(); return(','); }
"="			{ count(); return('='); }
"("			{ count(); return('('); }
")"			{ count(); return(')'); }
"."			{ count(); return('.'); }
"&"			{ count(); return('&'); }
"!"			{ count(); return('!'); }
"~"			{ count(); return('~'); }
"-"			{ count(); return('-'); }
"+"			{ count(); return('+'); }
"*"			{ count(); return('*'); }
"/"			{ count(); return('/'); }
"%"			{ count(); return('%'); }
"<"			{ count(); return('<'); }
">"			{ count(); return('>'); }
"^"			{ count(); return('^'); }
"|"			{ count(); return('|'); }

[ \t\v\n\f]		{ count(); }
.			{ /* ignore bad characters */ }

%%

int yywrap( void )
{
	return(1);
}

void count( void )
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			column = 0;
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;

	ECHO;
}

/* these two dummy routines for lex's debug options */
void allprint( void ) {}
void sprint( void ) {} 

