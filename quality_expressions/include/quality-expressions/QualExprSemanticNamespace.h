/**
   @file    QualExprSemanticNamespace.h
   @ingroup QualityExpressionNamespace
   @brief   Evaluation of quality expressions - Semantic Namespace Interface
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

#ifndef QUALEXP_SEMANTIC_NAMESPACE_H_
#define QUALEXP_SEMANTIC_NAMESPACE_H_

#include <string>
#include <vector>

#include "quality-expressions/QualExprSemantic.h"

namespace quality_expressions_ns {
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

/**
   @class QualExprSemanticNamespace
   @brief Store a group of semantic definitions.
   @ingroup QualityExpressionNamespace
 */
class QualExprSemanticNamespace {
public:
    /* Destructor */ virtual ~QualExprSemanticNamespace( void ) {
    }

public:   // -- Namespace initialization API
    const std::string& groupNamespace( void ) const {
        return m_namespace;
    }                                                         //!< The semantic name.

    virtual void closeAllSemantics( void ) = 0;                            //!< Remove all semantic constructors.

public:                                                       // -- Semantic builder API
    virtual QualExprSemantic* buildNewSemantic( const std::string& semName ) const = 0; //!< Build a new semantic entry from the the list of constructors available.

    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;                                                 //!< Display the full namespace description.

protected:
    /* Constructor */ QualExprSemanticNamespace( const std::string& ns ) : m_namespace( ns ) {
    }

    const std::string m_namespace;                                                              //!< Semantic namespace name.
};

/**
   @class QualExprSemanticNamespaceStem
   @brief Store a group of sub-namespaces.
   @ingroup QualityExpressionNamespace
 */
class QualExprSemanticNamespaceStem : public QualExprSemanticNamespace {
public:
    /* Constructor */ QualExprSemanticNamespaceStem( const std::string& ns ) : QualExprSemanticNamespace( ns ), m_subNamespace() {
    }

    /* Destructor */ virtual ~QualExprSemanticNamespaceStem( void ) {
        closeAllSemantics();
    }

public:                                                           // -- Namespace initialization API
    void registerNewNamespace( const QualExprSemanticNamespace& ns );  //!< Append a sub-namespace in the group.

    virtual void closeAllSemantics( void );                                    //!< Remove all semantic constructors.

public:                                                           // -- Semantic builder API
    virtual QualExprSemantic* buildNewSemantic( const std::string& semName ) const;         //!< Build a new semantic entry from the the list of constructors available.

    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;           //!< Display the full namespace description.

private:
    std::vector<const QualExprSemanticNamespace*> m_subNamespace; //!< Store all sub namespaces.
};

/**
   @class QualExprSemanticNamespaceLeaf
   @brief Store a group of semantic definitions.
   @ingroup QualityExpressionNamespace
 */
class QualExprSemanticNamespaceLeaf : public QualExprSemanticNamespace {
public:
    /* Constructor */ QualExprSemanticNamespaceLeaf( const std::string& ns ) : QualExprSemanticNamespace( ns ), m_semanticList() {
    }

    /* Destructor */ virtual ~QualExprSemanticNamespaceLeaf( void ) {
        closeAllSemantics();
    }

public:                                                     // -- Namespace initialization API
    void registerNewSemantic( const QualExprSemantic* semDesc ); //!< Append a new semantic constructor in the group.

    virtual void closeAllSemantics( void );                              //!< Remove all semantic constructors.

public:                                                     // -- Semantic builder API
    virtual QualExprSemantic* buildNewSemantic( const std::string& semName ) const;   //!< Build a new semantic entry from the the list of constructors available.

    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;     //!< Display the full namespace description.

private:
    std::vector<const QualExprSemantic*> m_semanticList;    //!< Store all semantic constructors.
};
}

#endif
