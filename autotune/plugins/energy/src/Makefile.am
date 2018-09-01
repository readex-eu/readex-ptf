energy_LTLIBRARIES       = libptfenergy.la

energydir = ${plugindir}/energy/

libptfenergy_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                         -I$(top_srcdir)/autotune/plugins/energy/include

libptfenergy_la_SOURCES  = autotune/plugins/energy/src/EnergyPlugin.cc

libptfenergy_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0 \
                           -release ${ENERGY_VERSION_MAJOR}.${ENERGY_VERSION_MINOR}.${ENERGY_REVISION}
