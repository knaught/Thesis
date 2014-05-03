// RmBayesCertaintyGrid.h

#ifndef RM_BAYES_CERTAINTY_GRID_H
#define RM_BAYES_CERTAINTY_GRID_H

#include <string>
#include <vector>
#include <iostream>
#include "RmSonarMap.h"
#include "RmUtility.h"
#include "RmSettings.h"
#include "RmMutableCartesianGrid.h"
#include "RmBayesSonarModel.h"
#include "RmPioneerController.h"


/**
 * Provides a Cartesian-based certainty grid that utilizes a Bayesian probabilisitc sonar model.
 * Takes a RmMutableCartesianGrid, with its dynamic resizability and tie to a global origin,
 * adds the probabilistic occupancy model of RmBayesSonarModel
 * and the robot sonar interface of RmPioneerController, to provide a Bayesian occupancy grid
 * populated with probabilities of occupied given the incoming Pioneer sonar readings.
 * Three sonar models, which define what cells are updated in response to a single sonar
 * range reading, are supported:
 * single cell, acoustic axis, or sonar cone.  These are described in RmUtility::SonarModelEnum.
 * <h3>Usage</h3>
 * Besides construction, the primary interface is through update(),
 * which processes one sonar range reading for each call, and provides a textual return value that
 * may be parsed for processing by the mapping application's viewer software.
 * <h3>Dependencies</h3>
 * In addition to the RmMutableCartesianGrid, RmBayesSonarModel, and RmPioneerController,
 * this class utilizes the various utility structures of 
 * RmUtility::Coord, RmUtility::Pose, RmUtility::SonarModelEnum, 
 * RmUtility::SonarReading, and RmUtility::RadianFactor,
 * and the settings defined by RmSettings::CellSize, 
 * RmSettings::EnabledSonars, RmSettings::GridStreamPrecision, 
 * RmSettings::RegionIHalfwidth, and RmSettings::SonarModel.
 * @see RmMutableCartesianGrid
 * @see RmBayesSonarModel
 * @see RmPioneerController
 * @see RmUtility
 * @see RmSettings
 */

