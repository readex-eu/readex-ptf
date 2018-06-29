/**
   @file    QualExprEvaluatorParser.yy
   @ingroup QualityExpressionEvaluation
   @brief   Main quality expressions parser
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

ifdef(BISON_VERSION, , [define(BISON_VERSION, 24)])
changecom('m4dnl')

%skeleton "lalr1.cc" /* -*- C++ -*- */
%defines
ifelse(BISON_VERSION, 23, %define "parser_class_name" "QualExprEvaluatorParser", %define parser_class_name "QualExprEvaluatorParser")
ifelse(BISON_VERSION, 23, %define "namespace" "quality_expressions_core", %define api.prefix "quality_expressions_core")
// ** The parsing context. **
ifelse(BISON_VERSION, 23, %{, %code requires {)
# include <stdio.h>
# include <string>
# include <sstream>

#define `BISON_VERSION' BISON_VERSION `'

  // Standard Lexer interface -- used by the BisonFlexInterface toolbox.
#define			LEXER_INTERFACE(prefix)                                                                         \
  typedef struct yy_buffer_state *	prefix ## buffer_t;		/* Opaque buffer pointer. */                    \
  typedef void *	prefix ## scan_t;				/* Opaque lexer pointer. */                     \
  typedef 		prefix ## scan_t extern_scan_t;			/* Opaque pointer for the grammar. */           \
  int			prefix ## lex_init(prefix ## scan_t* scanner);                                                  \
  int			prefix ## lex_destroy(prefix ## scan_t scanner);                                                \
  FILE *		prefix ## get_in(prefix ## scan_t scanner);                                                     \
  void			prefix ## set_in(FILE * in_str, prefix ## scan_t scanner);                                      \
  int			prefix ## get_debug(prefix ## scan_t scanner);                                                  \
  void			prefix ## set_debug(int debug_flag, prefix ## scan_t scanner);                                  \
  prefix ## buffer_t	prefix ## get_current_buffer(prefix ## scan_t scanner);                                         \
  prefix ## buffer_t	prefix ## _scan_string(const char *str, prefix ## scan_t scanner);                              \
  void			prefix ## _delete_buffer(prefix ## buffer_t buffer, prefix ## scan_t scanner);                  \
  void			prefix ## _switch_to_buffer(prefix ## buffer_t buffer, prefix ## scan_t scanner);               \

  namespace quality_expressions_core
  {
    class location;
    class QualExprEvaluatorParser;
    class QualExprEvaluatorParsingDriver;
    template<typename kind> class QualExprComputeNodeOf;
    typedef long long long64_t;

#define					yy(fct) qualexp_ ## fct
#define DECLARE_LEXER_INTERFACE		LEXER_INTERFACE(qualexp_);
#define YY_DECL                                                                                                                                      \
    ParserInterface<quality_expressions_core::QualExprEvaluatorParser>::TokenType                                                                    \
    quality_expressions_corelex(ParserInterface<quality_expressions_core::QualExprEvaluatorParser>::SemanticType* yylval,                            \
                                ParserInterface<quality_expressions_core::QualExprEvaluatorParser>::LocationType* yylloc,                            \
                                quality_expressions_core::QualExprEvaluatorParsingDriver& driver,                                                    \
                                extern_scan_t yyscanner                                                                                              \
                                )
  }
  typedef quality_expressions_core::location			location_t;

  DECLARE_LEXER_INTERFACE;

  // Parser intermediate data structures
  typedef struct semantic_desc_t {
    const std::string *	m_namespace;
    const std::string *	m_name;
  } semantic_desc_t;

  typedef struct aggregator_desc_t {
    char		m_op;
    const std::string *	m_name;
  } aggregator_desc_t;

  // Operator templates
#define BIN_OPERATOR( name, computation, show )                                                                             \
  template<typename kind> class name {                                                                                      \
  public:	static inline kind compute(kind a, kind b)					{ return computation ; }    \
  public:	static inline void display(const std::string &indent, std::stringstream &s)	{ show; }                   \
  };

BIN_OPERATOR( list,   a,  s << ';' );
BIN_OPERATOR( lt,   a<b,  s << '<' );
BIN_OPERATOR( gt,   a>b,  s << '>' );
BIN_OPERATOR( le,   a<=b, s << "<=" );
BIN_OPERATOR( ge,   a>=b, s << ">=" );
BIN_OPERATOR( add,  a+b,  s << '+' );
BIN_OPERATOR( sub,  a-b,  s << '-' );
BIN_OPERATOR( mul,  a*b,  s << '*' );
BIN_OPERATOR( divi, b!=0 ? a/b : 0,  s << '/' );

ifelse(BISON_VERSION, 23, %}, })

