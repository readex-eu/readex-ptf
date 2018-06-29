skeleton_LTLIBRARIES             = libptfskeleton.la
skeletondir = ${plugindir}/skeleton/

libptfskeleton_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                   -I$(top_srcdir)/autotune/plugins/skeleton/include

libptfskeleton_la_SOURCES  = autotune/plugins/skeleton/src/SkeletonPlugin.cc
libptfskeleton_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0
