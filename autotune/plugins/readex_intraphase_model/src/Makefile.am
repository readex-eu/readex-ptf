readex_intraphase_model_LTLIBRARIES             = libptfreadex_intraphase_model.la

readex_intraphase_modeldir = ${plugindir}/readex_intraphase_model/

libptfreadex_intraphase_model_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                  -I$(top_srcdir)/autotune/plugins/compilerflags/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/individual/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/exhaustive_atp/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/individual_atp/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/random/include \
                                  -I$(top_srcdir)/autotune/searchalgorithms/gde3/include \
                                  -I$(PSC_UTIL_INC)                                      \
                                  -I$(top_srcdir)/autotune/plugins/readex_intraphase_model/include



libptfreadex_intraphase_model_la_SOURCES  = autotune/plugins/readex_intraphase_model/src/readex_intraphase_model.cc \
                                            util/src/ATPService.cc

libptfreadex_intraphase_model_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0 \
                                            -release ${READEX_INTRAPHASE_MODEL_VERSION_MAJOR}.${READEX_INTRAPHASE_MODEL_VERSION_MINOR}.${READEX_INTRAPHASE_MODEL_REVISION}
