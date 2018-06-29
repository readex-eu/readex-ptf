lib_LTLIBRARIES += libtest.la

libtest_la_CXXFLAGS = ${autotune_test_base_cxxflags}

libtest_la_SOURCES = frontend/src/frontend.cc \
                     frontend/src/frontend_statemachine.cc \
                     frontend/src/frontend_accl_handler.cc \
                     frontend/src/frontend_accl_statemachine.cc \
                     frontend/src/ApplicationStarter.cc \
                     aagent/src/psc_agent.cc \
                     aagent/src/peer_acceptor.cc \
                     aagent/src/peer_connection.cc \
                     aagent/src/msghandler.cc \
                     aagent/src/accl_handler.cc \
                     aagent/src/accl_statemachine.cc \
                     aagent/src/application.cc \
                     aagent/src/asl_perfdata_cmm.cc \
                     aagent/src/Property.cc \
                     aagent/src/Region.cc \
                     aagent/src/Context.cc \
                     aagent/src/StrategyRequest.cc

libtest_la_LDFLAGS = -version-info 1:0:0

libtest_la_LIBADD = libdatamodel.la \
                    libservices.la \
                    ${PSC_ACE_LDFLAGS} \
                    ${PSC_ACE_LIBS} \
                    ${PSC_BOOST_LDFLAGS} \
                    ${PSC_BOOST_LIBS} \
                    $(AM_LDFLAGS)

libtest_la_DEPENDENCIES = libdatamodel.la \
                          libservices.la
