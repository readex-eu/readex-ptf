/**
   @file    Context.h
   @ingroup AnalysisAgent
   @brief   Execution context header
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
#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "global.h"
#include "application.h"
#include "rts.h"
/**
 * @class Context
 * @ingroup AnalysisAgent
 *
 * @brief Execution environment of the current property
 *
 */
class Context {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& rank;
        ar& thread;
        ar& reg;
        ar& rtsBased;
    }
private:
    int     rank;       ///< MPI rank of the process
    int     thread;     ///< Thread number
    Region* reg;        ///< Region where the property occurred
    Rts*    rts;        ///< Rts where the property occurred
    int     aa_rts_id;
    bool    rtsBased;

public:
    Context( Region* reg,
             int     rank     = 0,
             int     thread_p = 0 );
    Context( Rts*    rts_node,
             int     rank     = 0,
             int     thread_p = 0 );
    Context( int fileId,
             int rfl,
             int rank     = 0,
             int thread_p = 0 );
    virtual ~Context();

    /**
     * RegionId getter.
     *
     * @return the Region ID from the mapping
     */
    std::string getRegionId();

    Rts* getRts();

    /**
     * FileId getter.
     * @return the File ID from the mapping
     */
    int getFileId();

    /**
     * Source line number getter.
     * @return the source line number where the property occurred
     */
    int getRfl();

    /**
     * MPI rank getter.
     * @return the MPI rank of the process
     */
    int getRank();

    /**
     * Thread number getter.
     * @return the thread number
     */
    int getThread();

    /**
     * The first line of region getter.
     * @return the first line of region
     */
    int getStartPosition();

    /**
     * File name getter.
     * @return the file name
     */
    std::string getFileName();

    /**
     * Region getter.
     * @return the region where the property occurred
     */
    Region* getRegion();

    /**
     * Region type getter.
     * @return the region type
     */
    RegionType getRegionType();

    /**
     * Aagent rts ID getter.
     * @return the rts ID
     */
    int getRtsID();

    /**
     * Returns true if the context contains Rts information
     */
    bool isRtsBased();

    /**
     * Creates a copy of the context
     */
    Context* copy();

    std::string getCallpath();
};

#endif /*CONTEXT_H_*/
