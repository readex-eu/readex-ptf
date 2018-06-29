/**
   @file    hlagent.cc
   @ingroup Communication
   @brief   High-level agent
   @author  Karl Fuerlinger
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2013, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */


#include "hlagent.h"
#include "hagent_accl_handler.h"
#include "MetaProperty.h"
#include "selective_debug.h"
#include "config.h"

#include "ace/Reactor.h"
#include <math.h>

#ifdef __CLUSTERDEBUG
// dumping properties to files for debugging
#include <iostream>
#include <sstream>
#include <fstream>
#endif

bool search_done;
bool calltree_sent;

PeriscopeHLAgent::PeriscopeHLAgent( ACE_Reactor* r ): PeriscopeAgent( r ) {
    ACE_Event_Handler::reactor( r );
    nocluster            = false;
    gatherproperties_val = false;
}

int PeriscopeHLAgent::register_self() {
    if( !regsrv_ ) {
        psc_errmsg( "PeriscopeFrontend::register_self(): registry not set\n" );
        return -1;
    }

    int pid, port;
    pid  = getpid();
    port = get_local_port();

    char hostname[ 200 ];
    gethostname( hostname, 200 );

    EntryData data;
    data.id   = -1;
    data.app  = appname();
    data.site = sitename();
    data.mach = machinename();
    data.node = hostname;
    data.port = port;
    data.pid  = pid;
    data.comp = "Periscope HL Agent";
    data.tag  = get_local_tag();

    regid_ = regsrv_->add_entry( data );

    return regid_;
}



ACCL_Handler* PeriscopeHLAgent::create_protocol_handler( ACE_SOCK_Stream& peer ) {
    return new ACCL_HLAgent_Handler( this, peer );
}



void PeriscopeHLAgent::run() {
    psc_dbgmsg( 5, "Waiting for %d child agents\n", child_agents_.size() );

    ACE_Reactor* reactor = ACE_Event_Handler::reactor();

    if( reactor ) {
        // register SIGINT handler
        reactor->register_handler( SIGINT, this );

        /*
           ACE_Time_Value initial(2);
           ACE_Time_Value interval(1);

           reactor->schedule_timer(this, 0, initial, interval);

           ACE_Time_Value timeout = ACE_OS::gettimeofday();
           timeout += timeout_delta();
           set_timeout(timeout);
         */
#ifndef _BGP_PORT_HEARTBEAT_V1
// if not in heartbeat V1 mode then set the STARTUP timer and wait for all the predefined children to connect

        //TODO add timerless logic in the hagent
        set_timer( 2, 1, timeout_delta(), PeriscopeHLAgent::STARTUP );

#else
// else send own heartbeat to parent (will be forwarded to FE) and accept any agents sending heartbeat to this one and also forward their heartbeats up to FE
        if( parent_handler_ ) {
            parent_handler_->heartbeat( own_info_.hostname, own_info_.port,
                                        own_info_.tag, OWN_HEARTBEAT, 0 );
        }
        else {
            psc_errmsg( " parent_handler_ not set\n" );
            stop();
        }
        startup_mode_on();
#endif
        //TODO add timerless logic in the hagent goes here (now a runtime flag, instead of a configure variable)
        reactor->run_event_loop();
    }
    else {
        psc_errmsg( "Cannot access reactor\n" );
    }
}


void PeriscopeHLAgent::stop() {
    ACE_Reactor* reactor = ACE_Event_Handler::reactor();

    if( reactor ) {
        reactor->end_event_loop();
    }
#ifdef __p575
    regsrv_->delete_entry( regid_ );
#endif
}


int PeriscopeHLAgent::handle_signal( int signum, siginfo_t*, ucontext_t* ) {
    psc_dbgmsg( 2, "** signal handler called **\n" );
    stop();

    return 0;
}


int PeriscopeHLAgent::handle_input( ACE_HANDLE hdle ) {
    return 0;
}


