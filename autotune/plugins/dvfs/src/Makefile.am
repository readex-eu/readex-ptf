dvfs_LTLIBRARIES        = libptfdvfs.la
dvfsdir = ${plugindir}/dvfs/

libptfdvfs_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                         -I$(top_srcdir)/autotune/plugins/dvfs/include

libptfdvfs_la_SOURCES  = autotune/plugins/dvfs/src/DVFSPlugin.cc
libptfdvfs_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0 \
																 -release ${DVFS_VERSION_MAJOR}.${DVFS_VERSION_MINOR}.${DVFS_REVISION}  
