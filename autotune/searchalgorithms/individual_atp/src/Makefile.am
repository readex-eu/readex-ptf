individual_atp_LTLIBRARIES = libptfindividual_atp.la
individual_atpdir = ${searchdir}/individual_atp/

libptfindividual_atp_la_CXXFLAGS = ${autotune_search_base_cxxflags} \
                               -I$(top_srcdir)/autotune/searchalgorithms/individual_atp/include \
                               -I$(top_srcdir)/autotune/searchalgorithms/exhaustive_atp/include \
                               -I$(PSC_UTIL_INC)

libptfindividual_atp_la_SOURCES = autotune/searchalgorithms/individual_atp/src/IndividualATPSearch.cc \
                                  util/src/ATPService.cc
libptfindividual_atp_la_LDFLAGS = ${autotune_search_base_ldflags} -version-info 1:0:0
