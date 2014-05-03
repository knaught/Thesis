// RmGlobalMap.cpp

/* Notes:
 * On nomenclature.  Variables that have a world (unscaled) or grid (scaled) frame of reference are
 * prepended with a "w" or "g", respectively.
 */

#pragma warning( disable : 4786 )

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>
#include "RmGlobalMap.h"
using RmGlobalMap::RegionId;
#include "RmUtilityExt.h"
#include "RmExceptions.h"
using namespace RmExceptions;
#include "RmUtility.h"


using RmUtility::BoundBox;
using RmUtility::Coord;
using RmUtility::Pose;
using RmUtility::SonarReading;
using RmUtility::MappedSonarReading;


RmGlobalMap::RmGlobalMap( RmSettings* s )
	: RmBayesCertaintyGrid( s, Coord() ),
	  m_settings(s), m_finalized(false), m_currentMap(NULL), m_wDistance(0.0), 
	  m_regionMap(), m_maxRegionId(0)
{
	static const char *signature_ = "RmGlobalMap::RmGlobalMap(RmSettings*)";
	if ( s == NULL ) throw InvalidParameterException( signature_, "RmSettings may not be null" );

	// Open localization log file
	if ( s->Localize && s->GridName != "" ) {
		m_debugLogName = s->GridName;
		m_debugLogName.append( ".log" );
		m_debugLog.open( m_debugLogName.c_str() );
		m_debugLog.setf( std::ios_base::fixed, std::ios_base::floatfield );
		m_debugLog.precision( 4 );
	}
}


void RmGlobalMap::addToRegionMap( RmLocalMap *mt )
{
	// Integrates newly built independent L_t into G

	// Validate input
	assert( mt != NULL );
	assert( mt->cumDistance() > 0 );
	assert( m_maps.size() > 0 );

	// Init region boundary (rt) to that of the local map at time t (lt)
	// Note: map boundaries are expanded by one on their north and eastern borders so as to
	// compensate for the polygon filling routine which excludes cells along those borders
	RmPolygon rt( mt->bound().expandBy( 1, 0, 1, 0 ) );

	// Init history of processed regions to region 0, that is, the non-region
	std::set<RegionId> processedRegions;
	processedRegions.insert( 0 ); // non-region has id of 0, which is treated as processed

	// For each prior local map (mi{i=t-1..0})
	std::vector<RmLocalMap*>::reverse_iterator mip = m_maps.rbegin();
	while ( ++mip != m_maps.rend() )
	{
		// If prior local map i (bi) intersects that at time t (bx)
		const RmPolygon bi( (*mip)->bound().expandBy( 1, 0, 1, 0 ) );
		const RmPolygon bx( bi.intersectedWith( rt ) );
		if ( bx.numContours() > 0 ) 
		{
			// For each cell in global map (G) over intersection (bx)
			std::vector<Coord> fill;
			bx.fillInto( &fill );
			std::vector<Coord>::const_iterator cell;
			for( cell = fill.begin(); cell != fill.end(); ++cell )
			{
				// Get region assigned to that cell
				const RegionId regionId = m_regionMap.valueAt( cell->x, cell->y );

				// If region not previously processed
				if ( processedRegions.count( regionId ) == 0 )
				{
					// Get region's intersection (rx) with map bound intersection (bx)
					Region *rc = m_regions[regionId];
					// ***
					const RmPolygon rx( rc->boundary.intersectedWith( bx ) );
					// Subtract region intersection (rx) from bound of map at time t (rt)
					rt -= rx;

					// Subtract region intersection (rx) from region at current cell (rc)
					RmPolygon bcx( rc->boundary - rx );

					// [Remove Rc->L that no longer intersect]

					// If region at cell (rc) is now empty, delete the region
					// Note: must use temp bcx so deleteRegion() has rc->boundary to work with
					if ( bcx.numContours() == 0 ) deleteRegion( &rc );
					else rc->boundary = bcx;
					
					// Create new region (rnew) as intersection of cell region + map intersection
					// Fill new region (rnew) over global map (G) with Rnew.id
					Region *rnew = newRegion( rx );

					// Add map at time i (mi) and t (mt) to new region (rnew)
					rnew->maps.insert( *mip );
					rnew->maps.insert( mt );

					// Add maps from cell region (rc) that intersect new region (rnew)
					if ( rc != NULL ) { // (if not just deleted)
						std::set<RmLocalMap*>::const_iterator mcp;					
						for ( mcp = rc->maps.begin(); mcp != rc->maps.end(); ++mcp ) 
						{
							// Ignore those already assigned to the region
							if ( *mcp == *mip || *mcp == mt ) continue;

							// If intersects the region
							if ( rx.intersectedWith( 
								RmPolygon( (*mcp)->bound() ) ).numContours() > 0 ) 
							{
								rnew->maps.insert( *mcp );
							}
						}
					}

					// Mark intersecting region as processed (original and new)
					processedRegions.insert( regionId );
					processedRegions.insert( rnew->id );
				}
			}
		}
	}

	
	// If B not empty
	if ( rt.numContours() != 0 )
	{
		// Create new region R_t with boundary Bt
		// For each G region cell in L_t, replace with R_new.id
		Region *rnew = newRegion( rt );

		// Add L_t to R_new
		rnew->maps.insert( mt );
	}
}


