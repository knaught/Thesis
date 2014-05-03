// RmSonarMap.cpp

#include "RmSonarMap.h"

std::ostream& operator<<( std::ostream& os, const RmSonarMap& map )
{
	return map.put( os );
}