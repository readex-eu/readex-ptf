readex_intraphase_LTLIBRARIES             = libptfreadex_intraphase.la

readex_intraphasedir = ${plugindir}/readex_intraphase/

libptfreadex_intraphase_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                  -I$(top_srcdir)/autotune/plugins/compilerflags/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/individual/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/exhaustive_atp/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/individual_atp/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/random/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/gde3/include \
                                  -I$(PSC_UTIL_INC)                                      \
                                  -I$(top_srcdir)/autotune/plugins/readex_intraphase/include

templates_DATA += autotune/plugins/readex_intraphase/src/readex_config.xml.default

libptfreadex_intraphase_la_SOURCES  = autotune/plugins/readex_intraphase/src/readex_intraphase.cc \
                                  util/src/ATPService.cc
libptfreadex_intraphase_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0
