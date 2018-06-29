ufs_taurus_LTLIBRARIES             = libptfufs_taurus.la
ufs_taurusdir = ${plugindir}/ufs_taurus/

libptfufs_taurus_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                   -I$(top_srcdir)/autotune/plugins/ufs_taurus/include

libptfufs_taurus_la_SOURCES  = autotune/plugins/ufs_taurus/src/ufs_taurus.cc
libptfufs_taurus_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0