std::string RmGlobalMap::clearMapString( const BoundBox &gBound ) const
{
	char *buff = new char[(gBound.ul.y - gBound.lr.y + 1) * (gBound.lr.x - gBound.ul.x + 1) * 17 + 1];
	char *buffPtr = &buff[0];

	for ( int gY = gBound.ul.y; gY >= gBound.lr.y; --gY ) {
		for ( int gX = gBound.ul.x; gX <= gBound.lr.x; ++gX ) {
			sprintf( buffPtr, "%d %d %.4f;", gX, gY, RmBayesCertaintyGrid::InitVal );
			buffPtr += strlen( buffPtr );
		}
	}

	std::string logString( buff );
	delete[] buff;

	return logString;
}


float RmGlobalMap::convolvedValueAt( int x, int y ) const
{
	RegionId rId; // region id

	// If the requested cell is out of bounds are not covered by a region,
	// return the default value
	if ( !inBounds( Coord( x, y ) ) || (rId = m_regionMap[x][y]) == 0 ) {
		return RmBayesCertaintyGrid::InitVal;
	}

	// Get the region covering the cell
	std::map<RegionId,Region*>::const_iterator ri( m_regions.find( rId ) ); // region iterator
	assert( ri != m_regions.end() );
	Region *r = (*ri).second;

	// Sum non-empty probabilities for each local map assigned to the region
	float gPr = 0.0f; // global map prior probability
	int cnt = 0;
	std::set<RmLocalMap*>::const_iterator mi; // map iterator
	for ( mi = r->maps.begin(); mi != r->maps.end(); ++mi )
	{
		// Convolve values
		float pr = (*mi)->valueAt( x, y );
		if ( pr != RmBayesCertaintyGrid::InitVal ) {
			gPr += pr;
			++cnt;
		}
	}
	gPr = cnt == 0 ? RmBayesCertaintyGrid::InitVal : gPr / cnt;

	return gPr;
}


void RmGlobalMap::deleteRegion( Region **const r )
{
	assert( *r != NULL );
	assert( (*r)->id > 0 );

	Region *rorig = *r; // save state so can assert it doesn't change

	fillRegionMap( *r, 0 );
	m_usedRegionIds.push( (*r)->id );
	m_regions.erase( (*r)->id );

	assert( rorig == *r ); // this failed when calling this method from within an iteration
		// over m_regions in RmGlobalMap::empty() because m_regions.erase() would delete *r first,
		// and a second attempt below would cause a crash

	delete *r;
	*r = NULL;
}


