/**
   @file    BisonFlexInterface.h
   @ingroup QualityExpressionEvaluation
   @brief   Generic interface for standard C++ Flex/Bison parsers.
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

#ifndef BISONFLEXINTERFACE_H
#define BISONFLEXINTERFACE_H

# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <climits>
# include <string>
# include <vector>
# include <iostream>
#include <exception>

// --
// -- General Interface for a Bison/Flex parser
// --
template<class Grammar>
class ParserInterface {
public:
    typedef yy ( scan_t )    scanner_t;         //!< Opaque pointer on the scanner internal data structures (lexer reentrant)
    typedef yy ( buffer_t )  scan_buffer_t;     //!< Opaque pointer on the internal lexer buffer

    /**
       @class Exception
       @brief Hold an exception with a description and a location.
     */
    class Exception : public std::exception, public std::string {
public:
        /* Constructor */ Exception( const location_t& l, const std::string& m ) throw( ) : std::string( m ), m_location( l ) {
        }
        /* Destructor */ ~Exception( void ) throw( ) {
        }
        virtual const char* what( void ) const throw( );

        const location_t m_location;
    };

public:
    /* Constructor */ ParserInterface( void );
    /* Destructor */ ~ParserInterface( void );

    // Flags and values.
    void setTraceScanning( bool value = true ) {
        m_trace_scanning = value;
    }
    void setTraceParsing( bool value = true ) {
        m_trace_parsing = value;
    }
    void setFilename( const std::string& filename ) {
        m_filename = filename;
    }

    std::string* getFilename( void ) {
        return &m_filename;
    }
    bool getParsingTrace( void ) const {
        return m_trace_parsing;
    }

    // Handling the parser, return error code.
    scanner_t begin_parse( const std::string& filename );
    scanner_t begin_parse( const std::string& filename,
                           const std::string& buffer );
    int end_parse( void );

public:  // Interface for the Parser

    typedef typename Grammar::token Token;
    typedef typename Grammar::token_type TokenType;
    typedef typename Grammar::semantic_type SemanticType;
    typedef typename Grammar::location_type LocationType;

    FILE* getLexFile( void ) {
        return yy( get_in ) ( m_scanner );
    }
    void setLexFile( FILE* filed ) {
        return yy( set_in ) ( filed, m_scanner );
    }
    int getLexDebugFlag( void ) {
        return yy( get_debug ) ( m_scanner );
    }
    void setLexDebugFlag( int debug ) {
        return yy( set_debug ) ( debug, m_scanner );
    }

    // Handling the scanner.
    void scan_begin_filed( void );
    void scan_end_filed( void );

    void user_action( LocationType& loc, int leng ) {
        loc.columns( leng );
        m_defaultLocation = loc;
    }
    void lex_step( LocationType& loc ) {
        loc.step();
        m_defaultLocation = loc;
    }
    void new_line( LocationType& loc, int leng ) {
        loc.lines( leng );
        loc.step();
        m_defaultLocation = loc;
    }

    TokenType scan_integer( SemanticType& lval,
                            LocationType& lloc,
                            const char*   text );
    TokenType scan_identifier( SemanticType& lval,
                               LocationType& lloc,
                               const char*   text );

    // Error handling.
    void error( const location_t&  l,
                const std::string& m );
    void error( const std::string& m );

private:                                            // Members //
    scanner_t                  m_scanner;           //!< Opaque scanner data structures */
    std::string                m_filename;          //!< Name of the file parsed */
    bool                       m_trace_scanning;    //!< Activate the scanner trace generation */
    bool                       m_trace_parsing;     //!< Activate the parsing trace generation */
    std::vector<scan_buffer_t> m_scannerStreamData; //!< Stack of input buffers for the scanner */
    std::vector<const char*>   m_streamData;        //!< Input buffers allocated */
    location_t                 m_defaultLocation;   //!< Default location container */
};

/* --------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------- */

