mpit_custom_LTLIBRARIES       = libptfmpit-custom.la
mpit_customdir = ${plugindir}/mpit-custom/

libptfmpit_custom_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                -I$(top_srcdir)/autotune/plugins/mpit-custom/include

libptfmpit_custom_la_SOURCES  = autotune/plugins/mpit-custom/src/MPITCPlugin.cc
libptfmpit_custom_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0
