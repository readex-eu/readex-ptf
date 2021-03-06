# Copyright (C) 2004 Oren Ben-Kiki
# This file is distributed under the same terms as the Automake macro files.

# Generate automatic documentation using Doxygen. Goals and variables values
# are controlled by the various DX_COND_??? conditionals set by autoconf.
#
# The provided goals are:
# doxygen-doc: Generate all doxygen documentation.
# doxygen-run: Run doxygen, which will generate some of the documentation
#              (HTML, CHM, CHI, MAN, RTF, XML) but will not do the post
#              processing required for the rest of it (PS, PDF, and some MAN).
# doxygen-man: Rename some doxygen generated man pages.
# doxygen-ps: Generate doxygen PostScript documentation.
# doxygen-pdf: Generate doxygen PDF documentation.
#
# Note that by default these are not integrated into the automake goals. If
# doxygen is used to generate man pages, you can achieve this integration by
# setting man3_MANS to the list of man pages generated and then adding the
# dependency:
#
#   $(man3_MANS): doxygen-doc
#
# This will cause make to run doxygen and generate all the documentation.
#
# The following variable is intended for use in Makefile.am:
#
# DX_CLEANFILES = everything to clean.
#
# This is usually added to MOSTLYCLEANFILES.

## --------------------------------- ##
## Format-independent Doxygen rules. ##
## --------------------------------- ##

if DX_COND_doc

## ------------------------------- ##
## Rules specific for HTML output. ##
## ------------------------------- ##

if DX_COND_html

DX_CLEAN_HTML = @DX_DOCDIR@/html

endif DX_COND_html

## ------------------------------ ##
## Rules specific for CHM output. ##
## ------------------------------ ##

if DX_COND_chm

DX_CLEAN_CHM = docs/chm

if DX_COND_chi

DX_CLEAN_CHI = docs/chi

endif DX_COND_chi

endif DX_COND_chm

## ------------------------------ ##
## Rules specific for MAN output. ##
## ------------------------------ ##

if DX_COND_man

DX_CLEAN_MAN = docs/man

endif DX_COND_man

## ------------------------------ ##
## Rules specific for RTF output. ##
## ------------------------------ ##

if DX_COND_rtf

DX_CLEAN_RTF = docs/rtf

endif DX_COND_rtf

## ------------------------------ ##
## Rules specific for XML output. ##
## ------------------------------ ##

if DX_COND_xml

DX_CLEAN_XML = docs/xml

endif DX_COND_xml

## ----------------------------- ##
## Rules specific for PS output. ##
## ----------------------------- ##

if DX_COND_ps

DX_CLEAN_PS = docs/pdf

DX_PS_GOAL = doxygen-ps

doxygen-ps: 

%.tex:
	mkdir -p @DX_DOCDIR@/pdf; cp docs/ptf_installation_common.tex @DX_DOCDIR@/pdf; cd @DX_DOCDIR@/pdf; \
	$(DX_LATEX) -interaction=batchmode @abs_srcdir@/docs/$@; \
	countdown=5; \
	while [ $$countdown -gt 0 ]; do \
		$(DX_LATEX) -interaction=batchmode @abs_srcdir@/docs/$@; \
		countdown=$$[$$countdown-1]; \
	done; 

endif DX_COND_ps

## ------------------------------ ##
## Rules specific for PDF output. ##
## ------------------------------ ##

if DX_COND_pdf

DX_CLEAN_PDF = docs/pdf

DX_PDF_GOAL = doxygen-pdf

doxygen-pdf: \
	BPG/PTF_Best_Practices_Guide.tex \
	PTF_Installation_Guide.tex \
	PTF_Periscope_Users_Guide.tex \
	PTF_Quick_Installation.tex \
	PTF_Tutorial.tex \
	plugins/PTF_CFS_Plugin.tex \
	plugins/PTF_DVFS_Plugin.tex \
	plugins/PTF_FixedAdaptiveSequence_Plugins.tex \
	plugins/PTF_MasterWorker_Plugin.tex \
	plugins/PTF_MPIParameters_Plugin.tex \
	plugins/PTF_Pipeline_Plugin.tex

%.tex: 
	mkdir -p @DX_DOCDIR@/pdf; cp docs/ptf_installation_common.tex @DX_DOCDIR@/pdf; cd @DX_DOCDIR@/pdf; \
	$(DX_PDFLATEX) -interaction=batchmode @abs_srcdir@/docs/$@; \
	countdown=5; \
	while [ $$countdown -gt 0 ]; do \
		$(DX_PDFLATEX) -interaction=batchmode @abs_srcdir@/docs/$@; \
		countdown=$$[$$countdown-1]; \
	done; 

endif DX_COND_pdf

## ------------------------------------------------- ##
## Rules specific for LaTeX (shared for PS and PDF). ##
## ------------------------------------------------- ##

if DX_COND_latex

DX_CLEAN_LATEX = docs/pdf

endif DX_COND_latex

.PHONY: doxygen-run doxygen-doc $(DX_PS_GOAL) $(DX_PDF_GOAL)

.INTERMEDIATE: doxygen-run $(DX_PS_GOAL) $(DX_PDF_GOAL)

doxygen-run: @DX_DOCDIR@/@PACKAGE@.tag

doxygen-doc: doxygen-run $(DX_PS_GOAL) $(DX_PDF_GOAL)

@DX_DOCDIR@/@PACKAGE@.tag: $(DX_CONFIG) $(pkginclude_HEADERS)
	rm -rf @DX_DOCDIR@/html
	rm -rf @DX_DOCDIR@/pdf
	$(DX_ENV) $(DX_DOXYGEN) $(srcdir)/$(DX_CONFIG)

DX_CLEANFILES = \
    -rf \
    $(DX_CLEAN_HTML) \
    $(DX_CLEAN_CHM) \
    $(DX_CLEAN_CHI) \
    $(DX_CLEAN_MAN) \
    $(DX_CLEAN_RTF) \
    $(DX_CLEAN_XML) \
    $(DX_CLEAN_PS) \
    $(DX_CLEAN_PDF) \
    $(DX_CLEAN_LATEX)

endif DX_COND_doc