void RmGlobalMap::empty()
{
	// Free memory allocated for local maps
	std::vector<RmLocalMap*>::const_iterator map;
	for ( map = m_maps.begin(); map != m_maps.end(); ++map ) delete *map;
	m_maps.clear();

	// Free memory allocated for regions
	std::map<RegionId,Region*>::const_iterator region;
	for ( region = m_regions.begin(); region != m_regions.end(); ++region ) 
	{
		const Region *r = (*region).second;
		delete r;
		r = NULL;
	}
	m_regions.clear(); // empty collection of region pointers
	m_regionMap.empty(); // empty the region grid
	while ( !m_usedRegionIds.empty() ) m_usedRegionIds.pop(); // empty collection of used region ids
	m_maxRegionId = 0; // reset next region id

	m_wDistance = 0.0;
	m_currentMap = NULL;
	m_gAccumShift = Pose();
	m_finalized = false;
	m_debugLog.seekp( 0 ); // in lieu of closing and reopening, which doesn't work in dll mode

	RmMutableCartesianGrid<float>::empty();
}


void RmGlobalMap::fillRegionMap( const Region *const r, const RegionId id )
{
	assert( r != NULL );
	assert( id <= m_maxRegionId );

	std::vector<Coord> fill;
	r->boundary.fillInto( &fill );
	std::vector<Coord>::const_iterator cell;
	for( cell = fill.begin(); cell != fill.end(); ++cell )
	{
		// Replace with R_new.id
		m_regionMap.valueAt( cell->x, cell->y ) = id;
	}
}


void RmGlobalMap::finalize()
{
	if ( m_finalized ) return;

	if ( m_currentMap != NULL ) addToRegionMap( m_currentMap );
	integrate();

	m_finalized = true;
}


std::string RmGlobalMap::installNewMap( const SonarReading& wNewReading )
{
	// NOTE:  
	// In this context, the "current" map is the map just built in its entirety but not yet
	// convolved with the global map; the "new" map isn't created until after this current
	// map has been relocalized and convolved, and a new map pose localization accomplished
	// using newReading

	// Given three consecutive local maps, A, B, and C, with A being the first map built:
	// Pose A is neither localized prior to construction nor relocalized after construction
	//		because there is no current or prior map data to localize against
	// Pose B is localized prior to construction using map data from A, 
	//		and relocalized after construction using its own data
	// Pose C is processed in like manner to B, but in reference to B, and so on

	static SonarReading wCurrentReading_; // saves reading used for current map's pose
	std::string logString;

	if ( m_currentMap != NULL )
	{
		RmPolygon dirtyRegion( m_currentMap->bound() );

		//////
		// Relocalize map just finished building (at t-1)

		if ( m_settings->Localize && m_maps.size() > 1 ) 
		{
			// NOTE: m_current map will be non-null and m_maps size will be == 1
			// at the transition between maps A and B, as described above;
			// The test for size > 1 prevents a doomed attempt to relocalize map A

			// Convolve local map into global map
			addToRegionMap( m_currentMap );

			// Get updated relocalized pose for local map
			RmLocalMap &priorMap = *m_maps[m_maps.size() - 2]; // at transition between B and C, priorMap = A
			Pose gLocPose = localizedPose( priorMap, wCurrentReading_, m_debugLog );
			Pose gOldPose = m_currentMap->pose().scaled( m_settings->CellSize );
			gOldPose.theta = priorMap.pose().coord.scaled( m_settings->CellSize ).angleTo( gOldPose.coord );
			Pose gPoseShift = gLocPose - gOldPose; // scaled

			// Restore global map to pre-convolved state
			removeFromRegionMap( m_currentMap );

			// Shift local map by relocalized pose shift
			m_gAccumShift += gPoseShift;
			m_currentMap->reorientBy( gPoseShift.scaled( 1.0 / m_settings->CellSize ) );

			// Expand dirty region to include shift
			dirtyRegion.unionWith( RmPolygon( m_currentMap->bound() ) );

			if ( m_debugLog ) m_debugLog << "installNewMap() relocalize : " << gOldPose << ", " << gLocPose << ", " 
				<< gPoseShift << ", " << m_gAccumShift << std::endl;
		}	


		//////
		// Convolve newly finished map with global map

		// Add to region map
		addToRegionMap( m_currentMap );
		logString += integrate( dirtyRegion, true );


		//////
		// Localize new map based on map just finished

		if ( m_settings->Localize ) 
		{
			// Find delta between selected pose of new and finished local map
			// Shift local map pose by delta (heading?)
			// calc angle between coords of current and new map

			Pose gLocPose = localizedPose( *m_currentMap, wNewReading, m_debugLog );
			Pose gPrePose( wNewReading.robotPose.scaled( m_settings->CellSize ) );
			gPrePose.theta = m_currentMap->pose().coord.scaled( m_settings->CellSize ).angleTo( gPrePose.coord );
			Pose gPoseShift = gLocPose - gPrePose;
			m_gAccumShift += gPoseShift;

			if ( m_debugLog ) m_debugLog << "installNewMap() localize : " << gPrePose << ", " << gLocPose << ", " 
				<< gPoseShift << ", " << m_gAccumShift << std::endl;
		}
	}


	//////
	// Create new map, beginning with sonar data just used

	wCurrentReading_ = wNewReading;
	wCurrentReading_.robotPose = wNewReading.robotPose + m_gAccumShift.scaled( 1.0 / m_settings->CellSize );
	m_maps.push_back( m_currentMap = new RmLocalMap( m_settings, wCurrentReading_.robotPose ) );

	return logString;
}


