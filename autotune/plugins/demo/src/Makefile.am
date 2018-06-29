demo_LTLIBRARIES = libptfdemo.la
demodir = ${plugindir}/demo/
libptfdemo_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                         -I$(top_srcdir)/autotune/plugins/demo/include

libptfdemo_la_SOURCES = autotune/plugins/demo/src/DemoPlugin.cc
libptfdemo_la_LDFLAGS = ${autotune_plugin_base_ldflags} -version-info 1:0:0