%parse-param    { QualExprEvaluatorParsingDriver & driver }
%lex-param	{ QualExprEvaluatorParsingDriver & driver }
%parse-param    { extern_scan_t scanner }
%lex-param	{ extern_scan_t scanner }

// ** Location structure initialization **
%locations
%initial-action
{
// Initialize the initial location.
  @$.begin.filename = @$.end.filename = driver.getFilename();
};

// ** Debugging and tracing **
%debug
%error-verbose

// **********************************************
// **************** SEMANTIC DATA ***************
%union
 {
   long long           			ival;
   std::string *			sval;
   QualExprComputeNodeOf<long64_t> *	nodeLong;
   semantic_desc_t			semantic;
   aggregator_desc_t			aggregator;
};

// *****************************************************
// **************** CODE INSTRUMENTATION ***************
ifelse(BISON_VERSION, 23, %{, %code {)
# include "qualexpr-evaluator/QualExprEvaluator.h"
# include "qualexpr-evaluator/QualExprEvaluatorParserDriver.h"
ifelse(BISON_VERSION, 23, #define yylex quality_expressions_corelex)
ifelse(BISON_VERSION, 23, %}, })

// *****************************************************
// **************** TOKEN DEFINITION *******************

%token        END      0 "end of file"
%token        LE         "<="
%token        GE         ">="
%token        EQ         "=="
%token <ival> NUMBER     "number"
%token <sval> IDENTIFIER "identifier"

%token        time                             "time"
%token        size			       "size"
%token        bw			       "bw"

%type  <nodeLong>     	QualIndexExpression QualIndexExpressionList QualIndexEntryUnit
%type  <nodeLong>     	QualIndexEntryCompare QualIndexEntryAddSub QualIndexEntryMulDiv
%type  <nodeLong>     	QualIndexEntry
%type  <nodeLong>     	QualIndexAbsolute
%type  <nodeLong>     	QualIndexMeasure
%type  <nodeLong>     	QualIndexEventMeasure
%type  <semantic>	EventName
%type  <aggregator>	EventComputation

ifelse(BISON_VERSION, 23, , %printer    { yyoutput << *$$; } "identifier")
ifelse(BISON_VERSION, 23, , %printer    { yyoutput << $$; }  <ival>)
%destructor { delete $$; }       "identifier"

// *****************************************************
// **************** TOKEN DEFINITION *******************
%%
%start QualIndexExpression;
QualIndexExpression:
 /* Nothing.  */        					{ driver.setResult(NULL); }
|	QualIndexExpressionList					{ driver.setResult($$); }
;

QualIndexExpressionList:
	QualIndexExpressionList ';' QualIndexEntryUnit		{ $$ = new QualExprComputeNodeBinOp< long64_t, list<long64_t> >(*$1,*$3); }
|	QualIndexEntryUnit					{ $$ = $1; }
 	;

QualIndexEntryUnit:
	QualIndexEntryCompare					{ $$ = $1; }
;

QualIndexEntryCompare:
	QualIndexEntryCompare '<' QualIndexEntryAddSub		{ $$ = new QualExprComputeNodeBinOp< long64_t, lt<long64_t> >(*$1,*$3); }
|	QualIndexEntryCompare '>' QualIndexEntryAddSub		{ $$ = new QualExprComputeNodeBinOp< long64_t, gt<long64_t> >(*$1,*$3); }
|	QualIndexEntryCompare "<=" QualIndexEntryAddSub		{ $$ = new QualExprComputeNodeBinOp< long64_t, le<long64_t> >(*$1,*$3); }
|	QualIndexEntryCompare ">=" QualIndexEntryAddSub		{ $$ = new QualExprComputeNodeBinOp< long64_t, ge<long64_t> >(*$1,*$3); }
|	QualIndexEntryAddSub					{ $$ = $1; }
 	;

QualIndexEntryAddSub:
	QualIndexEntryAddSub '+' QualIndexEntryMulDiv		{ $$ = new QualExprComputeNodeBinOp< long64_t, add<long64_t> >(*$1,*$3); }
|	QualIndexEntryAddSub '-' QualIndexEntryMulDiv		{ $$ = new QualExprComputeNodeBinOp< long64_t, sub<long64_t> >(*$1,*$3); }
|	QualIndexEntryMulDiv					{ $$ = $1; }
 	;

QualIndexEntryMulDiv:
	QualIndexEntryMulDiv '*' QualIndexEntry			{ $$ = new QualExprComputeNodeBinOp< long64_t, mul<long64_t> >(*$1,*$3); }
|	QualIndexEntryMulDiv '/' QualIndexEntry			{ $$ = new QualExprComputeNodeBinOp< long64_t, divi<long64_t> >(*$1,*$3); }
|	QualIndexEntry						{ $$ = $1; }
 	;

QualIndexEntry:
	QualIndexAbsolute					{ $$ = $1; }
|	QualIndexMeasure					{ $$ = $1; }
|	'(' QualIndexEntryUnit ')'				{ $$ = $2; }
 	;

QualIndexAbsolute:
	"number"						{ $$ = new QualExprComputeNodeImmediate<long64_t>($1); }
	;

QualIndexMeasure:
	QualIndexEventMeasure					{ $$ = $1; }
	;

QualIndexEventMeasure:
	EventName ':' EventComputation				{ $$ = driver.buildSemanticAggregator($1, $3); }
|	EventName						{ $$ = driver.buildSemanticAggregator($1, driver.buildAggregator('!', NULL)); }
	;

EventName:
	IDENTIFIER ':' ':' IDENTIFIER				{ $$ = driver.buildSemantic($1, $4); }
	;

EventComputation:
	'|' IDENTIFIER						{ $$ = driver.buildAggregator('|', $2); }
 |	'~' IDENTIFIER						{ $$ = driver.buildAggregator('~', $2); }
 |	'+' IDENTIFIER						{ $$ = driver.buildAggregator('+', $2); }
 |	'-' IDENTIFIER						{ $$ = driver.buildAggregator('-', $2); }
 |	'!'							{ $$ = driver.buildAggregator('!', NULL); }
	;
%%

namespace quality_expressions_core
{
  void QualExprEvaluatorParser::error (const QualExprEvaluatorParsingDriver::LocationType & l, const std::string& m)
  {
    driver.error(l, m);
  }

  /* Constructor */ QualExprEvaluatorParsingDriver::QualExprEvaluatorParsingDriver(QualExprSemanticAggregatorDB &semanticAggregatorDB) :
    m_semanticAggregatorDB(semanticAggregatorDB), m_resultNode(NULL) {}

  semantic_desc_t QualExprEvaluatorParsingDriver::buildSemantic(const std::string *snamespace, const std::string *name)
  {
    semantic_desc_t semantic = { snamespace, name };
    return semantic;
  }

  aggregator_desc_t QualExprEvaluatorParsingDriver::buildAggregator(const char op, const std::string *name)
  {
    aggregator_desc_t aggreg = { op, name };
    return aggreg;
  }

  QualExprComputeNodeOf<long64_t> * QualExprEvaluatorParsingDriver::buildSemanticAggregator(semantic_desc_t semantic, aggregator_desc_t aggreg)
  {
    std::string eventName = *semantic.m_namespace;
    eventName += "::";
    eventName += *semantic.m_name;
    delete semantic.m_namespace;
    delete semantic.m_name;

    std::string aggregName;
    aggregName += aggreg.m_op;
    if (aggreg.m_name) aggregName += *aggreg.m_name;
    delete aggreg.m_name;

    QualExprComputeNodeOf<long64_t> * newAggregNode = NULL;
    try {
      QualExprSemanticAggregator & newAggreg = m_semanticAggregatorDB.pushAggregator(eventName, aggregName);
      newAggregNode = new QualExprComputeNodeAggreg<long64_t>(newAggreg);
    }
    catch (QualExprSemanticAggregatorDB::Exception e) {
      error(e.what());
    }
    return newAggregNode;
  }

  int QualExprEvaluatorParsingDriver::parse(const std::string &filename, const std::string &buffer)
  {
    scanner_t scan = begin_parse(filename, buffer);
    Grammar parser (*this, scan);
    parser.set_debug_level (getParsingTrace());
    int error = parser.parse();
    end_parse();
    return error;
  }
}
