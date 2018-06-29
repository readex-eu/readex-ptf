pcap_speedup_LTLIBRARIES       = libptfpcap-speedup.la
pcap_speedupdir = ${plugindir}/pcap-speedup/

libptfpcap_speedup_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                         -I$(top_srcdir)/autotune/plugins/pcap-speedup/include

libptfpcap_speedup_la_SOURCES  = autotune/plugins/pcap-speedup/src/PCAPSpeedupPlugin.cc
libptfpcap_speedup_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0
