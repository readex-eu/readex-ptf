/**
   @file    ExecutionTime.h
   @ingroup CUDAProperties
   @brief   GPU kernel execution time property header
   @author  Robert Mijakovic
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

#ifndef EXECUTIONTIME_H
#define EXECUTIONTIME_H

#include "Property.h"
#include "PropertyID.h"


class ExecutionTime : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& phaseContext;
        ar& Execution_Time;
        ar& evaluationResult;
    }

private:
    Context* phaseContext;
    INT64    Execution_Time;
    double   evaluationResult;

    ExecutionTime( void ); // should only be used for serialization

public:
    ExecutionTime( Context* ct,
                   Context* phaseCt );

    virtual ~ExecutionTime( void );

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    PropertyID id( void );

    std::string name( void );

    void print( void );

    std::string info( void );

    /**
     * @brief Additional XML property information
     */
    std::string toXMLExtra( void );

    Context* get_phaseContext( void );
    Property* clone( void );
};

#endif /* EXECUTIONTIME_H */
