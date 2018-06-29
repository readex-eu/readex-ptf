bin_PROGRAMS += psc_frontend

ANALYSISAGENT = psc_analysisagent
HLAGENT = psc_hlagent

psc_frontend_CXXFLAGS = ${global_compiler_flags} \
                        -std=c++14 \
                        -rdynamic -Wl,-export-dynamic \
                        -D_AAGENT_EXEC="$(ANALYSISAGENT)" \
                        -D_HLAGENT_EXEC="$(HLAGENT)" \
                        ${PSC_ACE_CPPFLAGS} \
                        ${PSC_BOOST_CPPFLAGS} \
                        ${PSC_SQLITE3_CPPFLAGS} \
                        -D_REENTRANT \
                        -I$(top_srcdir)/quality_expressions/include \
                        -I$(top_srcdir)/frontend/include \
                        -I$(top_srcdir)/aagent/include \
                        -I$(top_srcdir)/registry/include \
                        -I$(top_srcdir)/util/include \
                        -I$(top_srcdir)/autotune/plugins/include \
                        -I$(top_srcdir)/autotune/datamodel/include \
                        -I$(top_srcdir)/autotune/searchalgorithms/include \
                        -I$(top_srcdir)/autotune/services/include \
                        -I$(top_srcdir)/

psc_frontend_LDFLAGS = -rdynamic

psc_frontend_LDADD = libpscreg.a \
                     libpscutil.a \
                     libstrategies.a \
                     libpscproperties.a \
                     libdatamodel.la \
                     libservices.la \
                     ${PSC_ACE_LDFLAGS} ${PSC_ACE_LIBS} \
                     ${PSC_BOOST_LDFLAGS} ${PSC_BOOST_LIBS} \
                     ${PSC_SQLITE3_LDFLAGS} ${PSC_SQLITE3_LIBS} \
                     $(AM_LDFLAGS)

psc_frontend_DEPENDENCIES = libpscreg.a \
                            libpscutil.a \
                            libstrategies.a \
                            libpscproperties.a \
                            libdatamodel.la \
                            libservices.la 

if PSC_CEREAL_ENABLED
psc_frontend_CXXFLAGS+= -I$(top_srcdir)/frontend/src/tuning_model/include -DHAVE_CEREAL=1
psc_frontend_LDADD+= libtuningmodel.a
psc_frontend_DEPENDENCIES +=libtuningmodel.a
endif

psc_frontend_SOURCES = frontend/src/frontend_accl_handler.cc \
                       frontend/src/frontend_accl_statemachine.cc \
                       frontend/src/frontend.cc \
                       frontend/src/frontend_statemachine.cc \
                       frontend/src/frontend_main.cc \
                       frontend/src/frontend_main_statemachine.cc \
                       aagent/src/psc_agent.cc \
                       aagent/src/peer_acceptor.cc \
                       aagent/src/peer_connection.cc \
                       aagent/src/msghandler.cc \
                       aagent/src/accl_handler.cc \
                       aagent/src/accl_statemachine.cc \
                       aagent/src/application.cc \
                       aagent/src/asl_perfdata_cmm.cc  \
                       aagent/src/Property.cc \
                       aagent/src/Region.cc \
                       aagent/src/Context.cc \
                       aagent/src/StrategyRequest.cc \
                       aagent/src/rts.cc \
                       frontend/src/ApplicationStarter.cc  \
    					   frontend/src/generate_tuning_model.cc
