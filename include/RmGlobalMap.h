// RmGlobalMap.h

#ifndef GLOBAL_MAP_H
#define GLOBAL_MAP_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include "RmSonarMap.h"
#include "RmLocalMap.h"
#include "RmBayesCertaintyGrid.h"
#include "RmPolygon.h"


/**
 * Provides the mechanism for Bayesian probabilistic mapping of sonar readings into a collection of 
 * RmLocalMap objects, performing real-time localization on them, and integrating them into a single 
 * global map.
 * 
 * A RmGlobalMap may be created and utilized just like a RmLocalMap 
 * or RmBayesCertaintyGrid via its constructor and #update() method.
 * Internally, the difference between this and the RmLocalMap
 * lies in the procedure by which the RmGlobalMap maintains a collection of 
 * RmLocalMap objects.
 * Sonar readings are passed on to the current RmLocalMap::update()
 * method for processing until the distance traveled by the robot exceeds that prescribed by
 * RmSettings::LocalMapDistance.
 * Once this occurs, the current local map is archived, and a new one is installed with
 * the robot's current pose as its global origin, and processing resumes as before.
 */

class RmGlobalMap : /* is-a */ public RmBayesCertaintyGrid
{
public:

	/** The data type of the identification number that uniquely identifies each region
	    within the global map */
	typedef unsigned short RegionId;

	/**
	 * A region identifies an area of the global map that is uniquely covered by one or
	 * more local maps.  No two regions are covered by the same set of local maps.
	 */
	struct Region 
	{
		/** The identification number of the region */
		RegionId id;

		/** The polygonal boundary of the region */
		RmPolygon boundary;

		/** The local maps that cover the region */
		std::set<RmLocalMap*> maps;

		/** Creates a region with the given id and boundary, and no local map coverage */
		Region( RegionId id, const RmPolygon &p ) : id(id), boundary(p) {}
	};

	/**
	 * Initializes a 0 x 0 global map with no associated local maps and the given settings.
	 */
	RmGlobalMap( RmSettings* s );


	/**
	 * Destroys all local maps created via #update().
	 */
	~RmGlobalMap() { } // RmGlobalMap::empty(); }


	/**
	 * Passes the given sonar reading on to the current RmLocalMap, creating a new local map
	 * once the distance specified by RmSettings::LocalMapDistance has been traveled.
	 * @return the update string as returned by RmLocalMap::update()
	 */ // fulfills RmSonarMap pure virtual interface
	virtual const std::string update( const RmUtility::SonarReading &reading );


	/**
	 * Combines all local maps into a single global map, combining overlapping probabilities as
	 * an average.
	 */
	void integrate();


	/**
	 * Performs housekeeping on last open local map and integrates all maps into the global map.
	 * Further attempts to update() are ignored until until reset by call to empty().
	 */
	void finalize();


	/**
	 * Sets all cells in the global map back to their initialization value, 
	 * leaving local maps untouched.
	 */ // fulfills RmSonarMap pure virtual interface
	virtual void clear() { RmBayesCertaintyGrid::clear(); }


	/**
	 * Removes all local maps and reduces the global map back to its initialization size and state.
	 */ // fulfills RmSonarMap pure virtual interface
	virtual void empty();


	/**
	 * Returns the width of the global map as it exists after integration.
	 * Prior to integration, the global map remains in its initialialized size and state.
	 */ // fulfills RmSonarMap pure virtual interface
	virtual int width() const { return RmBayesCertaintyGrid::width(); }


	/**
	 * Returns the height of the global map as it exists after integration.
	 * Prior to integration, the global map remains in its initialialized size and state.
	 */ // fulfills RmSonarMap pure virtual interface
	virtual int height() const { return RmBayesCertaintyGrid::height(); }


	/**
	 * Returns the bound of the global map as it exists after integration.
	 * Prior to integration, the global map remains in its initialialized size and state.
	 */ // fulfills RmSonarMap pure virtual interface
	virtual RmUtility::BoundBox bound() const { return RmBayesCertaintyGrid::bound(); }


	/**
	 * Returns the value that is the average of all probabilities across the local maps
	 * mapped to the given global x-y coordinate.  Any empty cells, that is, those with the value
	 * RmBayesCertaintyGrid::InitVal, are excluded from the average.
	 * If the coordinate does not map to any local map, returns RmBayesCertaintyGrid::InitVal.
	 */ 
	float convolvedValueAt( int x, int y ) const;


	/**
	 * Inserts a text representation of this object into the given output stream.
	 * The local maps are first convolved with the global map.
	 */ // fulfills RmSonarMap pure virtual interface
	virtual std::ostream& put( std::ostream &os ) const;


	/**
	 * Returns true if there is a cell with a value of RmSettings::ObstructedCertainty or greater
	 * on the line extending between the <i>scaled</i> start and end coordinates.
	 * Both the current local map and global map are considered when making this
	 * determination.
	 */
	bool obstructionBetween( const RmUtility::Coord &start, const RmUtility::Coord &end ) const;

protected:

	/**
	 * Creates and adds a new local map to collection of maps, and dispatches any housekeeping
	 * chores.
	 * @param reading an unscaled sonar reading
	 * @return string of coordinate-probability pairs that identify all cells affected by the operation
	 */
	std::string installNewMap( const RmUtility::SonarReading &reading );


