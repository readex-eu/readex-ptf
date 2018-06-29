lib_LIBRARIES += libpscutil.a

PSC_UTIL_INC          = $(top_srcdir)/util/include

libpscutil_a_CXXFLAGS = ${global_compiler_flags}                   \
                        -std=c++14                                 \
                        -rdynamic -Wl,-export-dynamic              \
                        ${PSC_ACE_CPPFLAGS}                        \
                        ${PSC_BOOST_CPPFLAGS}                      \
                        -DDEBUG=1                                  \
                        -I$(top_srcdir)/aagent/include             \
                        -I$(top_srcdir)/registry/include           \
                        -I$(top_srcdir)/autotune/datamodel/include \
                        -I$(PSC_UTIL_INC)

libpscutil_a_CFLAGS = ${global_compiler_flags}         \
                      ${PSC_ACE_CPPFLAGS}              \
                      ${PSC_BOOST_CPPFLAGS}            \
                      -DDEBUG=1                        \
                      -I$(top_srcdir)/aagent/include   \
                      -I$(top_srcdir)/registry/include \
                      -I$(top_srcdir)/autotune/datamodel/include \
                      -I$(PSC_UTIL_INC)

libpscutil_a_SOURCES = util/src/MetaProperty.cc        \
                       util/src/state_machine_trace.cc \
                       util/src/cfgfile.c              \
                       util/src/getopt.c               \
                       util/src/getopt1.c              \
                       util/src/psc_config.c           \
                       util/src/psc_errmsg.c           \
                       util/src/selective_debug.cc     \
                       util/src/stringutil.cc          \
                       util/src/strutil.c              \
                       util/src/timing.c               \
                       util/src/Metric.c               \
                       util/src/string_helper.cc       \
                       util/src/ATPService.cc          

libpscutil_a_LIBADD = libpscreg.a
