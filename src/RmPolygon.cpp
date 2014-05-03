// RmPolygon.cpp

#pragma warning( disable : 4786 )

#include "RmPolygon.h"
using RmUtility::Coord;

RmPolygon::RmPolygon( const RmUtility::BoundBox &bound )
{
	// Modeled after 
	// void gpc::gpc_read_polygon(FILE *fp, int read_hole_flags, gpc_polygon *p)

	gpc_polygon *p = &m_polygon;
	p->num_contours = 1;
	p->hole = new int[p->num_contours];
	p->contour = new gpc_vertex_list[p->num_contours];
	const int c = 0;
    p->contour[c].num_vertices = 4;
	p->hole[c] = 0;

	p->contour[c].vertex = new gpc_vertex[p->contour[c].num_vertices];
	p->contour[c].vertex[0].x = bound.ul.x;
	p->contour[c].vertex[0].y = bound.ul.y;
	p->contour[c].vertex[1].x = bound.lr.x;
	p->contour[c].vertex[1].y = bound.ul.y;
	p->contour[c].vertex[2].x = bound.lr.x;
	p->contour[c].vertex[2].y = bound.lr.y;
	p->contour[c].vertex[3].x = bound.ul.x;
	p->contour[c].vertex[3].y = bound.lr.y;

	// Have native polygon code validate this bound
	// If invalid, will have num_contours of 0
	intersectWith( *this );
}


RmPolygon::RmPolygon( const gpc_polygon &polygon )
{
	createFrom( polygon );
}


RmPolygon::RmPolygon( const RmPolygon &polygon )
{
	createFrom( polygon.m_polygon );
}


RmPolygon::~RmPolygon()
{
	free();
}


RmPolygon& RmPolygon::operator=( const RmPolygon &polygon )
{
	free();
	createFrom( polygon.m_polygon );
	return *this;
}


void RmPolygon::createFrom( const gpc_polygon &polygon )
{
	// Modeled after 
	// void gpc::gpc_read_polygon(FILE *fp, int read_hole_flags, gpc_polygon *p)

	gpc_polygon *p = &m_polygon;
	p->num_contours = polygon.num_contours;
	p->hole = new int[p->num_contours];
	p->contour = new gpc_vertex_list[p->num_contours];
	for ( int c = 0; c < p->num_contours; ++c ) {
		p->hole[c] = polygon.hole[c];
		p->contour[c].num_vertices = polygon.contour[c].num_vertices;
		p->contour[c].vertex = new gpc_vertex[p->contour[c].num_vertices];
		for ( int v = 0; v < p->contour[c].num_vertices; ++v ) {
			p->contour[c].vertex[v] = polygon.contour[c].vertex[v];
		}
	}
}


void RmPolygon::free()
{
	// Modeled after 
	// void gpc::gpc_free_polygon(gpc_polygon *p)

	for ( int c = 0; c < m_polygon.num_contours; ++c ) {
		delete [] m_polygon.contour[c].vertex;
	}
	delete [] m_polygon.contour;
	delete [] m_polygon.hole;
}


RmPolygon& RmPolygon::intersectWith( const RmPolygon &polygon )
{
	RmPolygon clip( polygon );
	gpc_polygon_clip( GPC_INT, &m_polygon, &clip.m_polygon, &m_polygon );
	return *this;
}


RmPolygon RmPolygon::intersectedWith( const RmPolygon &clip ) const
{
	RmPolygon subj( *this );
	return subj.intersectWith( clip );
}


RmPolygon& RmPolygon::subtractOut( const RmPolygon &polygon )
{
	RmPolygon clip( polygon );
	gpc_polygon_clip( GPC_DIFF, &m_polygon, &clip.m_polygon, &m_polygon );
	return *this;
}


RmPolygon RmPolygon::subtractedOut( const RmPolygon &clip ) const
{
	RmPolygon subj( *this );
	return subj.subtractOut( clip );
}


RmPolygon& RmPolygon::unionWith( const RmPolygon &polygon )
{
	RmPolygon clip( polygon );
	gpc_polygon_clip( GPC_UNION, &m_polygon, &clip.m_polygon, &m_polygon );
	return *this;
}


RmPolygon RmPolygon::unionedWith( const RmPolygon &clip ) const
{
	RmPolygon subj( *this );
	return subj.unionWith( clip );
}


std::ostream& RmPolygon::put( std::ostream &os ) const
{
	os << m_polygon.num_contours << "\n";
	for ( int c = 0; c < m_polygon.num_contours; ++c ) {
		os << m_polygon.contour[c].num_vertices << "\n";
		os << m_polygon.hole[c] << "\n";
		for ( int v = 0; v < m_polygon.contour[c].num_vertices; ++v ) {
			os << "  " << m_polygon.contour[c].vertex[v].x << " " 
				<< m_polygon.contour[c].vertex[v].y << "\n";
		}
	}

	return os;
}


std::ostream& operator<<( std::ostream &os, const RmPolygon &p )
{
	return p.put( os );
}


void RmPolygon::fillInto( std::vector<Coord> *fv ) const
{
	// For each contour polygon
	for ( int c = 0; c < m_polygon.num_contours; ++c )
	{
		// Skip holes
		if ( m_polygon.hole[c] ) continue; 

		// Convert points for use by polyfill routine
		struct Point *points = new Point[m_polygon.contour[c].num_vertices];
		for ( int v = 0; v < m_polygon.contour[c].num_vertices; ++v ) {
			points[v].X = static_cast<int>(m_polygon.contour[c].vertex[v].x);
			points[v].Y = static_cast<int>(m_polygon.contour[c].vertex[v].y);
		}

		PointListHeader pointListHeader = { m_polygon.contour[c].num_vertices, points };

		// Fill contour
		PointList *fill = FillPolygon( 
			&pointListHeader, 0 /*color*/, NONCONVEX, 0 /*x-offset*/, 0 /*y-offset*/ );

		// Push into vector
		while ( fill ) {
			fv->push_back( Coord( fill->point.X, fill->point.Y ) );
			fill = fill->next;
		}
	}
}


std::vector<Coord> RmPolygon::fill() const
{
	std::vector<Coord> fill;
	fillInto( &fill );
	return fill;
}


std::vector<Coord> RmPolygon::line( const Coord &from, const Coord &to )
{
	std::vector<Coord> line;
	PointList* linePointList = FillLine( from.x, from.y, to.x, to.y );  // [start..end)
	while ( linePointList ) 
	{
		line.push_back( Coord( linePointList->point.X, linePointList->point.Y ) );
		linePointList = linePointList->next;
	}

	return line;
}
