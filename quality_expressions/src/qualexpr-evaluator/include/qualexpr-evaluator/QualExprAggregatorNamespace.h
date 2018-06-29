/**
   @file    QualExprAggregatorNamespace.h
   @ingroup QualityExpressionEvaluation
   @brief   Evaluation of quality expressions - aggregator namespace
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

#ifndef QUALEXP_AGGREGATOR_NAMESPACE_H_
#define QUALEXP_AGGREGATOR_NAMESPACE_H_

#include "qualexpr-evaluator/QualExprAggregator.h"

namespace quality_expressions_core {
/**
   @class QualExprAggregatorNamespace
   @brief Store a group of aggregator functions.
   @ingroup QualityExpressionEvaluation
 */
class QualExprAggregatorNamespace {
public:
    /* Constructor */ QualExprAggregatorNamespace( const std::string& ns ) : m_namespace( ns ), m_aggregatorList() {
    }

    /* Constructor */ ~QualExprAggregatorNamespace( void ) {
        closeAllAggregators();
    }

public:   // -- Namespace initialization API
    void registerNewAggregator( const char op, const QualExprAggregator* aggregDesc ) {
        m_aggregatorList.push_back( aggregDesc );
    }                                                                                                                                                                   //!< Append a new aggregator constructor in the namespace.

    void closeAllAggregators( void );                                                                                                                                        //!< Remove all aggregator constructors.

    const std::string& groupNamespace( void ) const {
        return m_namespace;
    }     //!< The namespace name - shall be globally unique.

public:   // -- Aggregator builder API
    QualExprAggregator* buildNewAggregator( const char         op,
                                            const std::string& aggregatorName,
                                            size_t             id );                                                    //!< Build a new aggregator entry from the the list of constructors available.

    void display( const std::string& indent,
                  std::stringstream& s ) const;                                                                         //!< Display debugging information about the object.

private:
    const std::string                      m_namespace;                 //!< Aggregator group namespace.
    std::vector<const QualExprAggregator*> m_aggregatorList;            //!< Store all aggregator constructors.
};
}

#endif
