lib_LTLIBRARIES       += libptfstarter.la

libptfstarter_la_CXXFLAGS =${global_compiler_flags} \
                        -std=c++14 \
                        ${PSC_ACE_CPPFLAGS} \
                        ${PSC_BOOST_CPPFLAGS} \
                        -I$(top_srcdir)/quality_expressions/include \
                        -I$(top_srcdir)/frontend/include \
                        -I$(top_srcdir)/aagent/include \
                        -I$(top_srcdir)/registry/include \
                        -I$(top_srcdir)/util/include \
                        -I$(top_srcdir)/autotune/plugins/include \
                        -I$(top_srcdir)/autotune/datamodel/include \
                        -I$(top_srcdir)/autotune/searchalgorithms/include \
                        -I$(top_srcdir)/autotune/services/include \
                        -I$(top_srcdir)/

libptfstarter_la_SOURCES  = starterplugin/ptf-plugin.cc
libptfstarter_la_LDFLAGS  = -module -version-info 1:0:0
