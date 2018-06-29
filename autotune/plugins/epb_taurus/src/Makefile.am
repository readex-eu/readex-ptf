epb_taurus_LTLIBRARIES             = libptfepb_taurus.la
epb_taurusdir = ${plugindir}/epb_taurus/

libptfepb_taurus_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                   -I$(top_srcdir)/autotune/plugins/epb_taurus/include

libptfepb_taurus_la_SOURCES  = autotune/plugins/epb_taurus/src/epb_taurus.cc
libptfepb_taurus_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0