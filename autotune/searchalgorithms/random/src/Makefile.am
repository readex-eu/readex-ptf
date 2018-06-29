random_LTLIBRARIES = libptfrandom.la
randomdir = ${searchdir}/random/

libptfrandom_la_CXXFLAGS = ${autotune_search_base_cxxflags} \
                           -I$(top_srcdir)/autotune/searchalgorithms/random/include

libptfrandom_la_SOURCES = autotune/searchalgorithms/random/src/RandomSearch.cc autotune/searchalgorithms/random/src/UniformDistributionModel.cc autotune/searchalgorithms/random/src/IndependentParametersModel.cc autotune/searchalgorithms/random/src/svm.cpp
libptfrandom_la_LDFLAGS = ${autotune_search_base_ldflags} -version-info 1:0:0  
