bin_PROGRAMS += scorep-autofilter

scorep_autofilter_INC = readex/cube_tools/filter_regions/include/

scorep_autofilter_CXXFLAGS = ${global_compiler_flags}      \
                             -std=c++14                    \
                             -I$(FilterRegion_INC)         \
                             -I$(READEX_DTA_LIBCOMMON_INC) \
                              ${PSC_CUBE_CPPFLAGS}         \
                              ${PSC_BOOST_CPPFLAGS}

scorep_autofilter_LDADD = lib_dta_tool_common.la 

scorep_autofilter_SOURCES = readex/cube_tools/filter_regions/src/FilterRegion.cc 

scorep_autofilter_LDFLAGS = ${PSC_CUBE_LDFLAGS}  \
                            ${PSC_BOOST_LDFLAGS}
