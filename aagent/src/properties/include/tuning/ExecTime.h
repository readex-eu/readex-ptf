/**
   @file    ExecTime.h
   @ingroup TuningProperties
   @brief   Return the execution time of the region in thread 0 as severity header
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
 * @defgroup TuningProperties Tuning Properties
 * @ingroup Properties
 */

#ifndef EXECTIME_H_
#define EXECTIME_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

class ExecTimeProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& ExecTime;
    }

private:
    double threshold;
    INT64  phaseCycles;
    double ExecTime;



public:
    ExecTimeProp( void ); // should only be used for serialization

    ExecTimeProp( Context* ct,
                  Context* phaseCt,
                  double   threshold = 0.0 );

    ~ExecTimeProp( void );

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    PropertyID id( void );

    std::string name( void );

    void print( void );

    std::string info( void ) {
        return "";
    }

    /**
     * @brief Additional XML property information
     */
    std::string toXMLExtra( void );

    Context* get_phaseContext( void );

    Property* clone( void );
};

//BOOST_CLASS_EXPORT_KEY(ExecTimeProp);

#endif /*EXECTIME_H_*/
