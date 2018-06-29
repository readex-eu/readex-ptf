mpiparameters_LTLIBRARIES = libptfmpiparameters.la
mpiparametersdir = ${plugindir}/mpiparameters/

templates_DATA += autotune/plugins/mpiparameters/src/MPIparam_ibm.conf
templates_DATA += autotune/plugins/mpiparameters/src/MPIparam_intel.conf
templates_DATA += autotune/plugins/mpiparameters/src/MPIparam_openmpi.conf

libptfmpiparameters_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                  -I$(top_srcdir)/autotune/plugins/mpiparameters/include\
                                  -I$(top_srcdir)/autotune/searchalgorithms/individual/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/random/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/gde3/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/load/include

libptfmpiparameters_la_SOURCES  = autotune/plugins/mpiparameters/src/MPIParametersPlugin.cc \
                                  autotune/plugins/mpiparameters/src/conf_parser.ypp \
                                  autotune/plugins/mpiparameters/src/conf_scanner.lpp

libptfmpiparameters_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0 \
																 -release ${MPIPARAMETERS_VERSION_MAJOR}.${MPIPARAMETERS_VERSION_MINOR}.${MPIPARAMETERS_REVISION}  

libptfmpiparameters_la_scriptsdir = ${mpiparametersdir}
libptfmpiparameters_la_scripts_SCRIPTS = autotune/plugins/mpiparameters/src/param_spec.conf 