void RmGlobalMap::integrate()
{
	// For each cell in the region map (which represents the entire global map)
	const BoundBox bound( m_regionMap.bound() );
	for ( int y = bound.ul.y; y >= bound.lr.y; --y ) {
		for ( int x = bound.ul.x; x <= bound.lr.x; ++x ) 
		{
			// Store the convolved value of all local maps over the cell
			const RegionId rId = m_regionMap.valueAt( x, y );
			valueAt( x, y ) = convolvedValueAt( x, y );
		}
	}
}



const std::string RmGlobalMap::integrate( const RmPolygon &bound, bool retVal )
{
	// get cells filling region
	std::vector<Coord> fill;
	bound.fillInto( &fill );

	// create update string buffer
	// format is "-xxx -yyy 0.dddd;"
	char *buff = new char[(fill.size() * 17 + 1) * sizeof(char)];
	char *buffPtr = buff;
	*buffPtr = '\0';

	// for each cell
	std::vector<Coord>::const_iterator ci;
	for ( ci = fill.begin(); ci != fill.end(); ++ci ) 
	{
		const float pr = convolvedValueAt( ci->x, ci->y );
		valueAt( ci->x, ci->y ) = pr;
		
		// update log string by appending over previous terminating null
		if ( retVal ) {
			sprintf( buffPtr, "%d %d %.4f;", ci->x, ci->y, pr );
			buffPtr += strlen( buffPtr );
		}
	}

	// tack on local map update string
	std::string updateString( buff );
	delete[] buff;

	return updateString;
}



#undef _LOG

