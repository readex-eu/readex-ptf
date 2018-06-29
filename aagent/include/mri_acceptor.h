/**
   @file    mri_acceptor.h
   @ingroup Communication
   @brief   Acceptor pattern header
   @author  Yury Oleynik
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

#ifndef MRI_ACCEPTOR_H_
#define MRI_ACCEPTOR_H_



#include "ace/INET_Addr.h"
#include "ace/Reactor.h"
#include "ace/SOCK_Acceptor.h"

class DataProvider;

/**
 * @class mriAcceptor
 * @ingroup AnalysisAgent
 *
 * @brief Accepts mri connection and handles them
 */
class mriAcceptor : public ACE_Event_Handler {
private:
    ACE_SOCK_Acceptor acceptor_;
    DataProvider*     dp_;


public:
    mriAcceptor() {
        dp_ = 0;
    }

    virtual ~mriAcceptor();

    /// get the I/O handle
    virtual ACE_HANDLE get_handle() const {
        return acceptor_.get_handle();
    }

    void set_dp( DataProvider* dp );

    /// called upon a new connection attempt
    virtual int handle_input( ACE_HANDLE hdle = ACE_INVALID_HANDLE );

    /// called when the handle is removed from the reactor
    virtual int handle_close( ACE_HANDLE       hdle,
                              ACE_Reactor_Mask close_mask );


    /**
     * Prepare to accept mri connections on the specified port
     * and add ourself to the reactors event loop.
     *
     * @param listen_addr Address to listen to
     * @return
     */
    int open( ACE_INET_Addr& listen_addr );

    /// get the local addr our acceptor_ is bound to
    int get_local_addr( ACE_INET_Addr& addr );
};

#endif /* MRI_ACCEPTOR_H */