void PeriscopeHLAgent::set_reinit_startup_timer() {
    set_timer( 2, 1, timeout_delta(), PeriscopeHLAgent::STARTUP_REINIT );
}


int PeriscopeHLAgent::handle_timeout( const ACE_Time_Value& time,
                                      const void*           arg ) {
    std::map< std::string, AgentInfo >::iterator it;
    bool                                         done = true;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "*** **** **** HLAtimeout %d\n", timer_action );

    switch( timer_action ) {
    case STARTUP:

        for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
            if( it->second.status != AgentInfo::STARTED ) {
                done = false;
                // psc_errmsg("Child agent %s at (%s:%d) not started\n",
                //            it->first.c_str(),
                //            it->second.hostname.c_str(),
                //            it->second.port);
            }
        }

        if( done ) {
            // send heartbeat to parent
            if( parent_handler_ ) {
                parent_handler_->heartbeat( own_info_.hostname, own_info_.port,
                                            own_info_.tag, OWN_HEARTBEAT, 0 );
                ACE_Reactor::instance()->cancel_timer( this );
            }
            else {
                psc_errmsg( " parent_handler_ not set\n" );
                stop();
            }
        }
        else {
            if( time > timeout_ ) {
#ifndef _BGP_PORT_HEARTBEAT_V1
                psc_errmsg( "Timed out waiting for child agent(s)\n" );
#endif
                //stop();
            }
        }
        break;

    case STARTUP_REINIT:

        for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
            if( it->second.status_reinit != AgentInfo::STARTED ) {
                done = false;
//        psc_errmsg("[reinit] Child agent %s at (%s:%d) not started\n",
//                   it->first.c_str(),
//                   it->second.hostname.c_str(),
//                   it->second.port);
            }
        }

        if( done ) {
            // send heartbeat to parent
            if( parent_handler_ ) {
                parent_handler_->heartbeat( own_info_.hostname, own_info_.port,
                                            own_info_.tag, OWN_HEARTBEAT, 0 );

                ACE_Reactor::instance()->cancel_timer( this );
            }
            else {
                psc_errmsg( " parent_handler_ not set\n" );
                stop();
            }
        }
        else {
            if( time > timeout_ ) {
                psc_errmsg( "Timed out waiting for child agent(s)\n" );
                stop();
            }
            //psc_dbgmsg(1,"HLA: waiting for child agents...\n");
        }
        break;

    case FOUNDPROP:
        clusterProps();
        ACE_Reactor::instance()->cancel_timer( this );
        break;

    case REQUESTCALLTREE:
      //  timer_set = false;

        ACE_Reactor::instance()->cancel_timer( this );
        break;
    }

    return 0;
}

/**
 * \deprecated
 */
void PeriscopeHLAgent::found_property( std::string hostname,
                                       std::string property,
                                       double      severity,
                                       double      confidence,
                                       int         numthreads,
                                       std::string context ) {
    if( nocluster ) {
        get_parent_handler()->foundprop( hostname, property, severity, confidence,
                                         numthreads,
                                         context );
    }
    /*
       else {
       info.name       = property;
       info.context    = context;
       info.host       = hostname;
       info.severity   = severity;
       info.confidence = confidence;
       info.numthreads = numthreads;

       psc_dbgmsg(1, "HLA found property, name = %s, host = %s, conf = %f, sev = %f, nth = %i, context = %s\n",
                 info.name.c_str(), info.host.c_str(), info.confidence, info.severity, info.numthreads, info.context.c_str());
       properties.push_back(info);

       if (!timer_set) {
        timer_set = true;
        set_timer(5, 5, 100, PeriscopeHLAgent::FOUNDPROP);
       }
       }
     */
}

