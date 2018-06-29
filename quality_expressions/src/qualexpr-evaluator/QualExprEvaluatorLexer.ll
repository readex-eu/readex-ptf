/**
   @file    QualExprEvaluatorLexer.ll
   @ingroup QualityExpressionEvaluation
   @brief   Main quality expressions lexer
   @author  Laurent Morin
   @verbatim
   Revision:       $Revision$
   Revision date:  $Date$
   Committed by:   $Author$

   This file is part of the Periscope performance measurement tool.
   See http://www.lrr.in.tum.de/periscope for details.

   Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
   See the COPYING file in the base directory of the package for details.

   @endverbatim
*/

%{ /* -*- C++ -*- */
# include <string>
# include "qualexpr-evaluator/QualExprEvaluatorParserDriver.h"

  typedef quality_expressions_core::QualExprEvaluatorParsingDriver driver_t;

# undef yywrap
# define yywrap(scanner) 1

#define yyterminate() return driver_t::Token::END
%}

/* Lexer header file */
%option header-file="QualExprEvaluatorLexer.h"
/* Lexer implementation file */
%option outfile="QualExprEvaluatorLexer.cc"
/* Lexer Prefix */
%option prefix="qualexp_"
/* Need to be reentrant */
%option reentrant
/* No need for includes */
%option noyywrap
/* No need to unput in buffer */
%option nounput
/* Not an interactive lexer, takes a file */
%option batch
/* Debug */
%option debug

/* ***************************************************** */
/* **************** MACROS ******************* */

id    [a-zA-Z][a-zA-Z_0-9]*
int   [0-9]+
blank [ \t]

%{
using namespace quality_expressions_core;
# define YY_USER_ACTION  driver.user_action(*yylloc, yyleng);
%}

%%
%{
  driver.lex_step(*yylloc);
%}

{blank}+   driver.lex_step(*yylloc);
[\n]+      driver.new_line(*yylloc, yyleng);

%{
%}

"<="           			return driver_t::Token::LE;
">="           			return driver_t::Token::GE;
"=="           			return driver_t::Token::EQ;
[-+*/;:<>()~|!]			return driver_t::TokenType(yytext[0]);
{int}          			return driver.scan_integer(*yylval, *yylloc, yytext);
[a-zA-Z_][a-zA-Z_0-9-]*         return driver.scan_identifier(*yylval, *yylloc, yytext);
.				driver.error (*yylloc, "invalid character");
%%
struct yy_buffer_state * qualexp_get_current_buffer(yyscan_t yyscanner) {
  struct yyguts_t * yyg = (struct yyguts_t*) yyscanner;
  return (struct yy_buffer_state *) YY_CURRENT_BUFFER;
}
