loadsearch_LTLIBRARIES = libptfload.la
loadsearchdir = ${searchdir}/load/

libptfload_la_CXXFLAGS = ${autotune_search_base_cxxflags} \
                         -I$(top_srcdir)/autotune/searchalgorithms/load/include

libptfload_la_SOURCES = autotune/searchalgorithms/load/src/LoadSearch.cc \
                        autotune/searchalgorithms/load/src/conf_parser.ypp \
                        autotune/searchalgorithms/load/src/conf_scanner.lpp
                              
libptfload_la_LDFLAGS = ${autotune_search_base_ldflags} -version-info 1:0:0  
