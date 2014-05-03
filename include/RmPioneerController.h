// RmPioneerController.h

#ifndef RM_PIONEER_CONTROLLER_H
#define RM_PIONEER_CONTROLLER_H

#include "Aria.h"
#include "RmActionHook.h"
#include "RmUtility.h"

/**
 * Provides a wrapper around and interface to a ActivMedia Robotics Pioneer P2AT mobile robot 
 * and its accompanying Aria robot control API.
 * Key functionality lies in its single-step initialization of a Pioneer robot or its associated
 * simulator that operates in manual key-driven or autonomous wander mode.
 * This class also provides the means for converting raw robot pose and range reading data into
 * measurements that conform to a standard four-quadrant Cartesian coordinate system and polar 
 * [0..360) degree orientation, and for mapping the location of sonars and their readings.
 * <h3>Usage</h3>
 * Standardized robot pose and sonar range reading coordinates are obtained through
 * #pose() and #rangeReading().
 * Other useful parameters include:
 * <ul>
 * <li>distances (#DistToSonar) and angles (#ThetaToSonar) from the center of the 
 * robot to each of its sonars
 * <li>the heading of each sonar relative to the robot's front (#SonarTheta)
 * <li>the number of sonars (#NumSonars) and their maximum range (#SonarRange)
 * </ul>
 * <h3>Dependencies</h3>
 * This class requires the Aria-defined <code>ArPose</code> and
 * utilizes the various utility structures of 
 * RmUtility::Coord, RmUtility::MappedSonarReading, RmUtility::Pose, and
 * RmUtility::SonarReading.
 */

class RmPioneerController
{
public:

	/** Identifies the status of the robot connection attempt */
	enum ConnectionStatus { FAILED, SUCCESS, SERIAL, TCP };


	/** Identifies whether the robot wanders on its own or is controlled by the keyboard */
	enum DriveMode { WANDER, KEYDRIVE };


	/**
	 * Sets the robot up for either manual keyboard control or autonomous wander mode,
	 * installs the given ArAction-derived object in the robot's event call chain,
	 * and runs the robot in either synchronous or asynchronous mode.
	 * The robot controller is designed to be run through a console window that accepts keyboard
	 * control input.  The up and down cursor keys control forward and reverse
	 * motion, the left and right control direction; the <code>Escape</code> key terminates the run.
	 * If in wander mode, sonars are automatically engaged.
	 * Run asynchronously when processing external to the action event chain needs to occur.
	 * @param wander specifies whether the the robot will wander autonomously
	 * @param asynch specifies whether the robot is run in asynchronous mode
	 * @param actionHandlers those handlers that will be called as part of the robot event call chain
	 * @param priority a value between 0 and 100, passed to ArRobot::addAction()
	 */
	RmPioneerController( bool wander = false, bool asynch = false, 
		std::vector<RmActionHandler*> *actionHandlers = NULL, int priority = 50 );


	/**
	 * Triggers proper RmPioneerController shut down procedures.
	 */
	~RmPioneerController();


	/**
	 * Returns true if the robot is currently running.
	 */
	bool isRunning() { return m_robot.isConnected(); }


	/**
	 * Returns the current pose of the robot that is mapped to a four-quadrant 
	 * Cartesian system with polar zero-north degrees measured from [0..360).
	 * The initial pose of the robot is (0, 0, 0).
	 */
	RmUtility::Pose pose();


	/**
	 * Translates the given raw Aria-reported robot pose to one that is mapped to a four-quadrant 
	 * Cartesian system with polar zero-north degrees measured from [0..360).
	 */
	static RmUtility::Pose pose( ArPose& arPose );


	/**
	 * Determines whether the robot operates in wander or keydrive mode.
	 * @see DriveMode
	 */
	void setDriveMode( DriveMode wander );

	
	/**
	 * Returns the ArRobot that was created during construction
	 * and through which sonar data is retrieved.
	 */
	ArRobot* arRobot() { return &m_robot; }


	/**
	 * Returns the keydrive action that is being used to drive the robot.
	 */
	ArActionKeydrive* arKeydriveAction() { return &m_keydriveAction; }

	
	/**
	 * Returns the status of the ArRobot connection attempt.
	 */
	ConnectionStatus getConnectionStatus() const { return m_connectionStatus; }


	/**
	 * Given a robot pose that has been pre-processed by #pose(), 
	 * locates the sonar device and range reading in the same Cartesian system.
	 */
	static RmUtility::MappedSonarReading rangeReading( const RmUtility::SonarReading& reading );


	/** Distance from center of robot to sonar device */
	static const double DistToSonar[];

	/** Angle from center of robot to sonar device */
	static const double ThetaToSonar[];

	/** Angle of sonar device relative to robot's heading */
	static const int SonarTheta[];

	/** The maximum range of the sonars */
	static const int SonarRange;

	/** The number of sonars on the robot */
	static const int NumSonars;

protected:

	/**
	 * Initializes the robot, setting up the TCP or serial connection,
	 * key handler, key drive action, and this RmPioneerController as an ArAction
	 * on the robot event chain.
	 * @return the result of the connection attempt
	 */
	ConnectionStatus initRobot( bool wander, 
		std::vector<RmActionHandler*> *actionHandlers, int priority );

	
	/**
	 * Converts Aria-reported heading value into a zero-north [0..360) orientation.
	 */
	static double polarTheta( double arTheta );

private:

	ConnectionStatus m_connectionStatus; // the status of the robot connection
	bool m_wander; // true enables autonomous wander

	// Standard Aria robot objects
	ArRobot m_robot; // the robot through which pose and sonar data will come

	// Wander mode objects
	ArActionStallRecover m_recover;
	ArActionBumpers m_bumpers;
	ArActionAvoidFront m_avoidFront;
	ArActionAvoidSide m_avoidSide;
	ArActionConstantVelocity m_constantVelocity; //( "Constant Velocity", 400 );

	ArActionLimiterForwards m_forwardLimiter;

	// Keydrive mode objects
	ArActionKeydrive m_keydriveAction; // the action that enables cursor control of the robot

	ArKeyHandler m_keyHandler; // the handler that listens for the Escape key
	ArSonarDevice m_sonar; // the range device associated with the robot
	ArSimpleConnector *m_connector;

	RmActionHook *m_actionHook; // the generic event hook that calls back to handleAction()
		// NOTE!!! RmActionHook must appear after ArRobot m_robot declaration
		// Otherwise, program will crash upon exit
};

#endif