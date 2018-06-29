masterworker_LTLIBRARIES       = libptfmasterworker.la
masterworkerdir = ${plugindir}/masterworker/

libptfmasterworker_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                 -I$(top_srcdir)/autotune/plugins/masterworker/include

libptfmasterworker_la_SOURCES  = autotune/plugins/masterworker/src/MasterWorkerPlugin.cc \
				 autotune/plugins/masterworker/src/conf_parser.ypp\
                                 autotune/plugins/masterworker/src/conf_scanner.lpp

libptfmasterworker_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0 \
																 -release ${MASTERWORKER_VERSION_MAJOR}.${MASTERWORKER_VERSION_MINOR}.${MASTERWORKER_REVISION}  

libptfmasterworker_la_scriptsdir = ${masterworkerdir}
libptfmasterworker_la_scripts_SCRIPTS = autotune/plugins/masterworker/src/param_spec.conf 
