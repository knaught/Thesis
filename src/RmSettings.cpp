// RmSettings.cpp

#include <fstream>
#include "RmSettings.h"
#include "RmPioneerController.h"

RmSettings::RmSettings()
{
	// Initialization of settings not saved to file
	SettingsName = "settings.dat";
	EnabledSonars = new bool[RmPioneerController::NumSonars];
	for ( int i = 0; i < RmPioneerController::NumSonars; ++i ) EnabledSonars[i] = true;

	// Attempt init via file
	if ( !read() ) 
	{
		init();
	}
	else {
		std::string iv = invalids();
		if ( iv.length() > 0 ) {
			std::cerr << "Settings file contains invalid data for:\n" << iv << "Using default values.\n\n";
			init();
		}
	}
}


RmSettings::~RmSettings()
{
	// not called in dll mode
	delete[] EnabledSonars;
}


void RmSettings::init()
{
	RegionIHalfwidth = 100;
	SonarModel = RmUtility::SingleCell;

	LocalMapDistance = 5000;
	CellSize = 100;
	PreRotate = false;
	MaxCollectionDistance = 100;
	MaxCollectionDegrees = 5;

	Beta = 15;
	AlphaFactor = 1.0f;
	MaxOccupied = 0.98f;
	MaxEmpty = 1.0f;
	OutOfRangeConversion = 1500;
	IgnoreOutOfRange = true;
	IgnoreObstructed = true;

	Localize = false;
	MotionModel.MinHeight = 0;
	MotionModel.MinWidth = 15;
	MotionModel.UnitDistance = 500;
	MotionModel.UnitTurn = 10;
	MotionModel.GaussianSigma = 5.0f;
	MotionModel.BendFactor = 1.0f;
	ObstructedCertainty = 0.7f;

	GridName = "";
	SonarName = "";
}


bool RmSettings::read()
{
	std::ifstream is( SettingsName.c_str() );
	if ( is ) {
		is >> RegionIHalfwidth;
		int sonarModel;
		is >> sonarModel;
		SonarModel = (RmUtility::SonarModelEnum)sonarModel;
		is >> LocalMapDistance;
		is >> CellSize;
		is >> MaxCollectionDistance;
		is >> MaxCollectionDegrees;
		is >> PreRotate;
		is >> Beta;
		is >> AlphaFactor;
		is >> MaxOccupied;
		is >> MaxEmpty;
		is >> OutOfRangeConversion;
		is >> IgnoreOutOfRange;
		is >> IgnoreObstructed;
		is >> Localize;
		is >> MotionModel.MinHeight;
		is >> MotionModel.MinWidth;
		is >> MotionModel.UnitDistance;
		is >> MotionModel.UnitTurn;
		is >> MotionModel.GaussianSigma;
		is >> MotionModel.BendFactor;
		is >> ObstructedCertainty;
		is >> GridName;
		is >> SonarName;
		is.close();
		return true;
	}
	return false;
}


std::string RmSettings::invalids()
{
	const std::string pre = " - ";
	const std::string post = "\n";
	std::string invalids;

	if ( RegionIHalfwidth < 10 ) invalids += pre + "RegionIHalfwidth " + post;
	if ( SonarModel != RmUtility::SingleCell && SonarModel != RmUtility::AcousticAxis && 
		SonarModel != RmUtility::Cone ) invalids += pre + "SonarModel " + post;
	if ( LocalMapDistance < 100 ) invalids += pre + "LocalMapDistance " + post;
	if ( CellSize < 1 ) invalids += pre + "CellSize " + post;
	if ( MaxCollectionDistance < 1 ) invalids += pre + "MaxCollectionDistance " + post;
	if ( MaxCollectionDegrees < 1 ) invalids += pre + "MaxCollectionDegrees " + post;
	if ( Beta < 7 || Beta > 90 ) invalids += pre + "Beta " + post;
	if ( AlphaFactor <= 0.0 ) invalids += pre + "AlphaFactor " + post;
	if ( MaxOccupied <= 0.0 || MaxOccupied >= 1.0 ) invalids += pre + "MaxOccupied " + post;
	if ( MaxEmpty <= 0.0 || MaxOccupied >= 1.0 ) invalids += pre + "MaxEmpty " + post;
	if ( OutOfRangeConversion < 1 ) invalids += pre + "OutOfRangeConversion " + post;
	if ( MotionModel.MinHeight < 0 ) invalids += pre + "MotionModel.MinHeight " + post;
	if ( MotionModel.MinWidth < 0 ) invalids += pre + "MotionModel.MinWidth " + post;
	if ( MotionModel.UnitDistance < 1 ) invalids += pre + "MotionModel.UnitDistance " + post;
	if ( MotionModel.UnitTurn < 1 ) invalids += pre + "MotionModel.UnitTurn " + post;
	if ( MotionModel.GaussianSigma <= 0.0 ) invalids += pre + "MotionModel.GaussianSigma " + post;
	if ( fabs( MotionModel.BendFactor ) > 20.0  ) invalids += pre + "MotionModel.BendFactor " + post;
	if ( ObstructedCertainty < 0.0 || ObstructedCertainty > 1.0 ) {
		invalids += pre + "ObstructedCertainty " + post;
	}
	if ( GridName.length() == 0 ) invalids += pre + "GridName " + post;

	return invalids;
}