void PeriscopeHLAgent::found_property( std::string& propData ) {
    //  psc_infomsg("HLAgent: found_property %s\n\n", req.xmlData.c_str());

    if( nocluster ) {
        get_parent_handler()->foundprop( propData );
    }
    else {
        MetaProperty prop = MetaProperty::fromXMLDeserialize( propData );
        psc_dbgmsg( 11, "*** Should try to cluster: %s with sev %f on proc %d\n",
                    prop.getName().c_str(), prop.getSeverity(), prop.getProcess() );

        std::string            prop_name = prop.getName().c_str();
        std::string::size_type p_loc     = prop_name.find( ":::::", 0 );
        if( p_loc != std::string::npos ) {
            properties_hotregionprop.push_back( prop );
        }
        else {
            properties.push_back( prop );
        }
//    std::cout << " properties_hotregionprop.size() " << properties_hotregionprop.size()
//              << " properties.size() " << properties.size() << std::endl;

//    get_parent_handler()->foundprop(propData);

//    timer_set = true;
//    set_timer(5, 5, 100, PeriscopeHLAgent::FOUNDPROP);

//    if (!timer_set) {
//      timer_set = true;
//      set_timer(5, 5, 100, PeriscopeHLAgent::FOUNDPROP);
//    }
    }
}


void PeriscopeHLAgent::send_calltree( std::string& calltreeData ) {
    get_parent_handler()->sendcalltree( calltreeData );
}


void PeriscopeHLAgent::set_timer( int         init,
                                  int         inter,
                                  int         max,
                                  TimerAction ta ) {
    ACE_Reactor* reactor = ACE_Event_Handler::reactor();

    ACE_Time_Value initial( init );
    ACE_Time_Value interval( inter );

    psc_dbgmsg( 10, "Schedule timer: init=%d interval=%d action=%d\n", init, inter, ta );

    reactor->schedule_timer( this, 0, initial, interval );

    ACE_Time_Value timeout = ACE_OS::gettimeofday();
    timeout += max;
    set_timeout( timeout );

    timer_action = ta;
}

/**
 * \brief Cluster the detected performance problems in similar groups
 *
 */
void PeriscopeHLAgent::clusterProps() {
    std::list < MetaProperty >        props;
    std::list<MetaProperty>::iterator it1, it2;
    MetaProperty                      nprop;

    psc_dbgmsg( 3, "HLA trying to cluster %d properties...\n", properties.size() );

    if( gatherproperties_val ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( OnlineClustering ), "HLA gathering properties...\n" );

        for( it1 = properties.begin(); it1 != properties.end(); it1++ ) {
            if( it1->done ) {
                continue;
            }

            // std::string host = it1->host;
            int    instances = it1->getThread();
            double sev       = it1->getSeverity();
            double conf      = it1->getConfidence();

            for( it2 = it1, it2++; it2 != properties.end(); it2++ ) {
                if( ( it2->getId().compare( it1->getId() ) == 0 ) && ( it2->getRegionId().compare( it1->getRegionId() ) == 0 ) ) {
                    sev       += it2->getSeverity();
                    conf      += it2->getConfidence();
                    instances += it2->getThread();
                    it2->done  = true;
                }
            }

            if( instances < 1 ) {
                instances = 1;
            }

            nprop.setCluster( true );

            nprop.setId( it1->getId() );

            nprop.setName( it1->getName() );

            nprop.setFileId( it1->getFileId() );
            nprop.setFileName( it1->getFileName() );
            nprop.setStartPosition( it1->getStartPosition() );
            nprop.setConfiguration( it1->getConfiguration() );
            nprop.setRegionType( it1->getRegionType() );
            nprop.setRegionId( it1->getRegionId() );

            nprop.setConfidence( conf / ( ( double )instances ) );
            nprop.setSeverity( sev / ( ( double )instances ) );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( OnlineClustering ), "HLA foundprop timeout, name = %s, conf = %f, sev = %f, #propsClustered = %i, regId = %s\n",
                        nprop.getName().c_str(), nprop.getConfidence(), nprop.getSeverity(), instances, nprop.getRegionId().c_str() );

            props.push_back( nprop );
        }
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( OnlineClustering ), "HLA clustering %d properties with similar severity(%f)...\n\n",
                    properties.size(), CLUSTER_SEV_TH );

