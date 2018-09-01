#include "../include/ConfigModel.h"

#include <boost/version.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
using boost::property_tree::ptree;

ConfigModel::ConfigModel()
{
}

ConfigModel::ConfigModel( bool                              isExist,
                          const list< SignificantRegion* >& significant_regions,
                          vector< string >&                 sig_region_names,
                          const PhaseRegion*                phase,
                          double                            weight,
                          double                            time_variation,
                          double                            compute_intensity_variation,
                          double                            granularity_sig_region,
                          vector< string >&                 t_variation_reg,
                          vector< string >&                 com_intensity_variation_reg )
{
    ptree significant_region;
    ptree intra_phase;
    if ( significant_regions.empty() )
    {
        for ( const auto& region : sig_region_names )
        {
            significant_region.put( "name", region );
            significant_region.put( "granularity", 1 );
            significant_region.put( "weight", 1 );
            significant_region.put( "time_variation_reg", 1 );
            significant_region.put( "time_variation_phase", 1 );
            significant_region.put( "compute_intensity", 1);
            intra_phase.push_back( make_pair( "significantRegion", significant_region ) );
        }
    } else {
        for ( const auto& region : significant_regions )
        {
            significant_region.put( "name", region->name );
            significant_region.put( "granularity", region->granularity );
            significant_region.put( "weight", region->dynamism_metrics[ 0 ].dyn_perc_phase );
            significant_region.put( "time_variation_reg", region->dynamism_metrics[ 0 ].dev_perc_reg );
            significant_region.put( "time_variation_phase", region->dynamism_metrics[ 0 ].dev_perc_phase );
            significant_region.put( "compute_intensity", region->dynamism_metrics[ 1 ].dyn_perc_phase );
            intra_phase.push_back( make_pair( "significantRegion", significant_region ) );
        }
    }

    ptree tuning_potential;
    tuning_potential.add_child( "Intra-phase", intra_phase );
    if( phase != NULL )
    {
        ptree inter_phase;
        inter_phase.put( "name", phase->name );
        inter_phase.put( "variation", phase->dev_perc );
        tuning_potential.add_child( "Inter-phase", inter_phase  );
    }

    ptree thresholds;
    thresholds.put( "weight", weight );
    thresholds.put( "time_variation", time_variation );
    thresholds.put( "compute_intensity_variation", compute_intensity_variation );
    thresholds.put( "granularity_significant_region", granularity_sig_region );
    tuning_potential.add_child( "Thresholds", thresholds );

    ptree dyn_t_variation;
    for ( const auto& t_reg : t_variation_reg )
    {
        dyn_t_variation.add( "name", t_reg );
    }

    ptree dynamism;
    dynamism.add_child( "dyn_time_variation", dyn_t_variation );

    ptree dyn_comIn_variation;
    for ( const auto& com_reg : com_intensity_variation_reg )
    {
        dyn_comIn_variation.add( "name", com_reg );
    }

    dynamism.add_child( "dyn_comIn_variation", dyn_comIn_variation );
    tuning_potential.add_child( "dynamism", dynamism );

    if( isExist )
        internalData = tuning_potential;
    else
        internalData.add_child( "Configuration.readex-dyn-detect", tuning_potential );
}

void
ConfigModel::updateTree( ptree& configTree )
{
    //XMLConfigTree = configTree.get_child( "Configuration.readex-dyn-detect" );
    try
    {
        configTree.put_child( "Configuration.readex-dyn-detect", internalData );
        XMLConfigTree = configTree;
    } catch ( exception &e )
    {
        remove( "readex_config.xml" );
        createTemplate();
    }
}

