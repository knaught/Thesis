// RmBayesSonarModel.cpp

#include <assert.h>
#include "RmBayesSonarModel.h"
#include "RmExceptions.h"

/*
 * Region I (Eqs 11.1, 11.4, 11.6):
 * - prSnGivOcc = ( ((R - r) / R) + ((BETA - alpha) / BETA) ) / 2 * maxOccupied;
 * - priorPrOccGivSn = m_prGrid[gridCoord.x][gridCoord.y]
 * - prSnGivEmp = 1 - prSnGivOcc
 * - priorPrEmpGivSn = 1 - priorPrOccGivSn
 * 
 * Region II (Eqs 11.2, 11.4, 11.6):
 * - prSnGivOcc = ( 1 - [( ((R - r) / R) + ((BETA - alpha) / BETA) ) / 2] ) * maxEmpty
 * - priorPrOccGivSn = m_prGrid[gridCoord.x][gridCoord.y]
 * - prSnGivEmp = 1 - prSnGivOcc
 * - priorPrEmpGivSn = 1 - priorPrOccGivSn
 * 
 * Where:
 * - prSnGivOcc = P(Occupied) in Eq 11.1/2, P(s|H) in Eq 11.3, and P(s|Occupied) in Eq 11.4
 * - priorPrOccGivSn = P(H) in Eq 11.3, P(Occupied) in Eq 11.4, and P(H|s_{n-1}) in Eq 11.6
 * - prSnGivEmp = P(Empty) in Eq 11.1/2, P(s|~H) in Eq 11.3, and P(s|Empty) in Eq 11.4
 * - priorPrEmpGivSn = P(~H) in Eq 11.3, P(Empty) in Eq 11.4, and P(~H|s_{n=1}) in Eq 11.6
 * - maxOccupied = 0.98
 * - maxEmpty = 0.50 (or 1.0, historically)
 * - resulting cell value = 
 *   prSnGivOcc * priorPrOccGivSn / ( prSnGivOcc * priorPrOccGivSn + prSnGivEmp * priorPrEmpGivSn )
 */


float RmBayesSonarModel::prOccupiedGivenSn( 
	float priorPrOccGivSn, Region region, double r, double alpha ) const
{
	char buff[30];
	if ( region != RegionI && region != RegionII ) {
		sprintf( buff, "Region specification must be %d or %d; is %d", RegionI, RegionII, region );
		throw RmExceptions::InvalidParameterException( 
			"RmBayesSonarModel::prOccupiedGivenSn()", buff );
	}
	if ( r < 0 || r > R ) {
		assert( false );
		sprintf( buff, "Range reading of %.0f is outside bounds of [0..%.0f]", r, R ); 
		throw RmExceptions::InvalidParameterException( 
			"RmBayesSonarModel::prOccupiedGivenSn()", buff );
	}
	if ( alpha < 0 || alpha > m_settings->Beta ) {
		sprintf( buff, "Alpha of %.2f is outside bounds of [0..%.2f]", alpha, m_settings->Beta );
		throw RmExceptions::InvalidParameterException( 
			"RmBayesSonarModel::prOccupiedGivenSn()", buff );
	}

	// It is possible that a cell will have obtained a probability of occupancy of 1.0
	// and that the cell will later be on the outer boundary of Region I, such that
	// alpha = BETA and r = R.  In this event, the below formula will become
	//      0.0 * 1.0 / ((0.0 * 1.0) + (1.0 * 0.0)) = 0 / 0 = Undefined
	// In this event, it doesn't seem reasonable that the probability should fall from
	// 1.0 to 0.0 in one iteration.  Therefore bring in r a bit, which actually leaves
	// the resulting pr unchanged, at 1.0, for this example.
	if ( r == R && alpha == m_settings->Beta ) r -= 0.0001f;

	float prSnGivOcc = prSnGivenOccupied( r, alpha, region ); // P(s_n|H)
	float prSnGivEmp = 1 - prSnGivOcc; // P(s_n|~H)
	float priorPrEmpGivSn = 1 - priorPrOccGivSn; // P(~H|s_{n-1})
	
	float pr = (prSnGivOcc * priorPrOccGivSn) / 
		((prSnGivOcc * priorPrOccGivSn) + (prSnGivEmp * priorPrEmpGivSn));

	return pr;
}


float RmBayesSonarModel::prSnGivenOccupied( double r, double alpha, Region region ) const
{
	// P(s_n|H) for
	//		Region I = [(R - r) / R + (Beta - alpha) / Beta] / 2 * Max_{occupied}
	//		Region II = 1 - P(s_n|~H)

	// NOTE: this formula doesn't account for the cell's distance from the sonar reading,
	// so that probabilities increase as r approaches 0, regardless of how far from the
	// sonar the cell is.  This doesn't agree with the pictured sonar model,
	// where the range reading is shown at the peak of a 3-d gaussian hill over Region I.

	if ( r < 0 || r > R ) 
		throw RmExceptions::InvalidParameterException( "prSnGivenOccupied( r )" );
	if ( alpha < 0 || alpha > m_settings->Beta ) 
		throw RmExceptions::InvalidParameterException( "prSnGivenOccupied( alpha )" );
	if ( region != RegionI && region != RegionII ) 
		throw RmExceptions::InvalidParameterException( "prSnGivenOccupied( region )" );

	float pr;

	switch ( region )
	{
		case RegionI:
			// In the original equation, alpha is not multiplied by any factor
			// Doing so here will often produce a negative result, so need to adjust for it
			pr = ( ((R - r) / R) + 
				((m_settings->Beta - (alpha * m_settings->AlphaFactor)) / m_settings->Beta) ) / 2 
				* m_settings->MaxOccupied;
			if ( pr <= 0 ) pr = 0.0001f;
			break;

		case RegionII:
			pr = ( 1 - prSnGivenEmpty( r, alpha, region ) ) * m_settings->MaxEmpty;
			break;
	}

	return pr;
}


float RmBayesSonarModel::prSnGivenEmpty( double r, double alpha, Region region ) const
{
	// P(s_n|~H) for 
	//   Region I = 1 - P(s_n|H)
	//   Region II = [(R - r) / R + (Beta - alpha) / Beta] / 2

	if ( r < 0 || r > R ) 
		throw RmExceptions::InvalidParameterException( "prSnGivenEmpty( r )" );
	if ( alpha < 0 || alpha > m_settings->Beta ) 
		throw RmExceptions::InvalidParameterException( "prSnGivenEmpty( alpha )" );
	if ( region != RegionI && region != RegionII )
		throw RmExceptions::InvalidParameterException( "prSnGivenEmpty( region )" );

	float pr;

	switch ( region )
	{
		case RegionI:
			pr = 1 - prSnGivenOccupied( r, alpha, region );
			break;

		case RegionII:
			// In the original equation, alpha is not multiplied by any factor
			// Doing so here will often result in a negative result, so need to adjust for it
			pr = ( ((R - r) / R) + 
				((m_settings->Beta - (alpha * m_settings->AlphaFactor)) / m_settings->Beta) ) / 2;
			if ( pr <= 0 ) pr = 0.0001f;				
			break;
	}

	return pr;
}
