// RmBayesSonarModel.h

#ifndef RM_BAYES_SONAR_MODEL_H
#define RM_BAYES_SONAR_MODEL_H

#include "RmSettings.h"
#include "RmPioneerController.h"

/**
 * Provides Bayesian probabilistic model for interpreting sonar readings. The model is based on the 
 * formulas presented in <b>Murphy, Robin R. <i>An Introduction to AI Robotics</i>.  MIT Press, 2000.</b>
 * The specific equations used from Murphy are 11.1, 11.4, and 11.6:
 * <p>
 * <table align=center cellpadding=5 border=0>
 *
 * <tr>
 * <td>(11.1)</td>
 * <td>P(s|Occupied)</td><td>=</td>
 * <td>[ ((R - r) / R) + ((<i>beta</i> - <i>alpha</i>) / <i>beta</i>) ] / 2 * Max<sub>occupied</sub></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>P(s|Empty)</td><td>=</td>
 * <td>1.0 - P(s|Occupied)</td>
 * </tr>
 * <tr>
 * <td>(11.2)</td>
 * <td>P(s|Occupied)</td><td>=</td>
 * <td>1.0 - P(s|Empty)</td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>P(s|Empty)</td><td>=</td>
 * <td>( ((R - r) / R) + ((<i>beta</i> - <i>alpha</i>) / <i>beta</i>) ) / 2 * Max<sub>empty</sub></td>
 * </tr>
 * <tr>
 * <td>(11.6)</td>
 * <td>P(Occupied|s<sub>n</sub>)</td><td>=</td>
 * <td>( P(s<sub>n</sub>|Occupied)  P(Occupied|s<sub>n-1</sub>) ) / 
 * ( P(s<sub>n</sub>|Occupied) P(Occupied|s<sub>n-1</sub>)
 * + P(s<sub>n</sub>|Empty) P(Empty|s<sub>n-1</sub>) )</td>
 * </tr>
 * </table>
 * <p>
 * where
 * <ul>
 * <li>s stands for the sonar range reading
 * <li>R is the maximum range of the sonar
 * <li>r is the range reading of the sonar
 * <li><i>beta</i> is the half-width of the sonar cone
 * <li><i>alpha</i> is the angular distance off the sonar acoustic axis that a cell of interest resides
 * <li>Max<sub>occupied</sub> is a cap on the maximum probability that a cell is occupied,
 * defined by Murphy to be 0.98, and defined within RmSettings::MaxOccupied
 * <li>Max<sub>empty</sub> is a cap on the maximum probability that a cell is occupied within
 * region II, where all cells are implicitly empty; this is not a Murphy-defined parameter,
 * but one that is deemed necessary to prevent probabilities in those cells from showing any
 * degree of occupied; defined in RmSettings::MaxEmpty
 * </ul>
 * <p>
 * Equation (11.1) captures the idea that the closer the grid element is to the acoustic axis,
 * the higher the belief.  Likewise, the nearer the grid element is to the origin of the sonar beam,
 * the higher the belief (the (R-r) / r term).  The Max<sub>occupied</sub> term expresses the
 * assumption that a reading of occupied is never fully believable.  A Max<sub>occupied</sub>
 * of 0.98 means that a grid element can never have a probability of being occupied greater than
 * 0.98.
 * <p>
 * Equation (11.6) generates the probability of interest, that the area at a certain cell is
 * really occupied given a particular sensor reading.
 * <p>
 * The probability values stored in the grid cells are those calculated by (11.6),
 * which itself utilizes (11.1) and (11.2), depending upon whether the cell of interest is
 * in Region I or II, respectively.  These formulas are implemented by #prOccupiedGivenSn(),
 * #prSnGivenOccupied(), and #prSnGivenEmpty().
 */

class RmBayesSonarModel
{
public:

	/** 
	 * Identifies the region within which a particular grid cell falls, in relation to the
	 * sonar range reading. 
	 */
	enum Region 
	{ 
		/** Identifies the cone region that lies between the sonar device and Region I */
		RegionII, 

		/** Identifies the region that extends the cone of Region II and surrounds the range reading
			by RmSettings::RegionIHalfwidth on either side */
		RegionI, 

		/** Identifies the region that extends the cone of Region I, including those cells that
			lie beyond Region I but remain within range of the sonar device */
		RegionIII, 

