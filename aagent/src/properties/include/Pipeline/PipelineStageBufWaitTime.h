/**
   @file    PipelineStageBufWaitTime.h
   @ingroup PipelineProperties
   @brief   Buffer wait time of pipeline stage for Vienna pattern property header
   @author  Research Group Scientific Computing, University of Vienna
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

/**
 * @defgroup PipelineProperties Pipeline Properties
 * @ingroup Properties
 */

#ifndef PIPELINESTAGEBUFWAITTIME_H_
#define PIPELINESTAGEBUFWAITTIME_H_

#include "Property.h"
#include "PropertyID.h"

class PipelineStageBufWaitTime : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& phaseContext;
        ar& context;
        ar& threshold;
        ar& phaseCycles;
        ar& PipeStageBufWaitTime;
    }

private:
    Context* phaseContext;
    Context* context;
    double   threshold;
    INT64    phaseCycles;
    double   PipeStageBufWaitTime;

    PipelineStageBufWaitTime( void ); // should only be used for serialization

public: ~PipelineStageBufWaitTime( void );
    PipelineStageBufWaitTime( Context* ct,
                              Context* phaseCt,
                              double   threshold = 0.0 );

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    PropertyID id( void );

    std::string name( void );

    void print( void );

    std::string info( void );

    Context* get_phaseContext( void );

    Property* clone( void );
};

#endif /*PIPELINESTAGEBUFWAITTIME_H_*/
