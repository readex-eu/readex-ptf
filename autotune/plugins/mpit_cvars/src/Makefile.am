mpit_cvars_LTLIBRARIES             = libptfmpit_cvars.la
mpit_cvarsdir = ${plugindir}/mpit_cvars/

libptfmpit_cvars_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                   -I$(top_srcdir)/autotune/plugins/mpit_cvars/include

libptfmpit_cvars_la_SOURCES  = autotune/plugins/mpit_cvars/src/MpitCvarsPlugin.cc
libptfmpit_cvars_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0 
