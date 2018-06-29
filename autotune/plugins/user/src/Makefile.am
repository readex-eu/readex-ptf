user_LTLIBRARIES        = libptfuser.la
userdir = ${plugindir}/user/

libptfuser_la_CXXFLAGS  = ${autotune_plugin_base_cxxflags}\
												 -I$(top_srcdir)/autotune/plugins/user/include 

libptfuser_la_SOURCES  = autotune/plugins/user/src/UserPlugin.cc 
libptfuser_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0 
