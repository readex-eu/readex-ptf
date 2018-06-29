/**
   @file    QualityExpressions.h
   @ingroup QualityExpressionDataModel
   @brief   Basic quality expression entry
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

#ifndef QUALITYEXPRESSION_H_
#define QUALITYEXPRESSION_H_

#include <string>

#define BOOST_SERIALIZATION
#ifdef BOOST_SERIALIZATION
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/list.hpp>
#endif

/**
 * @defgroup QualityExpression Quality expressions package
 */

/**
 * @defgroup QualityExpressionDataModel Quality expressions data model
 * @ingroup  QualityExpression
 */

/**
   @class QualityExpression
   @brief Hold a quality expression
   @ingroup QualityExpressionDataModel

   A quality expression is a string representing a combination of measurements.
   The simplified syntax is the following:
   @verbatim
        (<metric name> | <event name>:<event operator><event attribute>) ...
          [<quality operator> ...
            (<metric name> | ..
              <event name>:<event operator><event attribute> | ...
                <immediate>)]*
   @endverbatim
   For more information see the document "PTF Quality Expression - Design
   Description" version 3 issued the 17/09/2014.
 */
class QualityExpression : public std::string {
public:         // Class Definitions
    /* Constructor */ QualityExpression( void ) : std::string( "" ) {
    }
    /* Constructor */ QualityExpression( const char* c_str ) : std::string( c_str ) {
    }
    /* Constructor */ QualityExpression( std::string& expression ) : std::string( expression ) {
    }
    /* Constructor */ QualityExpression( const QualityExpression& qual_expr ) : std::string( qual_expr ) {
    }
    /* Destructor */ ~QualityExpression( void ) {
    }

    const std::string& mangle( void ) const;                                                       //!< Return the mangled version of the expression.
    bool setMangledExpression( const std::string& mangled );                         //!< Set the expression value from the mangled expression. @return return true if success.

    bool operator==( const QualityExpression& rhs ) {
        return this->mangle() == rhs.mangle();
    }
    QualityExpression& operator=( const std::string& rhs ) {
        assign( rhs );
        return *this;
    }

    void display( const std::string& indent,
                  std::stringstream& s ) const;                                         //!< Display debugging information about the object.

#ifdef BOOST_SERIALIZATION
private:        // Serialization API
    friend class boost::serialization::access;

    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<std::string>( *this );
    }
#endif
};

#endif /* QUALITYEXPRESSION_H_ */
