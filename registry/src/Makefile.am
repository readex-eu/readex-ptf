lib_LIBRARIES += libpscreg.a
bin_PROGRAMS += psc_regsrv

libpscreg_a_CXXFLAGS = ${global_compiler_flags} \
                       -std=c++14 \
                       ${PSC_ACE_CPPFLAGS} \
                       -I$(top_srcdir)/aagent/include \
                       -I$(top_srcdir)/util/include \
                       -I$(top_srcdir)/registry/include

libpscreg_a_CFLAGS = ${global_compiler_flags} \
                     ${PSC_ACE_CPPFLAGS} \
                     -I$(top_srcdir)/aagent/include \
                     -I$(top_srcdir)/util/include \
                     -I$(top_srcdir)/registry/include

psc_regsrv_CXXFLAGS = ${global_compiler_flags} \
                      -std=c++14 \
                      ${PSC_ACE_CPPFLAGS} \
                      -I$(top_srcdir)/aagent/include \
                      -I$(top_srcdir)/util/include \
                      -I$(top_srcdir)/registry/include

psc_regsrv_CFLAGS = ${global_compiler_flags} \
                    ${PSC_ACE_CPPFLAGS} \
                    -I$(top_srcdir)/aagent/include \
                    -I$(top_srcdir)/util/include \
                    -I$(top_srcdir)/registry/include

psc_regsrv_LDADD = libpscutil.a \
                   ${PSC_ACE_LDFLAGS} \
                   ${PSC_ACE_LIBS}

psc_regsrv_DEPENDENCIES = libpscutil.a

libpscreg_a_SOURCES = registry/src/registry.c \
                      registry/src/regxx.cc \
                      registry/src/sockets.c

psc_regsrv_SOURCES = registry/src/regsrv_client.cc \
                     registry/src/regsrv.cc \
                     registry/src/regsrv_main.cc
