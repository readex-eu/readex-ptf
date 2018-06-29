libservices_la_CXXFLAGS = ${global_compiler_flags} \
                          -std=c++14 \
                          ${PSC_ACE_CPPFLAGS} \
                          ${PSC_BOOST_CPPFLAGS} \
                          ${PSC_SQLITE3_CPPFLAGS} \
                          -I$(top_srcdir)/aagent/include \
                          -I$(top_srcdir)/autotune/datamodel/include \
                          -I$(top_srcdir)/autotune/plugins/include \
                          -I$(top_srcdir)/autotune/searchalgorithms/include \
                          -I$(top_srcdir)/autotune/services/include \
                          -I$(top_srcdir)/frontend/include \
                          -I$(top_srcdir)/util/include \
                          -I$(top_srcdir)/quality_expressions/include \
                          -I$(top_srcdir)/registry/include

lib_LTLIBRARIES += libservices.la

libservices_la_SOURCES = autotune/services/src/autotune_services.cc \
                         autotune/services/src/search_common.cc \
                         autotune/services/src/DriverContext.cc \
                         autotune/services/src/TuningDatabase.cc \
                         autotune/services/src/DummyTuningDatabase.cc \
                         autotune/services/src/JsonTuningDatabase.cc \
                         autotune/services/src/Sqlite3TuningDatabase.cc


if PSC_SQLITE3_ENABLED
libservices_la_LDFLAGS = ${PSC_SQLITE3_LDFLAGS} -version-info 1:0:0
endif
