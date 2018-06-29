/**
   @file	psc_errmsg.h
   @brief	Periscope debugging and logging output
   @author	Karl Fuerlinger
   @verbatim
        Revision:       $Revision$
        Revision date:  $Date$
        Committed by:   $Author$

        This file is part of the Periscope performance measurement tool.
        See http://www.lrr.in.tum.de/periscope for details.

        Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
        See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef PSC_ERRMSG_H_INCLUDED
#define PSC_ERRMSG_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// match the enum values with the string values
enum PSC_SELECTIVE_DEBUG_LEVEL {
    Autoinstrument = 1000,                      //1000
    AgentApplComm,                              //1001
    OnlineClustering,                           //1002
    PerformanceDatabase,                        //1003
    FrontendStateMachines,                      //1005
    FrontendStarters,                           //1006
    AutotuneAll,                                //1007
    AutotunePlugins,                            //1008
    AutotuneSearch,                             //1009
    AutotuneAgentStrategy,                      //1010
    ACECommunication,                           //1011
    AnalysisProperty,                           //1012
    HierarchySetup,                             //1013
    QualityExpressions,                         //1014
    QualityExpressionEvents,                    //1015
    RtsInfo,                                    //1016
    ApplTuningParameter,                        //1017
    LAST_SELECTIVE_DEBUG
};

#ifdef PSC_ERRMSG_C
const char* dbgLevelsDefs[] = {
    "Autoinstrument",                    //1000
    "AgentApplComm",                     //1001
    "OnlineClustering",                  //1002
    "PerformanceDatabase",               //1003
    "FrontendStateMachines",             //1005
    "FrontendStarters",                  //1006
    "AutotuneAll",                       //1007
    "AutotunePlugins",                   //1008
    "AutotuneSearch",                    //1009
    "AutotuneAgentStrategy",             //1010
    "ACECommunication",                  //1011
    "AnalysisProperty",                  //1012
    "HierarchySetup",                    //1013
    "QualityExpressions",                //1014
    "QualityExpressionEvents",           //1015
    "RtsInfo",                           //1016
    "ApplTuningParameter",               //1017
};

//If you add a level increase this constant.
//The new debug level will be 1000+number_dbgLevels-1.
// LM -- We can use enum for static computations.
const int number_dbgLevels = LAST_SELECTIVE_DEBUG - Autoinstrument;
#else
extern const char* dbgLevelsDefs[];
extern const int   number_dbgLevels;
#endif

void psc_abort( const char* message,
                ... );

void psc_errmsg( const char* fmt,
                 ... );

void psc_infomsg( const char* fmt,
                  ... );

char* psc_get_msg_prefix( void );

void psc_dbgmsg( unsigned int level,
                 const char*  fmt,
                 ... );

void psc_set_msg_prefix( const char* s );

void psc_set_progname( const char* s );

void psc_set_quiet( void );

void psc_unset_quiet( void );
int psc_get_debug_level( void );
int active_dbgLevel( int );

#ifdef __cplusplus
}
#endif

extern int psc_dbg_level;
#endif /* PSC_ERRMSG_H_INCLUDED */
