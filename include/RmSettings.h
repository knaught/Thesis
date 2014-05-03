// RmSettings.h

#ifndef RM_SETTINGS_H
#define RM_SETTINGS_H

#include <string>
#include "RmUtility.h"

/**
 * Aggregates all configurable settings within the application.
 * Initialization is first attempted from the file "settings.dat".
 * If that file doesn't exist, or one or more settings are invalid,
 * default values are assigned.
 */

struct RmSettings 
{
	//////
	// Sonar stuff (RmBayesCertaintyGrid)

	/** Half the distance of Region I, in millimeters 
		@see RmBayesCertaintyGrid */
	int RegionIHalfwidth;

	/** Array indicating for which sonars range readings should be processed
		@see RmBayesCertaintyGrid::update()() */
	bool* EnabledSonars;

	/** Which sonar model will be used to populate the occupancy grid with probabilities
		@see RmBayesCertaintyGrid::update()() */
	RmUtility::SonarModelEnum SonarModel;


	//////
	// For calculating probabilities (RmBayesSonarModel)
	// From forumulas used in Murphy

	/** Murphy's <i>beta</i> parameter defining the size of the half-width of the sonar cone; 
		typically 15<sup>o</sup> 
		@see RmBayesSonarModel */
	int Beta;

	/** Multiplier on Murphy's <i>alpha</i>, the angle of a cell off a sonar's acoustic axis.
		This isn't defined by Murphy, but was added with the intention
		of calibrating the resulting map.  Experience showed this should be left at 1.0. 
		@see RmBayesSonarModel */
	float AlphaFactor;

	/** A Murphy parameter specifying a cap on the probability that a cell is occupied; 
		typically 0.98 degrees 
		@see RmBayesSonarModel */
	float MaxOccupied;

	/** Specifies the cap on the probability of occupied for cells that fall within region II.
	    This isn't defined by Murphy, but has been seen in other literature, and has the
		effect of ensuring no probabilities in region II can be greater than this value,
		which would typically be 0.5. 
		@see RmBayesSonarModel */
	float MaxEmpty;

	/** For out of range sonar readings, the reading will be converted to
		RmPioneerController::SonarRange divided by this value */
	int OutOfRangeConversion;

	/** TODO: Indicates whether out of range sonar readings are ignored or processed for the
	    cone sonar model. */
	bool IgnoreOutOfRange;

	/** TODO: Indicates whether to ignore range readings obstructed by previous readings 
	    that have been convolved with the global map. */
	bool IgnoreObstructed;


	//////
	// Map size and scaling

	/** Maximum distance the robot may travel within a local map 
		@see RmGlobalMap */
	int LocalMapDistance;

	/** Number of millimeters per grid cell
		@see RmBayesCertaintyGrid
		@see RmSonarActionHandler */
	int CellSize;

	/** Maximum millimeters the robot may travel within one sonar collection cycle
		(for use by RmSonarMapper) */
	int MaxCollectionDistance;

	/** Maximum degrees of turn the robot may make within one sonar collection cycle 
		(for use by RmSonarMapper) */
	int MaxCollectionDegrees;

	/** Whether local maps are pre-rotated by the pose theta given during construction 
		@see RmLocalMap::update() */
	bool PreRotate;


	//////
	// Localization

	/** Whether global map is localized during construction. */
	bool Localize;

	/** 
	 * Parameters for configuring the motion model used for localization. 
	 * @see RmGlobalMap::localizedPose()
	 */
	struct MotionModelStruct 
	{
		/** 
		 * The model's minimum height (in the scale of the map)
		 * @see UnitDistance. 
		 */
		int MinHeight;

		/** 
		 * The model's minimum width (in the scale of the map)
		 * @see UnitTurn. 
		 */
		int MinWidth;

		/** The model's height increases in relation to distance traveled within a local map,
		  * as scaled by this parameter. The value corresponds to the number of millimeters
		  * of travel per model height increment.  Valid values fall within [1..inf];
		  * a smaller value makes the model taller. */
		int UnitDistance;

		/** The model's width increases in relation to degrees of turn made within a local map,
		  * as scaled by this parameter.  The value corresponds to the number of degrees
		  * of turn per model width increment.  Valid values fall within [1..inf]; a smaller
		  * value makes the model wider. */
		int UnitTurn;

		/** 
		 * The Gaussian scale parameter.  A smaller value narrows the distribution, 
		 * making the ascent sharper. 
		 * @see RmUtility::gaussKernel() 
		 */
		float GaussianSigma;

		/** 
		 * The amount by which the motion model is bent.
		 * Valid values fall within [0..inf]; a larger value produces greater bend. 
		 * @see RmUtility::gaussGrid()
		 */
		float BendFactor;
	} MotionModel;

	/** Minimum prior probability of occupied that identifies occupied cell for purposes
	  * of testing for path obstruction. */
	float ObstructedCertainty;


	//////
	// Log files

	/** Filename (without extension) the occupancy grid is saved to */
	std::string GridName;

	/** Filename (without extension) sonar range data is read from or written to */
	std::string SonarName;


	//////
	// Settings stuff

	/** Filename (without extension) of the settings file to and from which settings are
		written and read */
	std::string SettingsName;

	/** 
	 * Attempts initialization via settings file or default values.
	 * If settings file doesn't exist or contains invalid values, default values are used.
	 */
	RmSettings();

	/** Destroys memory allocated for #EnabledSonars. */
	~RmSettings();

	/**
	 * Initializes all settings to their default values.
	 */
	void init();

	/** 
	 * Reads the settings from the file specified by SettingsName.
	 * @return true if settings file read
	 */
	bool read();

	/** Writes the settings to the file specified by SettingsName */
	void write();

	/** 
	 * Returns a space-delimited string of all invalid settings. 
	 * A zero-length string indicates no invalid values.
	 */
	std::string invalids();

	/** Sends a text representation of all settings to the given stream */
	std::ostream& put( std::ostream& os, std::string prefix = "" ) const;

	/** Sends a text representation of all settings in s to the given stream */
	friend std::ostream& operator<<( std::ostream& os, const RmSettings &s );
};

#endif