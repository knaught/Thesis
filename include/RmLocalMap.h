// RmLocalMap.h

#ifndef RM_LOCAL_MAP_H
#define RM_LOCAL_MAP_H

#include "RmSettings.h"
#include "RmBayesCertaintyGrid.h"

/**
 * Builds on the dynamic occupancy grid provided by RmBayesCertaintyGrid
 * by adding functionality particular to the notion of a <i>local</i> map, namely the ability 
 * to shift and rotate a map within a global frame of reference.
 * A local map is one that, with a collection of others local maps, can be used to form
 * a <i>global</i> map, and in so doing will often need to be shifted and/or rotated to correct
 * for localization errors.
 */
class RmLocalMap : public RmBayesCertaintyGrid
{
public:

	/**
	 * Initializes a local map relative to the global origin such that global range readings
	 * provided via #update() are mapped to local coordinates.
	 * @param s settings that specify whether a local map should be pre-rotated based on
	 * its initial pose
	 * @param pose identifies what the origin of this map (0,0) and "due north"
	 * on the y-axis (theta = 0) are mapped to in the <i>unscaled</i> global environment 
	 */
	RmLocalMap( RmSettings* s, RmUtility::Pose pose = RmUtility::Pose() )
		: RmBayesCertaintyGrid(s, pose.coord), m_settings(s), m_globalOrigin(pose),
		  m_cumDist(0.0), m_cumTurn(0.0) {}


	/**
	 * Adds the given sonar reading to a history for later use in reorientation, and
	 * updates the map with the given sonar reading using the sonar model defined by
	 * RmSettings::SonarModel.
	 * If enabled by RmSettings::PreRotate, the pose information in the reading is
	 * first rotated by the global theta provided during construction.
	 * @param r the unscaled coordinate and heading of the robot that is oriented to a 
	 * four quadrant Cartesian coordinate and [0.0, 360.0) zero-north (polar) theta
	 * @return the data string itemizing
	 * the affected cells, as described by RmBayesCertaintyGrid::update().
	 */
	virtual const std::string update( const RmUtility::SonarReading &r );


	/**
	 * Converts the given reading to a RmUtility::MappedSonarReading and passes on to
	 * update( const RmUtility::SonarReading & ).
	 */
	const std::string update( const RmUtility::MappedSonarReading &mr ) {
		return update( mr.reading );
	}
	// This method is provided as means for RmGlobalMap to pass on a mapped reading
	// to RmBayesCertaintyGrid so that the BCG doesn't have to recalculate the mapped reading;
	// however, RmLocalMap deals in plain RmSonarReadings for now, so it strips off
	// the mapped portion, passes that on to BCG, which must then recalculate the mapped reading.



	/**
	 * Shifts and rotates a copy of this map from its origin by the given deltas
	 * using the historical <code>SonarReading</code> data.
	 * (see #reorientBy() for more information).
	 * The global origin and historical sonar readings are not modified.
	 * @return the data string itemizing
	 * the affected cells, as described by RmBayesCertaintyGrid::update().
	 */
	const std::string reorientedBy( const RmUtility::Pose &shift ) const;


	/**
	 * Shifts and rotates this map from its origin by the given deltas
	 * using the historical RmUtility::SonarReading data.
	 * For example, a call of <code>localMap.reorientBy( Pose( Coord(5,-5), -23.5 )</code>,
	 * where <code>localMap</code> has a global origin of (10, 10), will shift the map to
	 * the left and down by 5, giving it a new global origin of (15, 5), and rotate it
	 * counter-clockwise by 23.5<sup>o</sup>.
	 * <b>This modifies the global origin as well as all the historical sonar readings.</b>
	 * @param shift unscaled amount by which local map should be shifted
	 * @return the data string itemizing
	 * the affected cells, as described by RmBayesCertaintyGrid::update().
	 */
	const std::string reorientBy( const RmUtility::Pose &shift );


	/**
	 * Returns the <i>unscaled</i> position and heading of the map, as specified during construction.
	 */
	RmUtility::Pose pose() const { return m_globalOrigin; }


	/**
	 * Returns the accumulated distance traveled over the course of the map.
	 */
	double cumDistance() const { return m_cumDist; }


	/**
	 * Returns the accumulated degrees of turn made over the course of the map
	 */
	double cumTurn() const { return m_cumTurn; }


protected:

	/**
	 * Updates the map with the given sonar reading, rotating it by the given theta relative to this
	 * map's global origin, and returns the data string itemizing the affected cells,
	 * as described by RmBayesCertaintyGrid::update().
	 *
	 * @param r the unscaled coordinate of the robot that is oriented to a 
	 * four quadrant Cartesian coordinate and a heading that is oriented to a [0.0, 360.0) 
	 * zero-north (polar) theta
	 *
	 * @param pivot the coordinate about which the robot pose is rotated theta degrees
	 *
	 * @param saveHistory indicates whether given reading should be added to an internal history;
	 * the default is <code>true</code>
	 */
	const std::string update( const RmUtility::SonarReading &r, const RmUtility::Pose &pivot, 
		bool saveHistory = true );

private:

	/** Specifies via RmSettings::PreRotate whether robot poses should be rotated relative
		to the global origin prior to processing */
	RmSettings* m_settings;

	/** What the local origin of this map is anchored to in the global map */
	RmUtility::Pose m_globalOrigin;

	/** The history of sonar readings that are received via #update(RmUtility::SonarReading),
		used for localization */
	std::vector<RmUtility::SonarReading> m_sonarReadings; 

	/** Accumulates distance traveled over the course of the map */
	double m_cumDist;

	/** Accumulates degrees of turn over the course of the map */
	double m_cumTurn;
};

#endif