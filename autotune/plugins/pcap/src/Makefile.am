pcap_LTLIBRARIES       = libptfpcap.la
pcapdir = ${plugindir}/pcap/

libptfpcap_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                         -I$(top_srcdir)/autotune/plugins/pcap/include

libptfpcap_la_SOURCES  = autotune/plugins/pcap/src/PCAPPlugin.cc \
                         autotune/plugins/pcap/src/SpeedupAnalysis.cc
libptfpcap_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0
