/**
   @file	psc_config.h
   @brief   Periscope config file operations header
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

#ifndef PSC_CONFIG_H_INCLUDED
#define PSC_CONFIG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define PSC_CFGFILE ".periscope"


/*
 * open config file
 */
int psc_config_open( void );

int psc_config_open_file( char* fname );

int psc_config_write( void );

int psc_config_write_file( const char* fname );

/*
 * close config file
 */
void psc_config_close( void );

char* psc_config_update_str( const char* key,
                             const char* newValue );

int psc_config_update_int( const char* key,
                           int         newValue );

/*
 * get the machine name
 */
int psc_config_machname( char*  buf,
                         size_t len );


/*
 * get the site name
 */
int psc_config_sitename( char*  buf,
                         size_t len );


/*
 * get the hostname of the registry
 */
int psc_config_reghost( char*  buf,
                        size_t len );

int psc_config_reghost_init( char*  buf,
                             size_t len );

/*
 * get the application baseport
 */
int psc_config_appl_baseport( void );


/*
 * get the machine name
 */
int psc_config_agent_baseport( void );

int psc_config_ompnumthreads( void );


/*
 * get the port number of the registry
 */
int psc_config_regport( void );

int psc_config_regport_init( void );


/*
 * get the name of the target application
 */
int psc_config_appname( char*  buf,
                        size_t len );

/*
 * get the name of the host where frontend and HL are running (relevant for Bluegene P port phase I)
 */
int psc_config_frontend_host( char*  buf,
                              size_t len );

/*
 * get the name of the analysis strategy
 */
int psc_config_strategy( char*  buf,
                         size_t len );

/*
 * get the tag of the parent agent for the Analysis Agents (relevant for Bluegene P port phase I)
 */
int psc_config_aa_parent( char*  buf,
                          size_t len );

/*
 * get the tag of the Analysis Agents (relevant for Bluegene P port phase I)
 */
int psc_config_aa_tag( char*  buf,
                       size_t len );

/*
 * get the number of application processes per one Analysis Agent (relevant for Bluegene P port phase I)
 */
int psc_config_procs_per_aa( void );

/*
 * get the mode (relevant for Bluegene P port phase I)
 */
int psc_config_mode( void );

/*
 * get the number of application processes
 */
int psc_config_procs( void );

/*
 * get the debug level
 */
int psc_config_debug( void );

/*
 * get whether the application is instrumented
 */
int psc_config_uninstrumented( void );


#ifdef __cplusplus
}
#endif


#endif /* PSC_CONFIG_H_INCLUDED */
