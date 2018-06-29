adaptivesequence_LTLIBRARIES = libptfadaptivesequence.la
adaptivesequencedir = ${plugindir}/adaptivesequence/

libptfadaptivesequence_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                            -I$(top_srcdir)/autotune/plugins/adaptivesequence/include

libptfadaptivesequence_la_SOURCES = autotune/plugins/adaptivesequence/src/AdaptiveSequencePlugin.cc \
                                    autotune/plugins/adaptivesequence/src/conf_parser.ypp \
                                    autotune/plugins/adaptivesequence/src/conf_scanner.lpp

libptfadaptivesequence_la_LDFLAGS = ${autotune_plugin_base_ldflags} -version-info 1:0:0

templates_DATA += autotune/plugins/adaptivesequence/src/as_config.cfg.default
