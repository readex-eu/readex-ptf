fixedsequence_LTLIBRARIES = libptffixedsequence.la

fixedsequencedir = ${plugindir}/fixedsequence/

libptffixedsequence_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                  -I$(top_srcdir)/autotune/plugins/fixedsequence/include

libptffixedsequence_la_SOURCES = autotune/plugins/fixedsequence/src/FixedSequencePlugin.cc \
                                 autotune/plugins/fixedsequence/src/conf_parser.ypp \
                                 autotune/plugins/fixedsequence/src/conf_scanner.lpp
                                 
libptffixedsequence_la_LDFLAGS = ${autotune_plugin_base_ldflags} -version-info 1:0:0 \
                                 -release ${FIXED_SEQUENCE_VERSION_MAJOR}.${FIXED_SEQUENCE_VERSION_MINOR}.${FIXED_SEQUENCE_REVISION}
