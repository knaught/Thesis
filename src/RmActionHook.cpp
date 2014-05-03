// RmActionHook.cpp

#include <iostream>
#include "RmActionHook.h"

RmActionHook::RmActionHook( RmActionHandler* actionHandler )
	: ArAction("Hook")
{
	m_actionHandler = actionHandler;
}


void RmActionHook::setRobot( ArRobot *robot )
{
	m_robot = robot;
}


ArActionDesired *RmActionHook::fire( ArActionDesired currentDesired )
{
	if ( m_actionHandler ) m_actionHandler->handleAction( m_robot );

	return &m_desired;
}