void
ConfigModel::createTemplate()
{
    ptree tuning_parameter;
    XMLConfigTree = internalData;

    //Add cpu freq
    ptree cpu_freq;
    cpu_freq.put( "min_freq",  string() );
    cpu_freq.put( "max_freq",  string() );
    cpu_freq.put( "freq_step", string() );
    cpu_freq.put( "default", string() );
    tuning_parameter.add_child( "frequency", cpu_freq );

    //Add uncore freq
    ptree uncore_freq;
    uncore_freq.put( "min_freq",  string() );
    uncore_freq.put( "max_freq",  string() );
    uncore_freq.put( "freq_step", string() );
    uncore_freq.put( "default", string() );
    tuning_parameter.add_child( "uncore", uncore_freq );

    //Add OpenMPThread
    ptree openMPThreads;
    openMPThreads.put( "lower_value", string() );
    openMPThreads.put( "step", string() );
    tuning_parameter.add_child( "openMPThreads", openMPThreads );
    XMLConfigTree.add_child( "Configuration.tuningParameter", tuning_parameter );

    //Add Objectives
    ptree objectives;
    objectives.add( "objective", "Energy" );
    //objectives.add( "objective", "NormalizedEnergy" );
    objectives.add( "objective", "Time" );
    //objectives.add( "objective", "NormalizedTime" );
    objectives.add( "objective", "EDP" );
    //objectives.add( "objective", "NormalizedEDP" );
    objectives.add( "objective", "ED2P" );
    //objectives.add( "objective", "NormalizedED2P" );
    objectives.add( "objective", "CPUEnergy" );
    //objectives.add( "objective", "NormalizedCPUEnergy" );
    objectives.add( "objective", "TCO" );
    //objectives.add( "objective", "NormalizedTCO" );
    XMLConfigTree.add_child( "Configuration.objectives", objectives );

    //TCO
    ptree CostPerJoule;
    CostPerJoule.put( "", 0.00000008 );
    XMLConfigTree.add_child( "Configuration.CostPerJoule", CostPerJoule );

    ptree CostPerCoreHour;
    CostPerCoreHour.put( "", 1.0 );
    XMLConfigTree.add_child( "Configuration.CostPerCoreHour", CostPerCoreHour );

    //Add Metric Plugin
    ptree metric_plugin;
    metric_plugin.put( "name", "hdeem_sync_plugin" );
    metric_plugin.put( "name", "x86_energy_sync_plugin" );
    XMLConfigTree.add_child( "Configuration.periscope.metricPlugin", metric_plugin );

    //Add Metrics
    ptree metric_energy;
    metric_energy.put( "node_energy", "hdeem/BLADE/E" );
    metric_energy.put( "cpu0_energy", "hdeem/CPU0/E" );
    metric_energy.put( "cpu1_energy", "hdeem/CPU1/E" );
    metric_energy.put( "node_energy", "x86_energy/BLADE/E" );
    XMLConfigTree.add_child( "Configuration.periscope.metrics", metric_energy );

    //Add searchAlogirthm
    ptree search_alg;
    search_alg.put( "name", "exhaustive" );
    XMLConfigTree.add_child( "Configuration.periscope.searchAlgorithm", search_alg );

    //Add searchAlogirthm for ATP
    ptree atp_search_alg;
    atp_search_alg.put( "name", "exhaustive_atp" );
    XMLConfigTree.add_child( "Configuration.periscope.atp.searchAlgorithm", atp_search_alg );

    //Add tuning_model file path
    ptree tuning_model;
    tuning_model.put( "file_path", string() );
    XMLConfigTree.add_child( "Configuration.periscope.tuningModel", tuning_model );

    //Add tuning_model file path
    ptree scorep;
    scorep.put( "tuningSubstrate", string() );
    XMLConfigTree.add_child( "Configuration.scorep", scorep );
}

void
ConfigModel::toXML( const string& filename )
{
#if BOOST_VERSION >= 105600
    write_xml( filename, XMLConfigTree, locale(), xml_writer_settings< ptree::key_type >( ' ', 4 ) );
#else
    write_xml( filename, XMLConfigTree, locale(), xml_writer_settings< char >( ' ', 4 ) );
#endif
}
