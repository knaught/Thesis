// RmActionHook.h

#ifndef RM_ACTION_HOOK_H
#define RM_ACTION_HOOK_H

#include "Aria.h"
#include "RmActionHandler.h"

/**
 * Extends Aria's ArAction so as to provide a means for hooking into the Pioneer robot 
 * event call chain.
 */
class RmActionHook : public ArAction
{
public:

	/**
	 * Sets up the given RmActionHandler as the handler for
	 * this particular hook into the robot event chain.
	 * It will be called each time an Aria action event is fired.
	 */
	RmActionHook( RmActionHandler* actionHandler );


	/**
	 * The ArAction method that is called by the robot event chain
	 * to execute this particular action.
	 * Dispatches the event to the RmActionHandler registered with this hook.
	 */
	virtual ArActionDesired *fire( ArActionDesired currentDesired );


	/**
	 * Allows the object to capture the robot for which events
	 * are being received and the sonar device for that robot.
	 */
	virtual void setRobot( ArRobot *robot );

private:

	/** The robot for which events are being generated */
	ArRobot* m_robot;

	/** The value returned by #fire() */
	ArActionDesired m_desired;

	/** The handler to forward an action event on to */
	RmActionHandler* m_actionHandler;
};

#endif