#ifdef __CLUSTERDEBUG
        // Dump all properties to a file for comparison
        std::ofstream      allPropsFile;
        std::ostringstream os;
        os << "allProps_HL_" << get_local_tag() << "." << appname() << ".psc";
        allPropsFile.open( os.str().c_str(), ios::out | ios::app );
        for( it1 = properties.begin(); it1 != properties.end(); it1++ ) {
            allPropsFile << it1->toXML() << std::endl;
        }
        allPropsFile.close();
#endif

        for( it1 = properties.begin(); it1 != properties.end(); it1++ ) {
            if( it1->done ) {
                continue;
            }

            int         instances = 1;
            double      sev       = it1->getSeverity();
            double      conf      = it1->getConfidence();
            std::string reqrgns;

            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( OnlineClustering ), "Clustering1: id = %s regId = %s proc = %d sev = %f conf = %f\n",
                        it1->getId().c_str(), it1->getRegionId().c_str(), it1->getProcess(), it1->getSeverity(), it1->getConfidence() );

            it2 = it1;
            it2++;
            for(; it2 != properties.end(); it2++ ) {
                if( it2->done ) {
                    continue;
                }

                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( OnlineClustering ), "Clustering2: id = %s regId = %s proc = %d sev = %f conf = %f\n",
                            it2->getId().c_str(), it2->getRegionId().c_str(), it2->getProcess(), it2->getSeverity(), it2->getConfidence() );

                if( ( it2->getId().compare( it1->getId() ) == 0 ) &&
                    ( it2->getRegionId().compare( it1->getRegionId() ) == 0 ) &&
                    fabs( it2->getSeverity() - it1->getSeverity() ) < CLUSTER_SEV_TH &&
                    fabs( it2->getConfidence() - it1->getConfidence() ) < CLUSTER_CONF_TH ) {
                    sev  += it2->getSeverity();
                    conf += it2->getConfidence();

                    if( strcmp( it1->getName().c_str(), "Required regions in the next experiment" ) == 0 ) {
                        //psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(OnlineClustering), "Clustering required regions property: Id %s region id: %s \n",
                        //           it1->getId(), it1->getRegionId());
                        std::string                                  str1, str2, merged;
                        std::map < std::string, std::string >        rcv;
                        std::map<std::string, std::string>::iterator itr;

                        rcv = it1->getExtraInfo();
                        itr = rcv.begin();
                        if( itr != rcv.end() ) {
                            str1 = itr->second;
                        }
                        else {
                            str1 = "";
                        }

                        rcv = it2->getExtraInfo();
                        itr = rcv.begin();
                        if( itr != rcv.end() ) {
                            str2 = itr->second;
                        }
                        else {
                            str2 = "";
                        }

                        merged = mergeStrings( str1, str2 );
                        nprop.addExtraInfo( "", merged );
                        //psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(OnlineClustering), "merged %s \n", merged.c_str());
                    }
                    // if the property is already clustered..
                    if( it2->getCluster() ) {
                        nprop.addExecObjs( it2->getExecObjs() );
                    }
                    else {
                        nprop.addExecObj( it2->getProcess(), it2->getThread() );
                    }

                    instances++;
                    it2->done = true;
                    nprop.setCluster( true );
                }
            }

            // If successfully clustered some properties, send the resulting clustered prop
            if( nprop.getCluster() ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( OnlineClustering ), "HLA clustering done. Creating new prop...\n" );

                // if the property is already clustered..
                if( it1->getCluster() ) {
                    nprop.addExecObjs( it1->getExecObjs() );
                }
                else {
                    nprop.addExecObj( it1->getProcess(), it1->getThread() );
                }

                /*********** Load testing ******/
                //for(int kk = 0; kk < 3000000; kk++)
                //  nprop.addExecObj(10 + kk,10 + kk);

                nprop.setId( it1->getId() );

                nprop.setName( it1->getName() );

                nprop.setFileId( it1->getFileId() );
                nprop.setFileName( it1->getFileName() );
                nprop.setStartPosition( it1->getStartPosition() );
                nprop.setConfiguration( it1->getConfiguration() );
                nprop.setRegionType( it1->getRegionType() );
                nprop.setRegionId( it1->getRegionId() );

                nprop.setConfidence( conf / ( ( double )instances ) );
                nprop.setSeverity( sev / ( ( double )instances ) );

                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( OnlineClustering ), "HLA foundprop clustered %d props: id = %s, conf = %f, sev = %f, regId = %s\n",
                            instances, nprop.getId().c_str(), nprop.getConfidence(), nprop.getSeverity(), nprop.getRegionId().c_str() );

                props.push_back( nprop );
                nprop = MetaProperty::EmptyProp();
            }
            else {
                // if not clustered, send the original property
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( OnlineClustering ), "HLA clustering done. Adding original property: id = %s, conf = %f, sev = %f, regId = %s\n",
                            ( *it1 ).getId().c_str(), ( *it1 ).getConfidence(), ( *it1 ).getSeverity(), ( *it1 ).getRegionId().c_str() );
                props.push_back( *it1 );
            }
        }
    }

    psc_dbgmsg( 3, "HLA clustering resulted in %d props...\n", props.size() );

