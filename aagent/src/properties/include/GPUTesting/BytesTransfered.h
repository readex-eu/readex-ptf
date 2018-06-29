/**
   @file    BytesTransfered.h
   @ingroup CUDAProperties
   @brief   GPU Amount of bytes transferred by between a host and a device header
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

#ifndef BYTESTRANSFERED_H
#define BYTESTRANSFERED_H

#include "Property.h"
#include "PropertyID.h"


class BytesTransfered : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& phaseContext;
        ar& Bytes_Transfered;
        ar& evaluationResult;
    }


private:
    Context* phaseContext;
    INT64    Bytes_Transfered;
    double   evaluationResult;

    BytesTransfered( void ); // should only be used for serialization

public:
    BytesTransfered( Context* ct,
                     Context* phaseCt );

    virtual ~BytesTransfered( void );

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

#endif /* BYTESTRANSFERED_H_H */
