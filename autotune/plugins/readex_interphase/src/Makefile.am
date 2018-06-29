readex_interphase_LTLIBRARIES             = libptfreadex_interphase.la
readex_interphasedir = ${plugindir}/readex_interphase/

libptfreadex_interphase_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                  -I$(top_srcdir)/autotune/searchalgorithms/random/include \
                                   -I$(top_srcdir)/autotune/plugins/readex_interphase/include \
                                   -std=c++14
#templates_DATA += autotune/plugins/readex_tuning/src/readex_config.xml.default

libptfreadex_interphase_la_SOURCES  = autotune/plugins/readex_interphase/src/readex_interphase.cc
libptfreadex_interphase_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0