		/** Identifies those cells that are out of range of the sonar device */
		OutOfRange 
	};

	/** 
	 * Initializes the calculated configurable parameters that are used in the probability 
	 * calculations.  
	 * @param s defines various parameters used in calculating probabilities (see class description)
	 */
	RmBayesSonarModel( RmSettings* s ) 
		: m_settings(s), R(RmPioneerController::SonarRange / s->CellSize) {}

	/**
	 * Returns the probability that a cell is occupied given
	 * a particular sonar range reading. Calculated using equation (11.6) detailed in
	 * the class description.
	 * @param priorPrOccGivSn the prior probability that a cell is occupied given the
	 * previous sonar reading; this is the value currently stored in the cell of interest
	 * @param region the sonar model region within which the cell falls
	 * @param r the <i>scaled</i> distance from the sonar origin to the cell (not sensed object)
	 * @param alpha the angle to the cell object relative to the sonar's acoustic axis
	 * @throws an RmExceptions::InvalidParameterException on receiving an invalid region 
	 * specification, a range reading less than 0 or greater than #R, 
	 * or an alpha less than 0 or greater than RmSettings::Beta
	 */
	float prOccupiedGivenSn( 
		float priorPrOccGivSn, Region region, double r, double alpha = 0.0 ) const;

	/** A Murphy parameter defining the <i>scaled</i> maximum range of the sonar */
	double R;

protected:

	/**
	 * Returns the probability that the sensor would return a range reading
	 * given the corresponding location was actually occupied.
	 * In other words, if <code>r</code> were the range reading and we're looking at
	 * distance <code>alpha</code> off the acoustic axis, 
	 * what is the probability that there really is an object at that location.
	 * The closer the reading is to the sonar device and its acoustic axis
	 * (i.e., the smaller the <code>r</code> and <code>alpha</code>),
	 * the greater the probability of occupancy.
	 * Note that this is calculated without knowledge of specific cell coordinates, as the only values 
	 * considered in calculating the probability are the cell's nearness to the sonar (represented by
	 * <code>r</code>) and its distance off the acoustic axis (represented by <code>alpha</code>).
	 * For Region I, P(s|Occupied) of equation (11.1) is used; for Region II, 
	 * P(s|Occupied) of equation (11.2) is used.
	 * @param r the distance from the sonar origin to the cell of interest in the scale of the grid
	 * @param alpha the angle to the cell of interest relative to the sonar acoustic axis
	 * @param region the Region within which the probability should be calculated
	 * @throws an RmExceptions::InvalidParameterException on receiving an invalid region 
	 * specification, a range reading less than 0 or greater than #R, 
	 * or an alpha less than 0 or greater than RmSettings::Beta
	 */
	float prSnGivenOccupied( double r, double alpha, Region region ) const;


	/**
	 * Returns the probability that the sensor would return a range reading
	 * given the corresponding location was actually empty.
	 * In other words, if <code>r</code> were the range reading and we're looking at
	 * distance <code>alpha</code> off the acoustic axis, 
	 * what is the probability that the space at that location really is empty.
	 * The closer the reading is to the sonar device and its acoustic axis,
	 * i.e., the smaller the <code>r</code> and <code>alpha</code>,
	 * the greater the probability of vacancy.
	 * Note that this is calculated without knowledge of specific cell coordinates, as the only values 
	 * considered in calculating the probability are the cell's nearness to the sonar (represented by
	 * <code>r</code>) and its distance off the acoustic axis (represented by <code>alpha</code>).
	 * For Region I, P(s|Empty) of equation (11.1) is used; for Region II, 
	 * P(s|Empty) of equation (11.2) is used.
	 * @param r the distance from the sonar origin to the cell of interest in the scale of the grid
	 * @param alpha the angle to the cell of interest relative to the sonar acoustic axis
	 * @param region the Region within which the probability should be calculated
	 * @throws an RmExceptions::InvalidParameterException on receiving an invalid region 
	 * specification, a range reading less than 0 or greater than #R, 
	 * or an alpha less than 0 or greater than RmSettings::Beta
	 */
	float prSnGivenEmpty( double r, double alpha, Region region ) const;

private:

	const RmSettings* m_settings;
};

#endif