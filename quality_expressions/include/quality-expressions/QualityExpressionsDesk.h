/**
   @file    QualityExpressionsDesk.h
   @ingroup QualityExpressionCore
   @brief   Quality Expression manager headers
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

#ifndef QUALITYEXPRESSION_DESK_H_
#define QUALITYEXPRESSION_DESK_H_

/** @defgroup QualityExpressionCore Quality expressions core module
 *  @ingroup QualityExpression
 */

/** @defgroup QualityExpressionDeskC Quality expression Extern C desk
 *  @ingroup QualityExpressionCore
   @{
 */

#ifdef __cplusplus
extern "C" {
#endif

void QualExprDesk_globalInit( void );                                        //!< Global initialization.
int QualExprDesk_addCounter( int   metric,
                             char* expression );                        //!< Append a new quality expression indexed by a metric ID.
long long QualExprDesk_getLongCounter( int metric );                              //!< Get the value of a quality expression by a metric ID.
int QualExprDesk_resetCounters( void );                                     //!< Reset to 0 all quality expression values.
int QualExprDesk_removeCounters( void );                                    //!< Remove all quality expressions.
int QualExprDesk_startMeasures( void );                                     //!< Start measurements.
int QualExprDesk_stopMeasures( void );                                      //!< Stop measurements.

#ifdef __cplusplus
}
#endif

/** @} */

#ifdef __cplusplus
#include <string>
#include <exception>

#include "quality-expressions/QualityExpressions.h"
#include "quality-expressions/QualityExpressionsProfiler.h"

namespace quality_expressions_core {
class QualExprManager;
} // namespace quality_expressions_core

/**
   @class QualityExpressionsDesk
   @brief Global management of quality expressions, including events and evaluation.
   @ingroup QualityExpressionCore

   The global management of quality expressions consist is used for interfacing
   external profilers in order to get at runtimes counters, and for globally interfacing
   the registration of quality expressions.
 */
class QualityExpressionsDesk {
public:

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

public:
    // Quality expression management API
    /**
     * Destructor
     */
    ~QualityExpressionsDesk( void ) throw( Exception );

    size_t addCounter( pthread_t                     tid,
                       const QualityExpressionEntry& entry ) throw( Exception );                        //!< Append a quality expression evaluation request.

    long long getLongCounter( pthread_t             tid,
                              QualityExpressionID_T id ) throw( Exception );                            //!< Retrieve the current quality expression evaluation value.

    void resetCounters( pthread_t tid ) throw( Exception );                                                  //!< Reset all quality expressions to their neutral value.

    void removeCounters( pthread_t tid ) throw( Exception );                                                 //!< Remove a given quality expression.

    void enableMeasures( pthread_t tid );                    //!< Enable measurements.

    void disableMeasures( pthread_t tid );                   //!< Disable measurements.

public:                                                 // Profiling system API
    bool registerWithProfilers( void ) throw( Exception );   //!< Record this interface as an event listener for foreign profilers.

    bool unregisterWithProfilers( void ) throw( Exception ); //!< Remove this interface from registered event listeners in foreign profilers.

    void event( profiling_event_t* event ) throw( );  //!< Event handling of foreign profilers.

public:                                          // Static API
    static QualityExpressionsDesk* getGlobalManager( void ) throw( Exception ); //!< Return the Desk descriptor unique application wide.

private:
    quality_expressions_core::QualExprManager* m_instance;    //!< Manage the quality expressions profilers.

private:
    /**
     * Constructor
     */
    QualityExpressionsDesk( void ) throw( Exception );        //!< QualityExpressionsDesk constructor is only available via the getGlobalManager() method.
};

#endif /* __cplusplus */

#endif /* QUALITYEXPRESSION_DESK_H_ */
