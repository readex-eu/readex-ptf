readex_interphase_LTLIBRARIES             = libptfreadex_interphase.la

readex_interphasedir = ${plugindir}/readex_interphase/

libptfreadex_interphase_la_CXXFLAGS = ${autotune_plugin_base_cxxflags}                           \
                                      -I$(top_srcdir)/autotune/searchalgorithms/random/include   \
                                      -I$(top_srcdir)/autotune/plugins/readex_interphase/include

libptfreadex_interphase_la_SOURCES  = autotune/plugins/readex_interphase/src/readex_interphase.cc

libptfreadex_interphase_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0 \
                                      -release ${READEX_INTERPHASE_VERSION_MAJOR}.${READEX_INTERPHASE_VERSION_MINOR}.${READEX_INTERPHASE_REVISION}
