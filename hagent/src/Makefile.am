bin_PROGRAMS += psc_hlagent

psc_hlagent_CXXFLAGS = ${global_compiler_flags}                   \
                       -std=c++14                                \
                       ${PSC_ACE_CPPFLAGS}                        \
                       ${PSC_BOOST_CPPFLAGS}                      \
                       -D_REENTRANT                               \
                       -I../include                               \
                       -I$(top_srcdir)/aagent/include             \
                       -I$(top_srcdir)/hagent/include             \
                       -I$(top_srcdir)/autotune/datamodel/include \
                       -I$(top_srcdir)/registry/include           \
                       -I$(top_srcdir)/util/include               \
                       -I$(top_srcdir)/strategies/include

psc_hlagent_LDADD = libpscreg.a                            \
                    libpscutil.a                           \
                    ${PSC_ACE_LDFLAGS} ${PSC_ACE_LIBS}     \
                    ${PSC_BOOST_LDFLAGS} ${PSC_BOOST_LIBS} \
                    $(AM_LDFLAGS)

psc_hlagent_DEPENDENCIES = libpscreg.a \
                           libpscutil.a

psc_hlagent_SOURCES = hagent/src/hlagent.cc                  \
                      hagent/src/hlagent_main.cc             \
                      hagent/src/hagent_accl_handler.cc      \
                      hagent/src/hagent_accl_statemachine.cc \
                      aagent/src/psc_agent.cc                \
                      aagent/src/peer_acceptor.cc            \
                      aagent/src/peer_connection.cc          \
                      aagent/src/msghandler.cc               \
                      aagent/src/accl_handler.cc             \
                      aagent/src/accl_statemachine.cc