Pose RmGlobalMap::localizedPose( 
	const RmLocalMap &priorMap, const SonarReading& wReading, std::ofstream& log ) const
{
	// Note:  unscaled -> in   out -> scaled

	static int cnt_ = 0;
	++cnt_;
	#ifdef _LOG
	log << "\n\nLOCAL MAP " << ((cnt_+1)/2) << "\nDimension: " << priorMap.width() << " x " << priorMap.height() << "\n";
	log << "SonarReading:\n" << wReading << "\n";
	#endif

	const RmBayesSonarModel sonarModel( m_settings ); // used to calc Pr(Occ)
	const Pose gPose( wReading.robotPose.scaled( m_settings->CellSize ) );

	// Calculate probability distribution of pose using motion model
	// Factors:  distance and degree of turns, where
	//    height increases proportionally to distance traveled
	//    width increases proportional to degrees turned
	const int h = m_settings->MotionModel.MinHeight + 
		priorMap.cumDistance() / m_settings->MotionModel.UnitDistance;
	const int w = m_settings->MotionModel.MinWidth + 
		priorMap.cumTurn() / m_settings->MotionModel.UnitTurn;
	RmMutableCartesianGrid<float> gPoseDist( RmUtility::gaussGrid( w, h, gPose.coord, gPose.theta, 
		m_settings->MotionModel.GaussianSigma, m_settings->MotionModel.BendFactor ) );

	#ifdef _LOG
	log << "cumDistance(" << priorMap.cumDistance()
		<< ") cumTurn(" << priorMap.cumTurn() << ") gPoseDist[" << w << "][" << h << "]\n";
	#endif

	//*** Debug
	// #ifndef _USRDLL
	#ifdef _LOG
	if ( log )
	{
		char *buff = new char[m_settings->GridName.length() + 7];

		// Global map
		sprintf( buff, "%s%02d.gd", m_settings->GridName.c_str(), cnt_ );
		//sprintf( buff, "%s.gd", m_settings->GridName.c_str() );
		integrate();
		RmMutableCartesianGrid<float>::put( buff );

		// Global map with pose distributions
		static std::vector<RmMutableCartesianGrid<float> > gPoseDists_;
		gPoseDists.push_back( gPoseDist );
		RmMutableCartesianGrid<float> gpm( *this );
		std::vector<RmMutableCartesianGrid<float> >::const_iterator it;
		for ( it = gPoseDists_.begin(); it < gPoseDists_.end(); ++it ) gpm.mergeWith( *it );
		sprintf( buff, "%s%02dp.gd", m_settings->GridName.c_str(), cnt_ );
		//sprintf( buff, "%sp.gd", m_settings->GridName.c_str() );
		gpm.put( buff );

		delete[] buff;
	}
	#endif
	// #endif
	//*** End Debug
	
	// Histogram matrix over pose for tabulating selected poses for all sonars
	RmMutableCartesianGrid<float> gPoseHist( gPoseDist );
	gPoseHist.clear();
	int maxPoseHist = 0; // highest value in histogram matrix
	
	// Create pose selection matrix, reinitialized for each sonar
	// Note: while gPoseSel is a histogrammic matrix, are using float rather than int because can't
	// easily copy configuration of gPoseDist any other way, and template class copy constructor
	// requires same type.
	RmMutableCartesianGrid<float> gPoseSel( gPoseDist ); // convolves prOcc with gPoseDist
	#ifdef _LOG
	RmMutableCartesianGrid<float> gPoseObs( gPoseDist ); // tracks obstructions; log use only
	RmMutableCartesianGrid<float> gPoseGlo( gPoseDist ); // global map about range reading; log use only
	RmMutableCartesianGrid<float> gPoseOcc( gPoseDist ); // occ grid about range reading; log use only
	gPoseOcc.setInitValue( 0.5f );
	#endif

	// For each sonar
	bool noneInRange = true; // indicates no sonar readings in range
	SonarReading wReadingCopy( wReading ); // _UNSCALED_LOCALIZATION copy to be modified
	for ( int i = 0; i < RmPioneerController::NumSonars; ++i )
	{
		// Calc grid-based vector (direction and magnitude) for the range reading
		wReadingCopy.sonarNumber = i;
		wReadingCopy.distance = wReadingCopy.all[i];
		MappedSonarReading &gMR = 
			RmPioneerController::rangeReading( wReadingCopy ).scale( m_settings->CellSize );
			// rangeReading() is best used with unscaled data
		#ifdef _LOG
		log << "\n" << gMR;
		#endif

		// Skip out-of-range readings
		// (placed after calculation of mr so mr log entry can first be entered)
		if ( wReadingCopy.distance > RmPioneerController::SonarRange ) {
			#ifdef _LOG
			log << "Skipping out of range reading.\n";
			#endif
			continue;
		}
		noneInRange = false;


		// Init pose selection and log matrices
		gPoseSel.clear();
		#ifdef _LOG
		gPoseObs.clear();
		gPoseOcc.setOrigin( gMR.objectCoord );
		gPoseOcc.clear();
		#endif

		// For each cell in pose distribution matrix
		float maxPoseSel = 0.0; // highest value in pose selection matrix
		const BoundBox gBound = gPoseDist.bound();
		const Coord gSonarShift = gMR.sonarPose.coord - gPose.coord;
		const Coord gObjectShift = gMR.objectCoord - gMR.sonarPose.coord;
		for ( int gY = gBound.ul.y; gY >= gBound.lr.y; --gY ) {
			for ( int gX = gBound.ul.x; gX <= gBound.lr.x; ++gX ) 
			{
				// If cell at terminal end of range reading vector is unobstructed, 
				// record pose selection likelihood
				const Coord gStart = Coord( gX, gY ) + gSonarShift;
				const Coord gEnd = gStart + gObjectShift;
				if ( !obstructionBetween( gStart, gEnd ) ) 
				{
					#ifdef _LOG
					// Record unostructed path for log output
					gPoseObs[gX][gY] = 1.0f;
					#endif

					// get prior prob of occupied from global map
					const float priorPrOcc = convolvedValueAt( gEnd.x, gEnd.y );

					// get new prob of occupied for this pose dist cell
					const float prOcc = sonarModel.prOccupiedGivenSn( 
						priorPrOcc, RmBayesSonarModel::RegionI, gMR.reading.distance );
					#ifdef _LOG
					gPoseOcc[gEnd.x][gEnd.y] = prOcc;
					#endif

					const float s = gPoseSel[gX][gY] = prOcc * gPoseDist[gX][gY];
					
					if ( s > maxPoseSel ) maxPoseSel = s;
				}
			}
		}

		// Increment in pose histogram the cells corresponding to 
		// those in pose selection matrix with highest prob
		if ( maxPoseSel > 0 ) {
			for ( gY = gPoseSel.bound().ul.y; gY >= gPoseSel.bound().lr.y; --gY ) {
				for ( int gX = gPoseSel.bound().ul.x; gX <= gPoseSel.bound().lr.x; ++gX ) 
				{
					if ( gPoseSel[gX][gY] == maxPoseSel ) {
						const int h = ++gPoseHist[gX][gY]; // store so don't have to call [][] twice
						if ( h > maxPoseHist ) maxPoseHist = h;
					}
				}
			}
		}

		#ifdef _LOG
		gPoseGlo.setOrigin( gMR.objectCoord );
		copyInto( gPoseGlo );
		log << "PrOcc at object = " << gPoseOcc[gMR.objectCoord.x][gMR.objectCoord.y] << ", prior = " << gPoseGlo[gMR.objectCoord.x][gMR.objectCoord.y] << "\n";
		log << "\nposeDistribution over Robot Pose, by local map, " << "Bound: " << gPoseDist.bound() << " Origin: " 
			<< gPoseDist.origin() << "\n" << gPoseDist;
		log << "\nglobalMap over Object, by sonar, " << "Bound: " << gPoseGlo.bound() << " Origin: " 
			<< gPoseGlo.origin() << "\n" << gPoseGlo;
		log << "\nposeObstructions over Object, by sonar, 0 = obstructed (path from Robot Pose to Object)\n" << gPoseObs;
		log << "\nprOcc over Object, by unobstructed sonar\n" << gPoseOcc;
		log << "\nposeSelelections over Object, by sonar (gPoseDist * prOcc * gPoseObs)\n" << gPoseSel << "\n";
		log << "poseHistogram over Robot Pose, by local map (max gPoseSel)\n" << gPoseHist << "\n";
		
		log << "===== End Sonar " << i << "\n";
		#endif
	}

	// If no poses were selected, all sonar readings were out of range
	// Can only return original pose
	if ( maxPoseHist == 0 ) {
		#ifdef _LOG
		log << "\nNo sonar readings in range!\n\n";
		#endif
		Pose gLocPose( wReading.robotPose.scaled( m_settings->CellSize ) );
		gLocPose.theta = priorMap.pose().coord.angleTo( wReading.robotPose.coord );
		return gLocPose;
	}

	// Get poses with max pose histogram value and record corresponding max post dist value
	std::vector<Coord> gSelectedPoses;
	float maxSelPoseDist = 0.0f;
	for ( int gY = gPoseHist.bound().ul.y; gY >= gPoseHist.bound().lr.y; --gY ) {
		for ( int gX = gPoseHist.bound().ul.x; gX <= gPoseHist.bound().lr.x; ++gX ) 
		{
			if ( gPoseHist[gX][gY] == maxPoseHist ) {
				gSelectedPoses.push_back( Coord( gX, gY ) );
				const float pd = gPoseDist[gX][gY]; // store so don't have to call [][] twice
				if ( pd > maxSelPoseDist ) maxSelPoseDist = pd;
			}
		}
	}

	// Filter all but those with the highest underlying maximum pose distribution
	log << "maxSelPoseDist = " << maxSelPoseDist << "\n";
	std::vector<Coord>::iterator gSelectedPose = gSelectedPoses.begin();
	while ( gSelectedPose < gSelectedPoses.end() )
	{
		const Coord gc( *gSelectedPose );
		log << gc << ":" << gPoseDist[gc.x][gc.y] << " ";
		if ( gPoseDist[gc.x][gc.y] < maxSelPoseDist ) {
			gSelectedPoses.erase( gSelectedPose ); // implicit ++gSelectedPose
			log << "erased\n";
		}
		else {
			++gSelectedPose;
			log << "retained\n";
		}
	}

	// If more than one selected pose, take the first (for now)
	if ( gSelectedPoses.size() != 1 ) 
	{
		#ifdef _LOG
		log << "maxSelPoseDist = " << maxSelPoseDist << "\n";
		log << "candidate poses: ";
		for ( gSelectedPose = gSelectedPoses.begin(); gSelectedPose < gSelectedPoses.end(); ++gSelectedPose )
		{
			log << *gSelectedPose << " ";
		}
		log << "\n";
		#endif
		char buff[50];
		sprintf( buff, "%d poses remain in filtered selection matrix", gSelectedPoses.size() );
		RmExceptions::Exception e( NULL, "RmGlobalMap::localizedPose()", buff );

		// This isn't an "error" so much as it is a condition;
		// Notify and keep going
		std::cerr << e;

		// Additional filters for narrowing down selections:
		// - highest avg of surrounding cells, etc.
		// - closes proximity to original pose
	}

	Coord gSelCoord( gSelectedPoses.front() );

	// Return the selected position and angle to that pos from current map's starting position
	Pose gLocPose( gSelCoord, priorMap.pose().coord.scaled( m_settings->CellSize ).angleTo( gSelCoord ) );

	#ifdef _LOG
	log << "selPose: " << gLocPose << std::endl;
	#endif

	return gLocPose;
}


