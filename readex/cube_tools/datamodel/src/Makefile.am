lib_LTLIBRARIES += lib_config_model.la

lib_config_model_la_SOURCES  = readex/cube_tools/datamodel/src/ConfigModel.cc

lib_config_model_la_CXXFLAGS = ${global_compiler_flags}                 \
                               -I${readex/cube_tools/datamodel/include} \
                               ${PSC_CUBE_CPPFLAGS}                     \
                               ${PSC_BOOST_CPPFLAGS}                    \
                               ${AM_CXXFLAGS}

lib_config_model_la_LDFLAGS  = ${PSC_CUBE_LDFLAGS}  \
                               ${PSC_BOOST_LDFLAGS}
