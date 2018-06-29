exhaustive_atp_LTLIBRARIES = libptfexhaustive_atp.la
exhaustive_atpdir = ${searchdir}/exhaustive_atp/

libptfexhaustive_atp_la_CXXFLAGS = ${autotune_search_base_cxxflags} \
                                   -I$(top_srcdir)/autotune/searchalgorithms/exhaustive_atp/include \
                                   -I$(PSC_UTIL_INC)

libptfexhaustive_atp_la_SOURCES = autotune/searchalgorithms/exhaustive_atp/src/ExhaustiveATPSearch.cc \
                                  util/src/ATPService.cc
libptfexhaustive_atp_la_LDFLAGS = ${autotune_search_base_ldflags} -version-info 1:0:0