RmGlobalMap::Region* RmGlobalMap::newRegion( const RmPolygon &bound )
{
	RegionId id;
	if ( m_usedRegionIds.empty() ) id = ++m_maxRegionId;
	else {
		id = m_usedRegionIds.front();
		m_usedRegionIds.pop();
	}

	Region *r = new Region( id, bound );
	m_regions[id] = r;
	
	fillRegionMap( r, id );

	return r;
}


bool RmGlobalMap::obstructionBetween( const Coord& gStart, const Coord& gEnd ) const
{
	// Reasons for differences from original:
	// - Checking for obstruction to gcObject rather than f (in RmBayesCertaintyGrid obstruction check)
	// - Not processing last point in linePointList
	// - Checking global map

	bool obstructed = false;
	float prGlobal = 0.0f;
	float prLocal = 0.0f;
	const float prObstr = m_settings->ObstructedCertainty;

	for ( PointList* gLinePointList = FillLine( gStart.x, gStart.y, gEnd.x, gEnd.y ); // [start..end)
		gLinePointList; gLinePointList = gLinePointList->next )
	{
		const int gX = gLinePointList->point.X;
		const int gY = gLinePointList->point.Y;

		// These tests short-circuit further testing of either local or global bounds 
		// once it has been determined one of them has been exceeded
		int oneInBounds = 0;
		if ( prGlobal != -1.0f ) {
			if ( m_regionMap.inBounds( gX, gY ) ) {
				prGlobal = convolvedValueAt( gX, gY );
				++oneInBounds;
			}
			else prGlobal = -1.0f;
		}
		if ( prLocal != -1.0f ) {
			if ( m_currentMap->inBounds( gX, gY ) ) {
				prLocal = m_currentMap->valueAt( gX, gY );
				++oneInBounds;
			}
			else prLocal = -1.0f;
		}

		// This test short-circuits all future testing if neither one are in-bounds
		if ( !oneInBounds ) 
			break;

		if ( prLocal >= prObstr || prGlobal >= prObstr )
		{
			obstructed = true;
			break;
		}
	}

	return obstructed;
}


