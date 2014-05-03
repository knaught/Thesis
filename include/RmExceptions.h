// RmExceptions.h

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <iostream>

/**
 * Provides a collection of standardized exception objects that are thrown throughout the
 * application.
 */
namespace RmExceptions {

///////////////////////////////
// Exception                 //
///////////////////////////////

/**
 * Provides a base exception class that accepts an exception name, location, and message.
 */
struct Exception
{
	/** The name of the exception, generally the same as the struct or class name */
	const char* name;

	/** The location that the exception occurred, typically a fully qualified 
		<code>class::method()</code> name, optionally with parameters */
	const char* location;

	/** A message customized to the specific exception event, generally used to provided details
		on invalid parameters or unexpected values */
	const char* message;
	
	/** Constructs an Exception identified by name, code location, and error message */
	Exception( const char* name_ = NULL, const char* location_ = NULL, const char* message_ = NULL )
		: name( name_ ), location( location_ ), message( message_ ) {}

	/** Sends a text representation of the given Exception to the given stream */
	friend std::ostream& operator<<( std::ostream& os, const Exception& e );
};



///////////////////////////////
// InvalidDimensionException //
///////////////////////////////


/**
 * Indicates a negative or otherwise disallowed value was provided when attempting to size or resize
 * an array.
 */
struct InvalidDimensionException : public Exception 
{
	/** Constructs an InvalidDimensionException identified by code location, and error message */
	InvalidDimensionException( const char* location_ = NULL, const char* message_ = NULL ) 
		: Exception( "InvalidDimensionException", location_, message_ ) {}
};




///////////////////////////////
// IndexOutOfBoundsException //
///////////////////////////////


/**
 * Indicates an invalid index was provided when attempting to access an array.
 */
struct IndexOutOfBoundsException : public Exception 
{
	/** Constructs an IndexOutOfBoundsException identified by code location, and error message */
	IndexOutOfBoundsException( const char* location_ = NULL, const char* message_ = NULL ) 
		: Exception( "IndexOutOfBoundsException", location_, message_ ) {}
};




///////////////////////////////
// InvalidParameterException //
///////////////////////////////


/**
 * Indicates an invalid parameter was provided.
 */
struct InvalidParameterException : public Exception 
{
	/** Constructs an InvalidParameterException identified by code location, and error message */
	InvalidParameterException( const char* location_ = NULL, const char* message_ = NULL ) 
		: Exception( "InvalidParameterException", location_, message_ ) {}
};




///////////////////////////////
// InvalidStateException     //
///////////////////////////////


/**
 * Indicates an invalid parameter was provided.
 */
struct InvalidStateException : public Exception 
{
	/** Constructs an InvalidStateException identified by code location, and error message */
	InvalidStateException( const char* location_ = NULL, const char* message_ = NULL ) 
		: Exception( "InvalidStateException", location_, message_ ) {}
};




///////////////////////////////
// IOException               //
///////////////////////////////


/**
 * Indicates an input/output error occurred when attempting a file operation.
 */
struct IOException : public Exception 
{
	/** Constructs an IOException identified by code location, and error message */
	IOException( const char* location_ = NULL, const char* message_ = NULL ) 
		: Exception( "IOException", location_, message_ ) {}
};



///////////////////////////////
// SocketException           //
///////////////////////////////


/**
 * Indicates an input/output error occurred when attempting a file operation.
 */
struct SocketException : public Exception 
{
	/** Constructs an SocketException identified by code location, and error message */
	SocketException( const char* location_ = NULL, const char* message_ = NULL ) 
		: Exception( "SocketException", location_, message_ ) {}
};

};

#endif