void RmSettings::write()
{
	std::ofstream os( SettingsName.c_str() );
	if ( os ) {
		os << RegionIHalfwidth << " ";
		os << SonarModel << " ";
		os << LocalMapDistance << " ";
		os << CellSize << " ";
		os << MaxCollectionDistance << " ";
		os << MaxCollectionDegrees << " ";
		os << PreRotate << " ";
		os << Beta << " ";
		os << AlphaFactor << " ";
		os << MaxOccupied << " ";
		os << MaxEmpty << " ";
		os << OutOfRangeConversion << " ";
		os << IgnoreOutOfRange << " ";
		os << IgnoreObstructed << " ";
		os << Localize << " ";
		os << MotionModel.MinHeight << " ";
		os << MotionModel.MinWidth << " ";
		os << MotionModel.UnitDistance << " ";
		os << MotionModel.UnitTurn << " ";
		os << MotionModel.GaussianSigma << " ";
		os << MotionModel.BendFactor << " ";
		os << ObstructedCertainty << " ";
		os << GridName << " ";
		os << SonarName;
	}
	os.close();
}


std::ostream& RmSettings::put( std::ostream& os, std::string prefix ) const
{
	os << prefix << "RegionIHalfwidth " << RegionIHalfwidth << "\n";
	os << prefix << "SonarModel " << SonarModel << "\n";
	os << prefix << "LocalMapDistance " << LocalMapDistance << "\n";
	os << prefix << "CellSize " << CellSize << "\n";
	os << prefix << "MaxCollectionDistance " << MaxCollectionDistance << "\n";
	os << prefix << "MaxCollectionDegrees " << MaxCollectionDegrees << "\n";
	os << prefix << "PreRotate " << PreRotate << "\n";
	os << prefix << "Beta " << Beta << "\n";
	os << prefix << "AlphaFactor " << AlphaFactor << "\n";
	os << prefix << "MaxOccupied " << MaxOccupied << "\n";
	os << prefix << "MaxEmpty " << MaxEmpty << "\n";
	os << prefix << "OutOfRangeConversion " << OutOfRangeConversion << "\n";
	os << prefix << "IgnoreOutOfRange " << IgnoreOutOfRange << "\n";
	os << prefix << "IgnoreObstructed " << IgnoreObstructed << "\n";
	os << prefix << "Localize " << Localize << "\n";
	os << prefix << "MotionModel.MinHeight " << MotionModel.MinHeight << "\n";
	os << prefix << "MotionModel.MinWidth " << MotionModel.MinWidth << "\n";
	os << prefix << "MotionModel.UnitDistance " << MotionModel.UnitDistance << "\n";
	os << prefix << "MotionModel.UnitTurn " << MotionModel.UnitTurn << "\n";
	os << prefix << "MotionModel.GaussianSigma " << MotionModel.GaussianSigma << "\n";
	os << prefix << "MotionModel.BendFactor " << MotionModel.BendFactor << "\n";
	os << prefix << "ObstructedCertainty " << ObstructedCertainty << "\n";
	os << prefix << "GridName " << GridName << "\n";
	os << prefix << "SonarName " << SonarName << "\n";
	return os;
}


std::ostream& operator<<( std::ostream& os, const RmSettings& s )
{
	return s.put( os );
}