template<class Grammar>
/* Constructor */ ParserInterface<Grammar>::ParserInterface( void ) :
    m_filename( "" ), m_trace_scanning( false ), m_trace_parsing( false ),
#if BISON_VERSION <= 23
    m_defaultLocation()
#else
    m_defaultLocation( &m_filename )
#endif
{
    yy( lex_init ) ( &m_scanner );
}

template<class Grammar>
/* Destructor */ ParserInterface<Grammar>::~ParserInterface( void ) {
    yy( lex_destroy ) ( m_scanner );
}

template<class Grammar>
typename ParserInterface<Grammar>::scanner_t ParserInterface<Grammar>::begin_parse( const std::string& filename ) {
    m_filename = filename;
    scan_begin_filed();
    return m_scanner;
}

template<class Grammar>
typename ParserInterface<Grammar>::scanner_t ParserInterface<Grammar>::begin_parse( const std::string& filename, const std::string& buffer ) {
    m_filename = filename;
    scan_buffer_t lexBuffer = yy( get_current_buffer ) ( m_scanner );
    m_scannerStreamData.push_back( lexBuffer );
    const char* newString = strdup( buffer.c_str() );
    m_streamData.push_back( newString );
    scan_buffer_t newBuffer = yy( _scan_string ) ( newString, m_scanner );
    yy( _switch_to_buffer ) ( newBuffer, m_scanner );

    return m_scanner;
}

template<class Grammar>
int ParserInterface<Grammar>::end_parse( void ) {
    if( m_scannerStreamData.size() ) {
        scan_buffer_t lexBuffer = yy( get_current_buffer ) ( m_scanner );
        yy( _delete_buffer ) ( lexBuffer, m_scanner );
        const char* previousString = m_streamData.back();
        free( ( void* )previousString );
        scan_buffer_t newBuffer = m_scannerStreamData.back();
        yy( _switch_to_buffer ) ( newBuffer, m_scanner );
        m_scannerStreamData.pop_back();
    }
    else if( m_filename != "" ) {
        scan_end_filed();
    }
    return 0;
}

template<class Grammar>
typename ParserInterface<Grammar>::TokenType ParserInterface<Grammar>::scan_integer( SemanticType& lval, LocationType& lloc, const char* text ) {
    errno = 0;
    long long n = strtoll( text, NULL, 10 );
    if( errno == ERANGE ) {
        error( lloc, "integer is out of range" );
    }
    lval.ival = n;
    return Token::NUMBER;
}

template<class Grammar>
typename ParserInterface<Grammar>::TokenType ParserInterface<Grammar>::scan_identifier( SemanticType& lval, LocationType& lloc, const char* text ) {
    lval.sval = new std::string( text );
    return Token::IDENTIFIER;
}

template<class Grammar>
const char* ParserInterface<Grammar>::Exception::what( void ) const throw( )
      {
          std::stringstream s;
          s << m_location << ": " << c_str();
          return strdup( s.str().c_str() );
      }

      template<class Grammar>
      void ParserInterface<Grammar>::error( const location_t& l, const std::string& m ) {
          m_defaultLocation = l;
          throw( Exception( l, m ) );
      }

      template<class Grammar>
      void ParserInterface<Grammar>::error( const std::string& m ) {
          throw( Exception( m_defaultLocation, m ) );
      }

      template<class Grammar>
      void ParserInterface<Grammar>::scan_begin_filed( void ) {
          FILE* in = getLexFile();
          setLexDebugFlag( m_trace_scanning );
          if( m_filename.empty() || m_filename == "-" ) {
              in = stdin;
          }
          else if( !( in = fopen( m_filename.c_str(), "r" ) ) ) {
              error( "cannot open " + m_filename + ": " + strerror( errno ) );
              exit( EXIT_FAILURE );
          }
          setLexFile( in );
      }

      template<class Grammar>
      void ParserInterface<Grammar>::scan_end_filed() {
          fclose( getLexFile() );
      }

#undef yy
#undef LEXER_INTERFACE
#endif // BISONFLEXINTERFACE_H