	/**
	 * Convolves the given local map with the global map.
	 * @param bound the boundary that defines the area to integrate
	 * @param retVal if false, an update string is not built and the return value is an empty string
	 * @return string of coordinate-probability pairs that identify all non-empty cells
	 * in the format <code>x y pr;x y pr;...</code>; 
	 * see RmBayesCertaintyGrid::update( const RmUtility::MappedSonarReading &reading )
	 */
	const std::string integrate( const RmPolygon &bound, bool retVal );


	/**
	 * Adds the given map to the global region map.
	 */
	void addToRegionMap( RmLocalMap *map );


	/**
	 * Removes the given map from the global region map.
	 */
	void removeFromRegionMap( RmLocalMap *map );


	/**
	 * Returns the expected robot pose given the reported pose and sonar readings, terminating
	 * local map, and global map.
	 * A probabilistic motion model, akin to that described and illustrated in
	 * <i>Sonar-Based Mapping with Mobile Robots Using EM</i> (W. Burgard, et al, 1999),
	 * is computed as a Cartesian grid with its origin at the reported robot position.
	 * The cummulative distance and degrees of turn reported by the current local map
	 * determine the dimension of this pose distribution;
	 * a greater distance increases its height, a greater degree of turn increases its width.
	 * The motion model is represented as a horizontally-elongated Gaussian ellipsoid, 
	 * with its east and west
	 * borders "pulled" down, forming a convex northern and concave southern border
	 * (see RmUtility::gaussGrid() figure B for an illustration and further details).
	 * Each cell within the motion model grid holds a normally-distributed probability.
	 *
	 * The in-range reading from each sonar is evaluated from each point in the global
	 * map that corresponds to a non-zero value in the motion model.  The range reading vector
	 * is extended from this point as if the sonar were located in that position.  
	 * If the path is unobstructed (as per the current global map),
	 * the value of that motion model cell is convolved with that cell's
	 * probability of occupancy (see RmBayesSonarModel::prOccupiedGivenSn()).
	 * This combined pose likelihood value is retained in a second probabilistic
	 * Cartesian grid that is aligned with the motion model over the global map.
	 *
	 * After all pose likelihoods have been evaluated for a sonar, a third Cartesian grid--this
	 * one histogrammic--that is aligned with the reported robot pose over the global map, 
	 * is updated.
	 * Each cell that corresponds to a cell in the pose likelihood grid with a maximum likelihood
	 * value is incremented by one.  This same histogram is updated for each sonar.
	 * Once all sonars are processed, those cells with the maximum value indicate all possible
	 * localized poses.
	 *
	 * From these selected poses, those not associated with the maximum selected pose distribution
	 * are filtered out, thus theoretically leaving only one selected localized pose.
	 * The localized pose coordinate is identified by the corresponding coordinate on the global map;
	 * the pose theta is represented as the delta between the angle, <i>alpha</i>, from the current 
	 * local map's pose coordinate to the new local map's <i>unlocalized</i> pose coordinate, and 
	 * <i>beta</i>, from the current local map's pose coordinate to the new local map's 
	 * <i>localized</i> coordinate.
	 * @param priorMap the local map that has been built but not yet convolved into the global map
	 * @param reading the first sonar reading of the new local map, unscaled
	 * @param log the file to which localization log entries are sent; if the file is invalid or
	 * closed, no log entries are recorded
	 * @return localized robot position and angle to that pos from starting position of the
	 * current local map, scaled
	 */
	RmUtility::Pose localizedPose( const RmLocalMap &priorMap, const RmUtility::SonarReading &reading, 
		std::ofstream &log ) const;


	/**
	 * Produces a log string that clears all coordinates within the given bound,
	 * for use with the map viewer application.
	 */
	std::string clearMapString( const RmUtility::BoundBox &bound ) const;


	/**
	 * Fills the area of region r over the global region map with the given id.
	 */
	void fillRegionMap( const Region *const r, const RegionId id );


	/**
	 * Creates a new region over the given bound and adds it to the global region map.
	 */
	Region* newRegion( const RmPolygon &bound );


	/**
	 * Deletes the given region and removes it from the global region map.
	 */
	void deleteRegion( Region **const r );


private:

	RmSettings* m_settings;
	std::string m_debugLogName;
	std::ofstream m_debugLog;

	/** Scaled accumulation of pose shifts */
	RmUtility::Pose m_gAccumShift;

	/** The collection of local maps which dynamically represent the global map */
	std::vector<RmLocalMap*> m_maps;

	/** Indicates whether all local maps have been convolved into RmBayesCertaintyGrid. 
		Prevents further updates. */
	bool m_finalized;

	/** The map that's being/been built but not convolved into global map */
	RmLocalMap* m_currentMap;

	/** Distance traveled in current local map */
	double m_wDistance;

	/** Global region map that identifies regions covered by one or more local maps */
	RmMutableCartesianGrid<RegionId> m_regionMap;

	/** Collection of regions that identify overlaid local maps */
	std::map<RegionId,Region*> m_regions;

	/** Identifies those region ids that were previously used and are now unused,
		enabling reuse of those ids */
	std::queue<RegionId> m_usedRegionIds;

	/** Identifies the highest assigned region id */
	RegionId m_maxRegionId;
};

#endif