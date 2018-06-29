romio_LTLIBRARIES             = libptfromio.la
romiodir = ${plugindir}/romio/

libptfromio_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                   -I$(top_srcdir)/autotune/plugins/romio/include

libptfromio_la_SOURCES  = autotune/plugins/romio/src/ROMIOPlugin.cc
libptfromio_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0
