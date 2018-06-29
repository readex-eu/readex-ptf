lib_LTLIBRARIES += libscorep_substrate_tuning.la

libscorep_substrate_tuning_la_HEADERS = $(top_srcdir)/scorep/tuning_substrate_plugin/scorep/SCOREP_TuningPlugins.h        \
                                        $(top_srcdir)/scorep/tuning_substrate_plugin/scorep/SCOREP_TuningTypes.h          \
                                        $(top_srcdir)/scorep/tuning_substrate_plugin/scorep/SCOREP_UserTuning_Functions.h \
                                        $(top_srcdir)/scorep/tuning_substrate_plugin/scorep/SCOREP_UserTuning.h

libscorep_substrate_tuning_ladir = $(includedir)/scorep

libscorep_substrate_tuning_la_CFLAGS = -I$(top_srcdir)/scorep/tuning_substrate_plugin        \
                                       -I$(top_srcdir)/scorep/tuning_substrate_plugin/scorep \
                                       ${PSC_SCOREP_CPPFLAGS}

libscorep_substrate_tuning_la_SOURCES = scorep/tuning_substrate_plugin/SCOREP_Tuning.c                      \
                                        scorep/tuning_substrate_plugin/scorep_tuning_registered_functions.c \
                                        scorep/tuning_substrate_plugin/scorep_tuning_service_management.c   \
                                        scorep/tuning_substrate_plugin/scorep_tuning_stack.c                \
                                        scorep/tuning_substrate_plugin/scorep_tuning_table.c                \
                                        scorep/tuning_substrate_plugin/SCOREP_MappingTuningParameters.c     \
                                        scorep/tuning_substrate_plugin/UTILS_Debug.c                        \
                                        scorep/tuning_substrate_plugin/UTILS_Error.c                        \
                                        scorep/tuning_substrate_plugin/adapters/SCOREP_User_Tuning_Region.c


libscorep_substrate_tuning_la_LDFLAGS = -version-info 1:0:0
