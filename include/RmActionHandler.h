// ActionHandler.h

#ifndef RM_ACTION_HANDLER_H
#define RM_ACTION_HANDLER_H

#include "Aria.h"

/**
 * Provides an interface for any class that wants to act as an action listener
 * within the Pioneer robot Aria event call chain.
 * <h3>Usage</h3>
 * See RmActionHook for an example of how to utilize the handler.
 * <h3>Dependencies</h3>
 * None.
 */
class RmActionHandler
{
public:

	/** 
	 * Override to take some action as part of an RmActionHook event that is
	 * installed in the Aria action event call chain. 
	 */
	virtual void handleAction( ArRobot* robot ) = 0;
};

#endif