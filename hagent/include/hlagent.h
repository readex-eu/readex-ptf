/**
   @file    hlagent.h
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


#ifndef HLAGENT_H_INCLUDED
#define HLAGENT_H_INCLUDED

#include <list>
#include "psc_agent.h"
#include "ace/Reactor.h"

#include "MetaProperty.h"
#include "hagent_accl_statemachine.h"
using namespace hagent_accl_msm_namespace;

#define CLUSTER_SEV_TH 1.5
#define CLUSTER_CONF_TH 1.5

// Create an high level agents group
/// @defgroup HLAgent High Level Agent

/**
 * @class PeriscopeHLAgent
 * @ingroup HLAgent
 *
 * @brief Periscope communication agents
 *
 * This class extends the ACE_Event_Handler and implements
 * the communication agents
 *
 * @sa PeriscopeAgent
 */
class PeriscopeHLAgent : public PeriscopeAgent, public ACE_Event_Handler {
public:
    enum TimerAction {
        STARTUP         = 0,
        STARTUP_REINIT  = 100,
        FOUNDPROP       = 1000,
        REQUESTCALLTREE = 1001
    };

    bool nocluster;

private:
    TimerAction               timer_action;
    std::list< MetaProperty > properties;
    std::list< MetaProperty > properties_hotregionprop;
    bool                      gatherproperties_val;

public:
    PeriscopeHLAgent( ACE_Reactor* r );

    // register with registry service
    int register_self();

    void reinit_providers( int maplen,
                           int idmap_from[ 8192 ],
                           int idmap_to[ 8192 ] ) {
    };

    // create a suitable handler for the ACCL protocol
    ACCL_Handler* create_protocol_handler( ACE_SOCK_Stream& peer );

    void run();

    void stop();

    int handle_input( ACE_HANDLE hdle );

    int handle_timeout( const ACE_Time_Value& time,
                        const void*           arg );

    int handle_signal( int        signum,
                       siginfo_t* t,
                       ucontext_t* );

    void found_property( std::string hostname,
                         std::string property,
                         double      severity,
                         double      confidence,
                         int         numthreads,
                         std::string context );

    void found_property( std::string& propData );

    void send_calltree( std::string& calltreeData );

    void set_timer( int         init,
                    int         inter,
                    int         max,
                    TimerAction ta );

    void set_reinit_startup_timer();

    void dontcluster() {
        nocluster = true;
    }

    void gatherproperties() {
        gatherproperties_val = true;
    }

    void clusterProps();

    std::string mergeStrings( std::string,
                              std::string );

    void addHotRegionProps();
};

#endif /* HLAGENT_H_INCLUDED */