std::ostream& RmGlobalMap::put( std::ostream& os ) const
{ 
	m_settings->put( os, "% " );

	return RmBayesCertaintyGrid::put( os );
}


void RmGlobalMap::removeFromRegionMap( RmLocalMap *map )
{
	assert( map != NULL );

	// Set up tracking of processed regions
	std::set<RegionId> regionsProcessed;
	regionsProcessed.insert( 0 ); // non-region has id of 0, which is treated as processed

	// For each cell within the map's bounds (with explicit inclusion of n/e borders)
	RmPolygon bo( map->bound().expandBy( 1, 0, 1, 0 ) );
	std::vector<Coord> fill;
	bo.fillInto( &fill );
	std::vector<Coord>::const_iterator ci;
	for ( ci = fill.begin(); ci != fill.end(); ++ci ) 
	{
		// For regions not already processed
		const RegionId regionId = m_regionMap.valueAt( ci->x, ci->y );
		if ( regionsProcessed.count( regionId ) == 0 ) 
		{
			// Remove the map from the region
			Region *r = m_regions[regionId];
			r->maps.erase( map );

			// If no maps cover the region any longer, delete it
			if ( r->maps.size() == 0 ) deleteRegion( &r );

			// Mark region as processed
			regionsProcessed.insert( regionId );
		}
	}
}


