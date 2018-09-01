AUTOMAKE_OPTIONS = subdir-objects -Wno-portability

ACLOCAL_AMFLAGS= -I m4

AM_YFLAGS = -d
AM_LFLAGS = --header-file=conf_scanner.hpp

scriptsdir = ${bindir}
scripts_SCRIPTS = scripts/psc_clean.sh         \
                  scripts/psc_showprops        \
                  scripts/psc_wrapper.sh       \
                  scripts/reinstrumentedall.sh \
                  scripts/reinstrumented.sh    \
                  scripts/touchall.sh          \
                  scripts/psc_result           \
                  scripts/psc_properties.py

if DEVELOPER_MODE
scripts_SCRIPTS += scripts/psc_generate_bare_plugin_from_skeleton \
                   scripts/psc_source_code_directory
endif

templatesdir = $(prefix)/templates
templates_DATA = templates/periscope.sample              \
                 templates/periscope.sample.bluegene_FZJ \
                 templates/psc_bashrc                    \
                 templates/psc_BGP_LLscript              \
                 templates/psc_cshrc

AM_CXXFLAGS += -fexceptions
AM_CFLAGS += -fexceptions
AM_LDFLAGS += -lpthread


include util/Makefile.am
include registry/Makefile.am
include autotune/Makefile.am
include aagent/Makefile.am
include frontend/Makefile.am
include hagent/Makefile.am
include test/Makefile.am
include docs/Makefile.am


include readex/Makefile.am


include starterplugin/Makefile.am
if PSC_SCOREP_ENABLED
include scorep/tuning_substrate_plugin/Makefile.am
endif

EXTRA_DIST = aagent/include                                  \
             aagent/src/properties/include                   \
             aagent/src/strategies/include                   \
             autotune/datamodel/include                      \
             autotune/plugins/include                        \
             autotune/plugins/demo/include                   \
             autotune/plugins/compilerflags/include          \
             autotune/plugins/dvfs/include                   \
             autotune/plugins/user/include                   \
             autotune/plugins/masterworker/include           \
             autotune/plugins/mpit-custom/include            \
             autotune/plugins/mpiparameters/include          \
             autotune/plugins/pipeline/include               \
             autotune/plugins/skeleton/include               \
             autotune/searchalgorithms/include               \
             autotune/searchalgorithms/exhaustive/include    \
             autotune/searchalgorithms/compilerflags/include \
             autotune/searchalgorithms/activeharmony/include \
             autotune/services/include                       \
             frontend/include                                \
             hagent/include                                  \
             registry/include                                \
             util/include                                    \
             ${scripts_SCRIPTS}                              \
             ${templates_SCRIPTS}

SUBDIRS = 

MOSTLYCLEANFILES = $(DX_CLEANFILES)



install-pdf: 
	if [ -d ./docs/pdf ]; then                                                                                                                       \
		mkdir -p $(prefix)/docs/pdf;                                                                                                             \
		$(INSTALL) -m 644 --target-directory=$(prefix)/docs/pdf ./docs/pdf/*.pdf;                                                                \
	else                                                                                                                                             \
		echo "PDF folder does not exist. Make sure build was configured with --doxygen-pdf or --doxygen-doc and that 'make doc' was performed."; \
	fi

install-html: 
	if [ -d ./docs/html ]; then                                                                                                                       \
		mkdir -p $(prefix)/docs/html;                                                                                                             \
		$(INSTALL) -m 644 --target-directory=$(prefix)/docs/html ./docs/html/*;                                                                   \
	else                                                                                                                                              \
		echo "HTML folder does not exist. Make sure build was configured with --doxygen-html or --doxygen-doc and that 'make doc' was performed"; \
	fi

# convenience targets based on the doxygen targets; configure with --doxgigen-doc,-html and -pdf may be required
install-doc: install-pdf install-html

doc-install: install-doc

html-install: install-html

pdf-install: install-pdf

doc: doxygen-doc

html: doxygen-html

pdf: doxygen-pdf

