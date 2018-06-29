mpicap_LTLIBRARIES       = libptfmpicap.la
mpicapdir = ${plugindir}/mpicap/

libptfmpicap_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                         -I$(top_srcdir)/autotune/plugins/mpicap/include

libptfmpicap_la_SOURCES  = autotune/plugins/mpicap/src/MPICAPPlugin.cc
libptfmpicap_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0
