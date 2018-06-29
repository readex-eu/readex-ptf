/**
   @file    QualityExpressionsDB.h
   @ingroup QualityExpressionDataModel
   @brief   Main header for quality expressions DB
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

#ifndef QUALITYEXPRESSIONDB_H_
#define QUALITYEXPRESSIONDB_H_

#include <map>
#include <vector>

/**
   @class QualityExpressionDB
   @brief Manage at a global scale all the quality expressions declared and allocated them
   @ingroup QualityExpressionDataModel

   The quality expression database is in charge of assuring the unicity allocation of
   quality expressions for the measurement.
   A valid quality expressions id (of type QualityExpressionID_T) is an integer > 0.

   @sa QualityExpression
   @sa QualityExpressionEntry
 */
class QualityExpressionDB {
public: // Data structure Declarations
    typedef std::map<std::string, QualityExpressionEntry*> QualityExpressionSet_T;
    typedef std::map<QualityExpressionID_T, QualityExpressionEntry*> QualityExpressionMapById_T;

/**
   @class Exception
   @brief Hold an exception with a description.
 */
    class Exception : public std::exception, public std::string {
public:
        /* Constructor */ Exception( const char* message ) throw( ) : std::string( message ) {
        }

        /* Destructor */ ~Exception( void ) throw( ) {
        }

        virtual const char* what() const throw( )                     {
            return c_str();
        }
    };

public:         // Class Definitions
    /* Constructor */ QualityExpressionDB( void );
    /* Destructor */ ~QualityExpressionDB( void );

    //** Register the quality expression and return the associated list of ids. */
    void registerQualityExpression( const QualityExpression&            expression,
                                    std::vector<QualityExpressionID_T>& p_idList );

    const QualityExpression& getQualityExpressionById( QualityExpressionID_T id ) throw( Exception );                                    //!< Fetch the quality expression associated to the ID. */

    void clear( void );                                                                                              //!< Reset the database.

    void display( const std::string& indent,
                  std::stringstream& s ) const; //!< Display debugging information about the object.

private:                                   // Internal functions
    //** Register a single quality expression. */
    void registerQualityExpressionEntry( const QualityExpression&            expression,
                                         std::vector<QualityExpressionID_T>& p_idList );

private:        // Data structures
    QualityExpressionID_T      m_idIterator;
    QualityExpressionSet_T     m_qualityExpressionsSet;
    QualityExpressionMapById_T m_qualityExpressionsByID;
};

#endif // QUALITYEXPRESSIONDB_H_
