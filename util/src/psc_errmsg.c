#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define PSC_ERRMSG_C
#include "psc_errmsg.h"

#include "timing.h"

#if defined HAVE__PROGNAME || defined __CRAY
extern const char* __progname;        /* Program name supported by GNUC */
#else
char __progname[ 80 ] = { '\0' };     /* Not supported on some other architectures! */
#define __NO_PROGNAME
#endif

char msg_prefix[ 80 ] = { '\0' };
int  psc_quiet = 0;
int  psc_dbg_level;

char* psc_get_msg_prefix( void ) {
    return msg_prefix;
}

void psc_abort( const char* message, ... ) {
    va_list args;

    va_start( args, message );
    psc_errmsg( message, args );
    va_end( args );

    abort();
}

void psc_errmsg( const char* fmt, ... ) {
    va_list args;
    fprintf( stderr, "[%s][ERR%s] ", __progname, msg_prefix );

    va_start( args, fmt );
    ( void )vfprintf( stderr, fmt, args );
    va_end( args );

    fflush( stderr );
}

void psc_infomsg( const char* fmt, ... ) {
    va_list args;
    fprintf( stdout, "[%s][INFO%s] ", __progname, msg_prefix );

    va_start( args, fmt );
    ( void )vfprintf( stdout, fmt, args );
    va_end( args );

    fflush( stdout );
}

int psc_get_debug_level( void ) {
    return psc_dbg_level;
}


void psc_dbgmsg( unsigned int level, const char* fmt, ... ) {
    va_list args;
//  double time;
//
//  PSC_WTIME(time);
//  clock_t start;
//  start = clock();


    if( level <= psc_dbg_level || active_dbgLevel( level ) == 1 ) {
        if( level < 1000 ) {
            fprintf( stderr, "[%s][DBG:%u%s] ", __progname, level, msg_prefix );
        }
        else {
            fprintf( stderr, "[%s][DBG:%s%s] ", __progname, dbgLevelsDefs[ level - 1000 ],
                     msg_prefix );
        }

        va_start( args, fmt );
        ( void )vfprintf( stderr, fmt, args );
        va_end( args );

        fflush( stderr );
    }
}


void psc_set_msg_prefix( const char* s ) {
    if( !s ) {
        msg_prefix[ 0 ] = '\0';
    }
    else {
        sprintf( msg_prefix, ":%s", s );
    }
}

void psc_set_progname( const char* s ) {
#if defined __NO_PROGNAME
    if( !s ) {
        __progname[ 0 ] = '\0';
    }
    else {
        sprintf( __progname, "%s", s );
    }
#endif
}


void psc_set_quiet( void ) {
    psc_quiet = 1;
}

void psc_unset_quiet( void ) {
    psc_quiet = 0;
}
