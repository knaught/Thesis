// RmUtilityExt.cpp

#include <iostream>
#include "RmUtilityExt.h"


std::vector<float> RmUtility::gaussKernel( const int length, const float sigma, const float mu )
{
	const float fitFactor = 10.0f; // scales x to generate a "visually pleasing" distribution within
		// the confines of the vector; that is, one that tapers off nicely versus being cut off
		// abruptly; can be thought of as a "zoom", where a lower number zooms in, higher zooms out

	std::vector<float> kernel( length );

	const float sigmaSq2 = (sigma * sigma) * 2; // to speed up calc in loop
	const float sigmaSqrt2pi = sigma * sqrt( 2 * RmUtility::Pi ); // to speed up calc in loop
	for ( int i = 0; i < length; ++i ) 
	{
		// shift x in {0..9} to {-5..4} and smooth distribution into vector
		const float x = - ( length / 2 - i ) / (length / fitFactor);

		// precalc
		const float xMinusMu = x - mu;

		// probability density function for the normal distribution
		kernel[i] = exp( -(xMinusMu * xMinusMu) / (2 * (sigma * sigma)) ) / sigmaSqrt2pi;
	}

	return kernel;
}


RmMutableCartesianGrid<float> RmUtility::gaussGrid( const int w, const int h, const Coord origin, 
	const float theta, const float sigma, const float yBend, const float xBend )
{
	if ( yBend != 0 && xBend != 0 ) {
		throw RmExceptions::InvalidParameterException( "RmMutableCartesianGrid::gaussGrid()",
			"yBend and xBend cannot both be non-zero" );
	}

	RmMutableCartesianGrid<float> gaussG( w, h, origin );
	std::vector<float> gaussW( gaussKernel( w, sigma ) );
	std::vector<float> gaussH( gaussKernel( h, sigma ) );

	const int wMid = w / 2;
	const int hMid = h / 2;
	const float gaussMax = gaussW[wMid] * gaussH[hMid];
	const float gaussNorm = 0.9 / gaussMax;
	const float yBendFactor = yBend * h;
	const float xBendFactor = xBend * w;
	const RmUtility::BoundBox bound = gaussG.bound();

	for ( int y = bound.ul.y; y >= bound.lr.y; --y ) 
	{
		// Find index into vertical gauss vector
		const int hIndex = hMid - abs(y - origin.y);

		// Shift index into horizontal gauss vector relative to vertical distance from origin
		// As value of gaussK gets further from max(gaussK),
		// horizontal gaussG is shifted left (or right) proportionately
		const float xDelta = (gaussH[hMid] - gaussH[hIndex]) * xBendFactor;

		for ( int x = bound.ul.x; x <= bound.lr.x; ++x ) 
		{
			// Find index into horizontal gauss vector
			const int wIndex = wMid - abs(x - origin.x);

			// Shift index into vertical gauss vector relative to horizontal distance from origin
			// As value of gaussK gets further from max(gaussK),
			// vertical gaussG is shifted down (or up) proportionately
			const float yDelta = (gaussW[wMid] - gaussW[wIndex]) * yBendFactor;

			// Convolve vertical and horizontal gaussians, normalized to max
			gaussG[x - xDelta][y - yDelta] = gaussH[hIndex] * gaussW[wIndex] * gaussNorm;
		}
	}

	gaussG.rotateBy( theta );
	gaussG.trim();

	return gaussG;
}
