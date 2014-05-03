// RmExceptions.cpp

#include "RmExceptions.h"

// Note: Doxygen doesn't like the RmExceptions:: qualifier, but the linker requires it
std::ostream& RmExceptions::operator<<( std::ostream& os, const RmExceptions::Exception& e )
{
	os	<< (e.name == NULL ? "Exception" : e.name)
		<< " thrown in "
		<< (e.location == NULL ? "unspecified location" : e.location);
	if ( e.message != NULL ) os << "\n" << e.message << "\n";

	return os;
}