class RmBayesCertaintyGrid : 
	/* is-a */ public RmMutableCartesianGrid<float>, 
	/* with interface */ public RmSonarMap
{
public:

	/**
	 * Initializes the occupancy grid with default values of #InitVal, 
	 * and the sonar model used to calculate probabilities.
	 *
	 * @param s those settings used to perform mapping are RegionIHalfwidth, SonarModel,
	 * EnabledSonars, and CellSize
	 * @param origin the global origin to which the local origin of this grid is linked
	 * (see RmMutableCartesianGrid for more information)
	 */
	RmBayesCertaintyGrid( RmSettings* s, const RmUtility::Coord& origin = RmUtility::Coord() )
		: RmMutableCartesianGrid<float>(1, 1, RmUtility::Coord(), InitVal), 
		  m_settings(s), m_sonarModel(s) 
	{ 
		RmMutableCartesianGrid<float>::setOrigin( gridCoord( origin ) ); 
	}


	/**
	 * Converts the given reading to a RmUtility::MappedSonarReading and passes on to
	 * update( const RmUtility::MappedSonarReading & ).
	 */
	virtual const std::string update( const RmUtility::SonarReading& r ) {
		return update( RmPioneerController::rangeReading( r ) );
	}


	/**
	 * Updates the prior probability of occupied based on a range reading.
	 * Affected cells depend on the sonar model, as defined in RmUtility::SonarModelEnum,
	 * that is specified in the RmSettings object provided during construction. 
	 * Only readings from sonars that are enabled via RmSettings::EnabledSonars are processed.
	 * <p>
	 * The global method fireMapUpdateEvent() (defined in 
	 * <code>JniListener.cpp</code>) is called with this method's return string just before exiting 
	 * in order to enable an update of the graphical viewer application.
	 * @param reading the unscaled range reading and robot pose that is oriented to a 
	 * four quadrant Cartesian coordinate and [0.0, 360.0) zero-north (polar) theta
	 * @return an empty string if the sonar is disabled or the range reading is out of bounds;
	 * otherwise, a three line string of whitespace-delimited data (unless indicated otherwise),
	 * each line separated by a single newline, that includes
	 * <ul>
	 * <li>robot pose and sonar range data, in the format <code>x y th deviceId range</code>
	 * <li>seven coordinate pairs defining Region I and II boundaries for the pose and range data,
	 * in the format <code>x y x y...</code>
	 * <li>a series of coordinate-probability pairs for each cell affected by the reading,
	 * delimited only by semicolons, in the format <code>x y pr;x y pr;...</code>
	 * </ul>
	 */
	const std::string update( const RmUtility::MappedSonarReading& reading );


	/**
	 * Initializes all cells to #InitVal.
	 */
	virtual void clear() { RmMutableCartesianGrid<float>::clear(); }


	/**
	 * Reinitializes the grid to its initial height and width, and all cells to 
	 * #InitVal.
	 */
	virtual void empty() { RmMutableCartesianGrid<float>::empty(); }


	/**
	 * Streams a text representation of the grid in a top-down row-column format.
	 */
	virtual std::ostream& put( std::ostream& os ) const { 
		return RmMutableCartesianGrid<float>::put( os ); 
	}


	/**
	 * Returns the width of the grid.  Note this is not measured from the origin as the origin
	 * is mapped to the center of the grid.
	 */
	virtual int width() const { return RmMutableCartesianGrid<float>::width(); }


	/**
	 * Returns the height of the grid.  Note this is not measured from the origin as the origin
	 * is mapped to the center of the grid.
	 */
	virtual int height() const { return RmMutableCartesianGrid<float>::height(); }


	/**
	 * Returns the bounding box for the grid, oriented to a four-quadrant Cartesian system.
	 */
	virtual RmUtility::BoundBox bound() const { return RmMutableCartesianGrid<float>::bound(); }


	/**
	 * The value to which all cells in the grid are initialized, 0.5, which represents an even
	 * balance between the probability of occupied and the probability of empty.
	 */
	static const float InitVal;

protected:

	/**
	 * Scales a world coordinate to the grid, using the factor defined by RmSettings::CellSize.
	 * @param worldCoord an unscaled coordinate
	 */
	RmUtility::Coord gridCoord( RmUtility::Coord worldCoord ) const;


	/** 
	 * Updates the probability of the cell being occupied given the sonar reading,
	 * using RmBayesSonarModel::prOccupiedGivenSn().
	 * @param gcObject the scaled grid coordinate of the sensed object, as it lies at the
	 * intersection of the sonar's acoustic axis and the range reading
	 * @param distance the sonar's range reading
	 * @return a string containing a single coordinate-probability pair for the cell affected 
	 * by the reading, in the format <code>x y pr</code>
	 */
	std::string updateCell( const RmUtility::Coord& gcObject, double distance );


	/** 
	 * Updates the probabilities of occupied for the cells along the sonar's acoustic axis,
	 * using RmBayesSonarModel::prOccupiedGivenSn().
	 * @param gcRegionIII the scaled point that defines the innermost point of Region III 
	 * along the sonar's acoustic axis
	 * @param gcRobot the scaled point that identifies the location of the robot center on the grid
	 * @param gcObject the scaled point that identifies the location along the acoustic axis
	 * of the sensed object
	 * @return a string containing a series of coordinate-probability pairs for each cell affected 
	 * by the reading, delimited by semicolons, in the format <code>x y pr;x y pr;...</code>
	 */
	std::string updateAxis( const RmUtility::Coord& gcRobot, const RmUtility::Coord& gcObject,
		const RmUtility::Coord& gcRegionIII );

	
	/**
	 * Updates all the grid cells that fall within a particular region 
	 * with their probability of being occupied given the sonar reading,
	 * using RmBayesSonarModel::prOccupiedGivenSn().
	 * @param region the region being updated, either RmBayesSonarModel::RegionI or
	 * RmBayesSonarModel::RegionII
	 * @param boundary identifies the scaled polygon coordinates surrounding the region
	 * @param gcSonar the scaled coordinate of the originating sonar device
	 * @param thAxis the absolute angle of the sonar's acoustic axis, in degrees from [0..360)
	 * @return a string containing a series of coordinate-probability pairs for each cell affected 
	 * by the reading, delimited by semicolons, in the format <code>x y pr;x y pr;...</code>
	 * @throws an RmExceptions::InvalidParameterException on invalid region or null boundary
	 */
	std::string updateRegion( const RmBayesSonarModel::Region region, struct PointListHeader* boundary, 
		const RmUtility::Coord& gcSonar, double thAxis );


	/**
	 * Finds the region within which a particular coordinate falls.
	 * @param sonar the scaled coordinate of the originating sonar device
	 * @param object the scaled coordinate of the sensed object, as measured by the
	 * range reading along the acoustic axis of the sonar
	 * @param cell the scaled coordinate identifying the cell of interest
	 */
	RmBayesSonarModel::Region cellRegion( const RmUtility::Coord& sonar, const RmUtility::Coord& object, 
		const RmUtility::Coord& cell ) const;

private:

	/**
	 * Helper function for updateAxis() that updates the given cell according to the Bayesian sonar
	 * model.  This is different from updateCell(), which describes the sonar model, and not that it's
	 * updating an arbitrary cell.
	 */
	inline std::string RmBayesCertaintyGrid::updateAxisCell( const RmUtility::Coord &gcSonar, 
		const RmUtility::Coord &gcObject, const RmUtility::Coord &gcCell );


	RmSettings* m_settings;
	RmBayesSonarModel m_sonarModel;
};

#endif