const std::string RmGlobalMap::update( const SonarReading& wReading )
{
	if ( m_finalized ) return "";

	// Accummulate distance traveled for current map
	static bool newMap_ = true;
	static Coord wLastPos_;
	Coord wCurrPos = wReading.robotPose.coord;
	if ( !newMap_ ) m_wDistance += wLastPos_.distanceFrom( wCurrPos );
	wLastPos_ = wCurrPos;
	newMap_ = false;

	// Initialize new map on first run
	// Would do in constructor but would have to assume origin pose of Pose()
	std::string newMapString;
	if ( m_currentMap == NULL )
	{
		m_debugLog << "Settings:\n" << *m_settings << "\n";
		newMapString = installNewMap( wReading );
	}

	// If total distance exceeds prescribed amount
	else if ( m_settings->LocalMapDistance > 0 && m_wDistance > m_settings->LocalMapDistance )
	{
		// Create new map
		// Add to map collection
		// Set current local map to new map
		if ( m_settings->Localize ) newMapString = installNewMap( wReading );
		else installNewMap( wReading );

		newMap_ = true;
		m_wDistance = 0.0;
	}

	// Modify reading by pose shift updated in installNewMap()
	SonarReading wShiftedReading( wReading );
	wShiftedReading.robotPose += m_gAccumShift.scaled( 1.0 / m_settings->CellSize );

	// Prepare default pose string
	Pose gRobotPose( wShiftedReading.robotPose.scaled( m_settings->CellSize ) );
	char logPose[30];
	sprintf( logPose, "%d %d %d %d %d\n", gRobotPose.coord.x, gRobotPose.coord.y, 
		static_cast<int>(wReading.robotPose.theta), wReading.sonarNumber, wReading.distance );

	// Skip obstructed readings (cell and axis models only)
	const RmUtility::MappedSonarReading wMR = RmPioneerController::rangeReading( wShiftedReading );
	if ( m_settings->SonarModel != RmUtility::Cone && m_settings->IgnoreObstructed && 
		obstructionBetween( gridCoord( wMR.sonarPose.coord ), gridCoord( wMR.objectCoord ) ) ) {
		return "";
	}

	// Pass reading on to current local map
	std::string logString = m_currentMap->update( wMR );

	// If update string is empty (due to out-of-range or disabled sonar)
	// but we have a new map update string, include log line 1 for return to viewer
	if ( logString.length() == 0 && newMapString.length() > 0 ) logString = logPose;
	else logString = logString.substr( 0, logString.length() - 2 );
	logString += newMapString;

	return logString;
}


template<class T>
std::ostream& operator<<( std::ostream& os, const std::vector<T>& v )
{
	std::vector<T>::const_iterator it;
	for( it = v.begin(); it != v.end(); ++it ) {
		os << *it << " ";
	}
	return os;
}
