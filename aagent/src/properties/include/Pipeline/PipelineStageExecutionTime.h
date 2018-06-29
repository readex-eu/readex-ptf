/**
   @file    PipelineStageExecutionTime.h
   @ingroup PipelineProperties
   @brief   Execution time of pipeline stage for Vienna pattern property header
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

#ifndef PIPELINESTAGEEXECUTIONTIME_H_
#define PIPELINESTAGEEXECUTIONTIME_H_

#include "Property.h"
#include "PropertyID.h"

class PipelineStageExecutionTime : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& phaseContext;
        ar& context;
        ar& threshold;
        ar& phaseCycles;
        ar& PipelineStageExecTime;
    }

private:
    Context* phaseContext;
    Context* context;
    double   threshold;
    INT64    phaseCycles;
    double   PipelineStageExecTime;

    PipelineStageExecutionTime( void ); // should only be used for serialization

public: ~PipelineStageExecutionTime( void );
    PipelineStageExecutionTime( Context* ct,
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

#endif /*PIPELINESTAGEEXECUTIONTIME_H_*/