#ifdef __CLUSTERDEBUG
    // Dump clustered properties to a file for comparison
    std::ofstream      clusteredPropsFile;
    std::ostringstream os;
    os << "clusteredProps_HL_" << get_local_tag() << "." << appname() << ".psc";
    clusteredPropsFile.open( os.str().c_str(), ios::out | ios::app );
#endif

    // Send the clustered properties to the parent
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( OnlineClustering ), "HLA sending clustered props...\n" );
    for( it1 = props.begin(); it1 != props.end(); it1++ ) {
        std::string propData = it1->toXMLSerialize();
        get_parent_handler()->foundprop( propData );

#ifdef __CLUSTERDEBUG
        clusteredPropsFile << propData << std::endl;
#endif
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( OnlineClustering ), "HLA done sending clustered props!\n" );

#ifdef __CLUSTERDEBUG
    clusteredPropsFile.close();
#endif
    properties.clear();
}

std::string PeriscopeHLAgent::mergeStrings( std::string str1,
                                            std::string str2 ) {
    std::string merged, checkstr;
    size_t      start, stop;

    merged = str2;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( OnlineClustering ), "Merging str1 ::%s:: str2 ::%s:: merged\n",
                str1.c_str(), str2.c_str() );
    start = str1.find( "f: " );
    if( str1.size() > 1 && str2.size() > 2 ) {
        while( ( int )start != -1 && ( int )stop != -1 ) {
            stop     = str1.find( "\n", start );
            checkstr = str1.substr( start, stop - start );
            if( merged.find( checkstr, 0 ) == -1 ) {
                merged.append( checkstr );
            }
            start = str1.find( "f: ", stop );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( OnlineClustering ), "in while ::%s:: start %d stop %d\n",
                        checkstr.c_str(), ( int )start, ( int )stop );
        }
    }
    return merged;
}

/**
 * \brief Execution time for scalability requirements
 *
 */
void PeriscopeHLAgent::addHotRegionProps() {
    std::list<MetaProperty>::iterator it;
    if( properties_hotregionprop.size() > 0 ) {
        for( it = properties_hotregionprop.begin(); it != properties_hotregionprop.end(); ++it ) {
            std::string hotRegionProp = it->toXMLSerialize();
            get_parent_handler()->foundprop( hotRegionProp );
        }
        properties_hotregionprop.clear();
    }
    else {
        ;
    }
}
