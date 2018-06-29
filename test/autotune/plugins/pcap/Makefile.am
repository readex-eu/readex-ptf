TESTS += test_pcap
check_PROGRAMS += test_pcap

test_pcap_CXXFLAGS = -I$(top_srcdir)/autotune/plugins/pcap/include  ${autotune_test_base_cxxflags}

test_pcap_SOURCES = test/autotune/plugins/pcap/PCAPPlugin.cc
test_pcap_LDADD = $(autotune_test_base_ldadd)
test_pcap_DEPENDENCIES = $(autotune_test_base_dependencies)
