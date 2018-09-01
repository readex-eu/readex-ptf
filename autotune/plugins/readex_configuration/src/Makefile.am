readex_configuration_LTLIBRARIES = libptfreadex_configuration.la

readex_configurationdir = ${plugindir}/readex_configuration/

libptfreadex_configuration_la_CXXFLAGS = ${autotune_plugin_base_cxxflags}                              \
                                         -I$(top_srcdir)/autotune/plugins/readex_configuration/include \
                                         -I$(top_srcdir)/autotune/searchalgorithms/individual/include  \
                                         -I$(top_srcdir)/autotune/searchalgorithms/random/include      \
                                         -I$(top_srcdir)/autotune/searchalgorithms/gde3/include

libptfreadex_configuration_la_SOURCES  = autotune/plugins/readex_configuration/src/readex_configuration.cc \
                                         autotune/plugins/readex_configuration/src/appConfigParameter.cc   \
                                         autotune/plugins/readex_configuration/src/conf_parser.ypp         \
                                         autotune/plugins/readex_configuration/src/conf_scanner.lpp


templates_DATA += autotune/plugins/readex_configuration/src/appConfigParams.cfg

libptfreadex_configuration_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0 \
                                         -release ${READEX_CONFIGURATION_VERSION_MAJOR}.${READEX_CONFIGURATION_VERSION_MINOR}.${READEX_CONFIGURATION_REVISION}

libptfreadex_configuration_la_scriptsdir = ${readex_configurationdir}

libptfreadex_configuration_la_scripts_SCRIPTS = autotune/plugins/readex_configuration/src/set_configuration_parameter.sh

