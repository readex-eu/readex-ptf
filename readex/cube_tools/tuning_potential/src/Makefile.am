bin_PROGRAMS += readex-dyn-detect

readex_dyn_detect_INC      = readex/cube_tools/tuning_potential/include/

readex_dyn_detect_CXXFLAGS = ${global_compiler_flags}     \
                             -std=c++14                   \
                            -I$(readex_dyn_detect_INC)    \
                            -I$(READEX_DTA_LIBCOMMON_INC) \
                             ${PSC_CUBE_CPPFLAGS}         \
                             ${PSC_BOOST_CPPFLAGS}

readex_dyn_detect_LDADD   = lib_dta_tool_common.la \
                            lib_config_model.la



readex_dyn_detect_SOURCES = readex/cube_tools/tuning_potential/src/AdjacencyMatrix.cc \
                            readex/cube_tools/tuning_potential/src/DetectRegion.cc    \
                            readex/cube_tools/tuning_potential/src/TuningPotential.cc \
                            readex/cube_tools/tuning_potential/src/ProcessingIntensity.cc

readex_dyn_detect_LDFLAGS = ${PSC_CUBE_LDFLAGS}  \
                            ${PSC_BOOST_LDFLAGS}
