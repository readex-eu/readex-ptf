/**
   @file    QualExprAggregator.h
   @ingroup QualityExpressionEvaluation
   @brief   Evaluation of quality expressions - aggregator interface
   @author  Laurent Morin
   @verbatim
   Revision:       $Revision$
   Revision date:  $Date$
   Committed by:   $Author$

   This file is part of the Periscope performance measurement tool.
   See http://www.lrr.in.tum.de/periscope for details.

   Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
   See the COPYING file in the base directory of the package for details.

   @endverbatim
 */

#ifndef QUALEXP_AGGREGATOR_H_
#define QUALEXP_AGGREGATOR_H_

#include "qualexpr-profiler/QualExprProfiler.h"

namespace quality_expressions_core {
/**
   @class QualExprAggregator
   @brief Interface for the definition of a particular aggregator.
   @ingroup QualityExpressionEvaluation
 */
class QualExprAggregator {
protected:
    /* Constructor */ QualExprAggregator( size_t id ) : m_id( id ) {
    }                                                                                           //!< Pure interface, no direct constructor.

public:
    /* Constructor */ virtual ~QualExprAggregator( void ) {
    }

    size_t getId( void ) const {
        return m_id;
    }                                                                                           //!< Aggregator unique ID.

    virtual const char* name( void ) const = 0;                                                                     //!< Aggregator fully qualified name.

    virtual const char* description( void ) const = 0;                                                              //!< Display a description of the aggregator - debug.

    virtual void processEvent( const QualExprEvent& event ) = 0;                                             //!< Aggregate the given event.

    virtual QualExprAggregator* build( size_t id ) const = 0;                                                               //!< Operate as an aggregator constructor node.

    virtual void reset( void ) = 0;                                                                          //!< Reset the aggregator state.

    virtual void display( const std::string& indent,
                          std::stringstream& s ) const = 0;                                                     //!< Display debugging information about the object.

private:
    size_t m_id;                                        //!< Unique aggregator id.
};
}

#endif
