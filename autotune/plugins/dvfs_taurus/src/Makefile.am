dvfs_taurus_LTLIBRARIES             = libptfdvfs_taurus.la
dvfs_taurusdir = ${plugindir}/dvfs_taurus/

libptfdvfs_taurus_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                   -I$(top_srcdir)/autotune/plugins/dvfs_taurus/include

libptfdvfs_taurus_la_SOURCES  = autotune/plugins/dvfs_taurus/src/dvfs_taurus.cc
libptfdvfs_taurus_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0