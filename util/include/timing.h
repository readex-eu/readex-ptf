/**
   @file	timing.h
   @brief	Periscope timing routines header
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

#ifndef TIMING_H_INCLUDED
#define TIMING_H_INCLUDED

#include <sys/time.h>
#include <stdbool.h>

#define PSC_WTIME( time_ )                       \
    {                                            \
        struct timeval tv;                       \
        gettimeofday( &tv, NULL );               \
        time_ = tv.tv_sec + tv.tv_usec * 1.0e-6; \
    }

extern double psc_start_time;

extern double  atp_s_timeout;

#ifdef __cplusplus
extern "C" {
#endif


double psc_wall_time( void );

void psc_init_start_time( void );

void set_atp_server_timeout (double g_timeout_);

double get_atp_server_timeout ();

bool is_timed_out ();

#ifdef __cplusplus
}
#endif

#endif /* TIMING_H_INCLUDED */
