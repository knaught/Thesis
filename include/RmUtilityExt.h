// RmUtilityExt.h
// Provides a functional extension to RmUtility.

#ifndef RM_UTILITY_EXT_H
#define RM_UTILITY_EXT_H

#include <vector>
#include "RmMutableCartesianGrid.h"


namespace RmUtility {

	/**
	 * Returns a normally-distributed probability density vector of the given length.
	 * The general formula for the probability density function of the normal distribution is
	 * f(x) = [ e<sup>-(x-mu)<sup>2</sup> / (2 sigma<sup>2</sup>)</sup> ] / 
	 * ( sigma * (2pi)<sup>0.5</sup> )
	 * @param length the size of the distribution
	 * @param sigma the <i>scale parameter</i>, for which a higher value stretches out the distribution
	 * (values in the periphery are higher; the descent is slower); sigma is 1 for a standard
	 * normal distribution
	 * @param mu the <i>location parameter</i>, which shifts the distribution peak;
	 * values on the extent toward which the shift is made are truncated; mu is 0.0 for a standard
	 * normal distribution
	 */
	std::vector<float> gaussKernel( 
		const int length, const float sigma = 1.0f, const float mu = 0.0f );

	
	/**
	 * Generates an ellipsoid probability density matrix with the specified dimensions and
	 * orientation.  The ellipse may be "bent outward" on any of its four sides using the bend
	 * factors, where a positive value affects the bottom and left sides, a negative the top
	 * and left.  Only one of these factors may have a non-zero value.
	 * Note that bending the ellipse enlarges it, so its final dimensions will be greater than
	 * those requested; its center will remain unchanged.
	 * Consider the following three calls to gaussGrid():
	 * <pre>
	 *     RmMutableCartesianGrid<float> A( gaussGrid( 18, 5 ) );
	 *     RmMutableCartesianGrid<float> B( gaussGrid( 18, 5, 0, 1.75 ) );
	 *     RmMutableCartesianGrid<float> C( gaussGrid( 18, 5, 0, 0, -1.75 ) );
	 *
	 *     .....xxxxxx.....    .....xxxxxx.....    .....xxxxxxxxxxxxxx
	 *     .xxxxxxxxxxxxx..    ...xxxxxxxxxx...    .xxxxxxxxxxxxxxxx..
	 *     xxxxxxxOxxxxxxxx    ..xxxxxOxxxxxx..    xxxxxxxOxxxxxxxx...
	 *     .xxxxxxxxxxxxxx.    .xxxxxxxxxxxxxx.    .xxxxxxxxxxxxxxxx..
	 *     .....xxxxxx.....    .xxx........xxx.    .....xxxxxxxxxxxxxx
	 *                         .xx..........xx.
	 *            A                   B                     C
	 * </pre>
	 * Notice grids B and C have both expanded to accommodate the skewing of the ellipse.
	 * Notice also that for grid C, the skewing is to the right; a positive xBend value would have it
	 * going to the left.
	 * Finally, notice that the center/origin of all the grids does not change, even when resized.
	 * @param origin the global origin to which the ellipsoid origin should map,
	 * in the same scale as w and h
	 * @param w the width of the ellipse
	 * @param h the height of the ellipse
	 * @param theta the orientation of the elipse, a value between [0..360) degrees
	 * @param sigma Gaussian scale parameter, passed on to gaussKernel()
	 * @param yBend the factor by which the bottom (positive) or top (negative) side of the
	 * ellipse is bent inward
	 * @param xBend the factor by which the left (positive) or right (negative) side of the
	 * ellipse is bent inward
	 * @throws an RmExceptions::InvalidParameterException if both <code>yBend</code> and
	 * <code>xBend</code> are non-zero
	 */
	RmMutableCartesianGrid<float> gaussGrid( const int w, const int h, const Coord origin, 
		const float theta = 0.0f, const float sigma = 2.1f, const float yBend = 0.0f, 
		const float xBend = 0.0f );

};

#endif