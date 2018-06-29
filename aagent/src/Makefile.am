bin_PROGRAMS += psc_analysisagent

psc_analysisagent_CXXFLAGS = ${global_compiler_flags} \
                             -std=c++14 \
                             ${PSC_ACE_CPPFLAGS} \
                             ${PSC_BOOST_CPPFLAGS} \
                             ${PSC_OPENCL} \
                             ${PSC_CUPTI} \
                             ${PSC_ENOPT} \
                             ${PSC_VPATTERN} \
                             -D_REENTRANT \
                             -I$(top_srcdir)/quality_expressions/include \
                             -I$(top_srcdir)/aagent/include \
                             -I$(top_srcdir)/registry/include \
                             -I$(top_srcdir)/aagent/src/properties/include/overhead \
                             -I$(top_srcdir)/autotune/datamodel/include \
                             -I$(top_srcdir)/autotune/services/include \
                             -I$(top_srcdir)/util/include \
                             -I$(top_srcdir)/aagent/src/strategies/include

psc_analysisagent_CFLAGS = ${PSC_ACE_CPPFLAGS} \
                           ${PSC_BOOST_CPPFLAGS} \
                           ${PSC_OPENCL} \
                           ${PSC_CUPTI} \
                           ${PSC_ENOPT} \
                           ${PSC_VPATTERN} \
                           -D_REENTRANT \
                           -I$(top_srcdir)/quality_expressions/include \
                           -I$(top_srcdir)/aagent/include \
                           -I$(top_srcdir)/registry/include \
                           -I$(top_srcdir)/aagent/src/properties/include \
                           -I$(top_srcdir)/autotune/services/include \
                           -I$(top_srcdir)/util/include \
                           -I$(top_srcdir)/aagent/src/strategies/include

psc_analysisagent_LDADD = -L./ libpscreg.a \
                          libdatamodel.la \
                          libpscutil.a \
                          libstrategies.a \
                          libpscproperties.a \
                          ${PSC_ACE_LDFLAGS} \
                          ${PSC_BOOST_LDFLAGS} \
                          $(AM_LDFLAGS) 

psc_analysisagent_DEPENDENCIES = libpscreg.a \
                                 libdatamodel.la \
                                 libpscutil.a \
                                 libstrategies.a \
                                 libpscproperties.a

psc_analysisagent_SOURCES = aagent/src/Context.cc \
                            aagent/src/DataProvider.cc \
                            aagent/src/PerformanceDataBase.cc \
                            aagent/src/Property.cc \
                            aagent/src/accl_handler.cc \
                            aagent/src/accl_statemachine.cc \
                            aagent/src/accl_mrinodeagent_handler.cc \
                            aagent/src/analysisagent.cc \
                            aagent/src/application.cc \
                            aagent/src/asl_perfdata_cmm.cc \
                            aagent/src/experiment.cc \
                            aagent/src/main.cc \
                            aagent/src/msghandler.cc \
                            aagent/src/peer_acceptor.cc \
                            aagent/src/peer_connection.cc \
                            aagent/src/psc_agent.cc \
                            aagent/src/Region.cc \
                            aagent/src/strategy.cc \
                            aagent/src/StrategyRequest.cc \
                            aagent/src/rts.cc


if PSC_TDA_ENABLED
psc_analysisagent_CXXFLAGS+=${PSC_TDA_CPPFLAGS} -DHAVE_TDA=1
psc_analysisagent_LDADD+= ${PSC_TDA_LDFLAGS} 
endif

# TODO This one gets added on the blue genes
# mri_acceptor.cc 

