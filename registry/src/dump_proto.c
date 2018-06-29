/**
   @file	dump_proto.c
   @brief   Dump all registry commands
   @deprecated
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

#include <stdio.h>
#include "protocol.h"

int main( int argc, char* argv[] ) {
    printf( "MSG_WELCOME:\t %s", MSG_WELCOME );
    printf( "MSG_QUIT:\t %s", MSG_QUIT );

    printf( "MSG_UNKNOWN_COMMAND:\t %s", MSG_UNKNOWN_COMMAND );

    printf( "MSG_CREATE_BAD_FORMAT:\t %s", MSG_CREATE_BAD_FORMAT );
    printf( "MSG_CREATE_INCOMPLETE:\t %s", MSG_CREATE_INCOMPLETE );
    printf( "MSG_CREATE_SUCCESS:\t %s", MSG_CREATE_SUCCESS      );

    printf( "MSG_DELETE_BAD_FORMAT:\t %s", MSG_DELETE_BAD_FORMAT );
    printf( "MSG_DELETE_NOT_FOUND:\t %s", MSG_DELETE_NOT_FOUND  );
    printf( "MSG_DELETE_SUCCESS:\t %s", MSG_DELETE_SUCCESS      );
    printf( "MSG_LIST_SUCCESS:\t %s", MSG_LIST_SUCCESS          );

    printf( "MSG_SHOW_BAD_FORMAT:\t %s", MSG_SHOW_BAD_FORMAT );
    printf( "MSG_SHOW_NOT_FOUND:\t %s", MSG_SHOW_NOT_FOUND  );
    printf( "MSG_SHOW_SUCCESS:\t %s", MSG_SHOW_SUCCESS      );

    printf( "MSG_ADDSTR_BAD_FORMAT:\t %s", MSG_ADDSTR_BAD_FORMAT );
    printf( "MSG_ADDSTR_NOT_FOUND:\t %s", MSG_ADDSTR_NOT_FOUND  );
    printf( "MSG_ADDSTR_SUCCESS:\t %s", MSG_ADDSTR_SUCCESS      );

    printf( "MSG_GETSTR_BAD_FORMAT:\t %s", MSG_GETSTR_BAD_FORMAT );
    printf( "MSG_GETSTR_ENTRY_NOT_FOUND:\t %s", MSG_GETSTR_ENTRY_NOT_FOUND  );
    printf( "MSG_GETSTR_STRING_NOT_FOUND:\t %s", MSG_GETSTR_STRING_NOT_FOUND );
    printf( "MSG_GETSTR_SUCCESS:\t %s", MSG_GETSTR_SUCCESS        );
    printf( "MSG_LISTSTR_BAD_FORMAT:\t %s", MSG_LISTSTR_BAD_FORMAT );
    printf( "MSG_LISTSTR_NOT_FOUND:\t %s", MSG_LISTSTR_NOT_FOUND  );
    printf( "MSG_LISTSTR_SUCCESS:\t %s", MSG_LISTSTR_SUCCESS      );

    printf( "MSG_COUNTSTR_BAD_FORMAT:\t %s", MSG_COUNTSTR_BAD_FORMAT );
    printf( "MSG_COUNTSTR_NOT_FOUND:\t %s", MSG_COUNTSTR_NOT_FOUND  );
    printf( "MSG_COUNTSTR_SUCCESS:\t %s", MSG_COUNTSTR_SUCCESS      );
}
