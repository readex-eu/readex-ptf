compilerflags_LTLIBRARIES = libptfcompilerflags.la
compilerflagsdir = ${plugindir}/compilerflags/

libptfcompilerflags_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                  -I$(top_srcdir)/autotune/plugins/compilerflags/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/individual/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/random/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/gde3/include

libptfcompilerflags_la_SOURCES = autotune/plugins/compilerflags/src/CompilerFlagsPlugin.cc \
                                 autotune/plugins/compilerflags/src/CFSTuningParameter.cc \
                                 autotune/plugins/compilerflags/src/conf_parser.ypp \
                                 autotune/plugins/compilerflags/src/conf_scanner.lpp

templates_DATA += autotune/plugins/compilerflags/src/cfs_ifort.cfg
templates_DATA += autotune/plugins/compilerflags/src/cfs_icc.cfg
templates_DATA += autotune/plugins/compilerflags/src/cfs_config.cfg.default

libptfcompilerflags_la_LDFLAGS = ${autotune_plugin_base_ldflags} -lboost_program_options -version-info 1:0:0 \
																 -release ${COMPILERFLAGS_VERSION_MAJOR}.${COMPILERFLAGS_VERSION_MINOR}.${COMPILERFLAGS_REVISION}  

libptfcompilerflags_la_scriptsdir = ${compilerflagsdir}
libptfcompilerflags_la_scripts_SCRIPTS = autotune/plugins/compilerflags/src/touchall_cfs.sh \
                                         autotune/plugins/compilerflags/src/cfs_extract_files.sh \
                                         autotune/plugins/compilerflags/src/cfs_selected_files.sh \
                                         autotune/plugins/compilerflags/src/cfs_touchselected.sh \
                                         autotune/plugins/compilerflags/src/cfs_sha1.sh
