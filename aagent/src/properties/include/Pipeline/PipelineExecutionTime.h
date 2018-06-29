/**
   @file    PipelineExecutionTime.h
   @ingroup PipelineProperties
   @brief   Execution time of pipeline for Vienna pattern property header
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

#ifndef PIPELINEEXECUTIONTIME_H_
#define PIPELINEEXECUTIONTIME_H_

#include "Property.h"
#include "PropertyID.h"

class PipelineExecutionTime : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& phaseContext;
        ar& context;
        ar& threshold;
        ar& phaseCycles;
        ar& PipelineExecTime;
    }

private:
    Context* phaseContext;
    Context* context;
    double   threshold;
    INT64    phaseCycles;
    double   PipelineExecTime;

    PipelineExecutionTime( void ); // should only be used for serialization

public: ~PipelineExecutionTime( void );
    PipelineExecutionTime( Context* ct,
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

#endif /*PIPELINEEXECUTIONTIME_H_*/
