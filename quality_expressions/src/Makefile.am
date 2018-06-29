lib_LTLIBRARIES += libqualexpr.la
libqualexpr_la_REVISION:=$(shell cd $(top_srcdir) ; git log -n 1 --format="format:-%t" quality_expressions 2>/dev/null)
libqualexpr_la_TAG:=$(shell head -1 $(top_srcdir)/quality_expressions/VERSION)$(libqualexpr_la_REVISION)
libqualexpr_la_VERSION_MAJOR:=$(shell echo $(libqualexpr_la_TAG) | cut -d. -f1)
libqualexpr_la_VERSION_MINOR:=$(shell echo $(libqualexpr_la_TAG) | cut -d. -f2 | cut -d- -f1)
libqualexpr_la_VERSION:=$(libqualexpr_la_VERSION_MAJOR):$(libqualexpr_la_VERSION_MINOR):0

libqualexpr_la_CXXFLAGS = ${global_compiler_flags} \
                          -std=c++14 \
                          ${PSC_BOOST_CPPFLAGS} \
                          -rdynamic -Wl,-export-dynamic \
                          -I$(top_srcdir)/quality_expressions/include \
                          -I$(top_srcdir)/quality_expressions/src/qualexpr-profiler/include \
                          -I$(top_srcdir)/quality_expressions/src/qualexpr-evaluator/include \
                          -DTAG=\"$(libqualexpr_la_TAG)\" \
                          -Wall # -Werror

libqualexpr_la_SOURCES = quality_expressions/src/qualexpr-evaluator/QualExprEvaluatorParser.yy \
                         quality_expressions/src/qualexpr-evaluator/QualExprEvaluatorLexer.ll \
                         quality_expressions/src/qualexpr-evaluator/QualExprEvaluator.cc \
                         quality_expressions/src/qualexpr-evaluator/QualExprManager.cc \
                         quality_expressions/src/qualexpr-profiler/QualExprProfiler.cc \
                         quality_expressions/src/QualityExpressions.cc \
                         quality_expressions/src/QualityExpressionsDB.cc \
                         quality_expressions/src/QualityExpressionsDesk.cc \
                         quality_expressions/src/QualExprSemantic.cc \
                         quality_expressions/src/QualityExpressionsProfilerLocal.cc \
                         quality_expressions/src/QualExprSemanticLocal.cc \
                         quality_expressions/src/QualityExpressionsProfilerPAPI.cc \
                         quality_expressions/src/QualExprSemanticPAPI.cc

libqualexpr_la_LDFLAGS = -static -version-info $(libqualexpr_la_VERSION)

CLEANFILES= quality_expressions/src/qualexpr-evaluator/QualExprEvaluatorParser.yy

quality_expressions/src/qualexpr-evaluator/QualExprEvaluatorLexer.cc: quality_expressions/src/qualexpr-evaluator/QualExprEvaluatorLexer.ll
	$(AM_V_LEX)$(am__skiplex) $(SHELL) $(YLWRAP) $< $(LEX_OUTPUT_ROOT).c $@ -- $(LEXCOMPILE)
	@mv QualExprEvaluatorLexer.h QualExprEvaluatorLexer.cc quality_expressions/src/qualexpr-evaluator/

quality_expressions-clean:
	rm -f $(libqualexpr_la_OBJECTS) quality_expressions/src/qualexpr-evaluator/QualExprEvaluator{Parser,Lexer}.{h,hh,cc,yy}

quality_expressions/src/qualexpr-evaluator/QualExprEvaluatorParser.yy: quality_expressions/src/qualexpr-evaluator/QualExprEvaluatorParser.yy.m4
	m4 -DBISON_VERSION=$(BISON_VERSION) $< > $@
