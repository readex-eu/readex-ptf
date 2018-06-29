READEX_DTA_LIBCOMMON_INC = readex/cube_tools/datamodel/include

noinst_LTLIBRARIES = lib_dta_tool_common.la

lib_dta_tool_common_la_SOURCES = readex/cube_tools/common_src/helper.cc     \
                                 readex/cube_tools/common_src/Predicates.cc \
                                 readex/cube_tools/common_incl/helper.h     \
                                 readex/cube_tools/common_incl/Predicates.h \
                                 readex/cube_tools/common_incl/enums.h

lib_dta_tool_common_la_CXXFLAGS = ${global_compiler_flags}      \
                                  -I${READEX_DTA_LIBCOMMON_INC} \
                                  ${PSC_CUBE_CPPFLAGS}          \
                                  ${AM_CXXFLAGS}

lib_dta_tool_common_la_LDFLAGS = ${PSC_CUBE_LDFLAGS}
