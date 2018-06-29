debug_LTLIBRARIES             = libptfdebug.la
debugdir = ${plugindir}/debug/

libptfdebug_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                   -I$(top_srcdir)/autotune/plugins/debug/include

libptfdebug_la_SOURCES  = autotune/plugins/debug/src/DebugPlugin.cc
libptfdebug_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0
