lib_LIBRARIES += libtuningmodel.a

libtuningmodel_a_CXXFLAGS = \
	${global_compiler_flags} \
	-DTUNING_MODEL_DEBUG \
	-std=c++14 \
	${PSC_BOOST_CPPFLAGS} \
	${PSC_CEREAL_CPPFLAGS} \
	-I$(top_srcdir)/frontend/src/tuning_model/include \
	-I$(top_srcdir)/aagent/include \
	-I$(top_srcdir)/util/include

libtuningmodel_a_SOURCES = \
	frontend/src/tuning_model/src/clustering/clusterer.cc\
	frontend/src/tuning_model/src/clustering/dendrogram.cc \
	frontend/src/tuning_model/src/common.cc \
	frontend/src/tuning_model/src/conversion.cc \
	frontend/src/tuning_model/src/merge.cc \
	frontend/src/tuning_model/src/phase.cc \
	frontend/src/tuning_model/src/serialization.cc \
	frontend/src/tuning_model/src/tuning_model.cc

bin_PROGRAMS += tmmerger

tmmerger_CXXFLAGS = \
	${global_compiler_flags} \
	-DTUNING_MODEL_DEBUG \
	-std=c++14 \
	-I$(top_srcdir)/frontend/src/tuning_model/include \
	${PSC_BOOST_CPPFLAGS}

tmmerger_LDFLAGS = -rdynamic

tmmerger_LDADD = libtuningmodel.a

tmmerger_SOURCES = \
	frontend/src/tuning_model/src/tmmerger.cc
