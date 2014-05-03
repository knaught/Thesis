/* GridView.java
 * Created on May 25, 2004 * Last revised Sep 28, 2004
 */

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;import javax.swing.event.*;

public class GridView extends JPanel
{	private static final boolean DEBUG_TRACE = false;		public static final int MODE_LIVE = 0; // receive range data from simulator or robot feed
	public static final int MODE_ANIM = 1; // retrieve range data from file at regular intervals	public static final int MODE_ANIM_INSTANT = 2; // retrieve all range data from file at once before drawing map	public static final int MODE_STATIC = 3;
	public static final int CELL_SIZE = 4;
	public static final int CELL_SIZE_MIN = 2;
	public static final int CELL_SIZE_MAX = 30;
	public static final int LINED_GRID_CELL_INTERVAL = 10;	public static final int ZOOM_STEP = 2;
	private GridModel m_gridModel;	private int m_gridSize;
	private int m_mode;	private int m_cellSize = CELL_SIZE;	private int m_lineInterval = LINED_GRID_CELL_INTERVAL * m_cellSize;
	private Rectangle m_mapRegion;	private GridView m_gridView = this;
	private GridModel.Cell m_robotPos;
	private boolean m_trackRobot = false;
	private boolean m_drawGrid = true;
	private boolean m_drawAxis = true;
	private int m_trackCounter = 0;
	
	GridView( GridModel gridModel )
	{
		if ( DEBUG_TRACE ) System.err.println( "GridView.GridView()" );
		m_gridModel = gridModel;		init();
		
		setPreferredSize( new Dimension( m_gridSize * m_cellSize, m_gridSize * m_cellSize ) );
		setDoubleBuffered( true );
		setFocusable( true );		
		MouseInputAdapter mouseAdapter = installMouseAdapter();		addMouseListener( mouseAdapter );		addMouseMotionListener( mouseAdapter );
		
		repaint();	}
	
	public void init()
	{
		m_mapRegion = m_gridModel.getDimension();
		m_gridSize = m_gridModel.getGridSize();		m_mapRegion = new Rectangle( m_gridSize / 2, m_gridSize / 2 - 10, 0, 0 );		m_robotPos = new GridModel.Cell();		m_robotPos.x = m_robotPos.y = GridModel.GRID_SIZE / 2;

		switch ( m_gridModel.getDataSource() ) {
			case GridModel.SOURCE_LIVE:				m_mode = MODE_LIVE;
				scrollToOriginCorner();				break;			case GridModel.SOURCE_RECORDED:				m_mode = MODE_ANIM;				scrollToOriginCorner();				break;			case GridModel.SOURCE_NONE:
				m_mode = MODE_STATIC;				scrollToOriginCorner();				break;			case GridModel.SOURCE_STATIC:				m_mode = MODE_STATIC;				m_mapRegion = m_mapRegion.union( m_gridModel.getDimension() );
				scrollToMapCenter();
				break;		}

	}		public GridModel getModel()	{
		if ( DEBUG_TRACE ) System.err.println( "GridView.getModel()" );
		return m_gridModel;	}
	
	public int getMode()
	{		return m_mode;
	}		public void setRegion( Rectangle region )	{
		m_mapRegion = region;	}		public void setGridVisible( boolean visible )	{
		m_drawGrid = visible;
		repaint();	}
	
	public boolean getGridVisible()
	{
		return m_drawGrid;
	}		public void setAxisVisible( boolean visible )	{		m_drawAxis = visible;		repaint();	}		public boolean getAxisVisible()	{		return m_drawAxis;	}
		/**
	 * This is called by an AnimationToolBar AbstractAction each time the user steps through 
	 * the map.
	 */
	public GridModel.RangeReading update()
	{
		if ( DEBUG_TRACE ) System.err.println( "GridView.update(): mode " + m_mode );
		
		if ( m_mode == MODE_ANIM || m_mode == MODE_LIVE ) {			GridModel.RangeReading rangeReading = m_gridModel.update( );			if ( rangeReading != null ) 
			{				if ( DEBUG_TRACE ) System.err.println( "GridView.update(): " + rangeReading.range );				
				Rectangle gridRegion = rangeReading.boundary;				if ( gridRegion == null ) {					System.err.println( "GridView.update() gridRegion == null" );					return null;				}				
				Rectangle clientRegion = new Rectangle( 					(int)gridRegion.getX() * m_cellSize - 1, 
					(int)gridRegion.getY() * m_cellSize - 1,					(int)gridRegion.getWidth() * m_cellSize + m_cellSize + 1, 					(int)gridRegion.getHeight() * m_cellSize + m_cellSize + 1 );				m_mapRegion = m_mapRegion.union( gridRegion );
				m_robotPos.x = rangeReading.poseX + GridModel.GRID_SIZE / 2;
				m_robotPos.y = rangeReading.poseY + GridModel.GRID_SIZE / 2;
				
				if ( m_trackRobot && m_trackCounter++ % (1280 / m_cellSize) == 0 ) scrollToRobotCenter();				else repaint( clientRegion );				
				return rangeReading;			}
		}		return null;
	}		public void updateAll()	{
		if ( DEBUG_TRACE ) System.err.println( "GridView.updateAll()" );		m_mode = MODE_ANIM_INSTANT;
		GridModel.RangeReading rangeReading;		while ( (rangeReading = m_gridModel.update()) != null )
		{			m_mapRegion = m_mapRegion.union( rangeReading.boundary );
		}
		repaint();
		m_mode = MODE_ANIM;	}		public void updateLocalized()	{
		if ( DEBUG_TRACE ) System.err.println( "GridView.updateLocalized()" );		m_mode = MODE_ANIM_INSTANT;
		m_gridModel.updateLocalized( );
		m_mapRegion = new Rectangle( 250, 250, 0, 0 );		repaint();
		m_mode = MODE_ANIM;	}
		public boolean reset()	{		if ( DEBUG_TRACE ) System.err.println( "GridView.reset()" );		if ( !m_gridModel.reset() ) return false;
				m_mapRegion = new Rectangle( 250, 250, 0, 0 );
		repaint();		
		return true;	}		public void clear()	{
		if ( DEBUG_TRACE ) System.err.println( "GridView.clear()" );		m_gridModel.clear();
		m_mapRegion = new Rectangle( 250, 250, 0, 0 );
		repaint();	}		public void undo()	{
//		m_mapRegion = m_gridModel.undo().boundary;		m_gridModel.undo();
		repaint();	}

	public void paintComponent( Graphics g )
	{		if ( DEBUG_TRACE ) System.err.println( "GridView.paintComponent()" );		
		super.paintComponent( g );
		drawData( (Graphics2D)g );
	}
	
	void drawData( Graphics2D g2 )
	{		if ( DEBUG_TRACE ) System.err.println( "GridView.drawData()" );
		//////		// Get drawing area metrics				
		// Extract clipping region
		Rectangle clipRect = g2.getClipBounds();
		int clipX = (int)clipRect.getX();
		int clipY = (int)clipRect.getY();
		int width = (int)clipRect.getWidth();
		int height = (int)clipRect.getHeight();		
		// Validate clipping region is within bounds of map		int clientWidth = m_gridSize * m_cellSize;
		int clientHeight = m_gridSize * m_cellSize;
		if ( clipX > clientWidth || clipY > clientHeight ) {
			g2.clearRect( clipX, clipY, width, height );			return;		}
		if ( clipX + width > clientWidth ) width = clientWidth - clipX;
		if ( clipY + height > clientHeight ) height = clientHeight - clipY;		if ( width > clientWidth ) width = clientWidth;		if ( height > clientHeight ) height = clientHeight;

		// Convert to upper-left, lower-right coordinates, rounded to cell size
		int x1 = clipX / m_cellSize * m_cellSize;
		int y1 = (clipY / m_cellSize - 1) * m_cellSize;
		if ( y1 < 0 ) y1 = 0;		int x2 = Math.min( ((clipX + width) / m_cellSize + 1) * m_cellSize, m_gridSize * m_cellSize );
		int y2 = Math.min( ((clipY + height) / m_cellSize + 2) * m_cellSize, m_gridSize * m_cellSize );
				//////		// Draw a grid
				if ( m_drawGrid )		{
			g2.setColor( new Color( 0.9f, 0.9f, 0.9f ) ); //Color.lightGray ); //new Color( 0.90f, 0.90f, 0.90f ) );			g2.fillRect( clipX, clipY, (int)clipRect.getWidth(), (int)clipRect.getHeight() ); // intentionally				// re-retrieve width height since modified above
			g2.setColor( Color.lightGray );			int lineInterval = m_cellSize < 26 ? m_lineInterval : m_lineInterval / 5;
			for ( int x = x1 / m_lineInterval * m_lineInterval; x < x2; x += lineInterval ) { 				g2.drawLine( x, clipY, x, clipY + height );
			}			for ( int y = y1 / m_lineInterval * m_lineInterval; y < y2; y += lineInterval ) { 				g2.drawLine( clipX, y, clipX + width, y );			}		}
						//////
		// Draw the map		
		for ( int x = x1; x < x2; x += m_cellSize )
		{
			for ( int y = y1; y < y2; y += m_cellSize )
			{
				float cellData = m_gridModel.getCellValue( x / m_cellSize, y / m_cellSize );
//				if ( cellData != 0.5 )
//				{
					g2.setColor( Color.white ); //new Color( 0.99f, 0.99f, 0.99f ) );
										// Robot's path
					if ( cellData < 0 ) g2.setColor( Color.BLUE );
					//else g2.setColor( new Color( cellData, 0, 0, cellData ) );					
										// High probability of occupancy
					else if ( cellData > 0.95 ) 
						g2.setColor( new Color( 1.0f - cellData, 1.0f - cellData, 1.0f - cellData, 1.0f ) ); // black										// Medium probability of occupancy
					else if ( cellData > 0.7 ) g2.setColor( new Color( (float)(1.0 - cellData + 0.7), 0, 0, cellData ) ); // red										// Low probability of occupancy
					else if ( cellData != 0.5 ) g2.setColor( new Color( cellData / 2, (float)(0.7 - cellData + 0.3), 0, cellData ) ); // green-brown					
										// Color the cell
					if ( m_drawGrid ) {						g2.fillRect( x+1, y - m_cellSize + 1, m_cellSize - 1, m_cellSize - 1 ); 
							// y - m_cellSize causes cell to be drawn above line
					}
					else {						if ( cellData != 0.5 ) g2.fillRect( x+1, y - m_cellSize + 1, m_cellSize - 1, m_cellSize - 1 ); 						else g2.fillRect( x, y - m_cellSize, m_cellSize, m_cellSize ); 					}
										// Write in probability if zoom level high enough
					if ( m_cellSize >= 28 && cellData > 0 && cellData != 0.5 ) {
						java.text.DecimalFormat probFormat = new java.text.DecimalFormat();						probFormat.setMaximumFractionDigits( 2 );												probFormat.setMinimumFractionDigits( 2 );												String label = probFormat.format( cellData );
						int centerHorizOffset = m_cellSize / 2 - g2.getFontMetrics().stringWidth( label ) / 2;
						//int centerVertOffset = m_cellSize / 2 - g2.getFontMetrics().getHeight() / 2;
						g2.setColor( Color.black );						g2.drawString( label, x + centerHorizOffset, y /*- centerVertOffset*/ - 1 );					}
//				}
			}
		}		
		//////		// Label the axes		
		if ( m_drawAxis )
		{			int xMid = m_gridSize * m_cellSize / 2;			int yMid = xMid;			int fontWidth = g2.getFontMetrics().getMaxAdvance();
			int fontHeight = g2.getFontMetrics().getHeight();			int labelInterval = 
				m_cellSize < 4 ? m_lineInterval * 5 : 
				m_cellSize < 6 ? m_lineInterval * 2 : 
				m_cellSize < 10 ? m_lineInterval :				m_cellSize < 16 ? m_lineInterval / 2:								  m_lineInterval / 5;					// Y-Axis
			if ( x1 <= xMid && xMid - fontWidth * 4 < x2 ) 			{
				// Thick axis lines				g2.setColor( Color.black );
				g2.drawLine( xMid, clipY, xMid, clipY + height );
				g2.drawLine( xMid + 1, clipY, xMid + 1, clipY + height );								// Interval labels
				int yTop = clipY / labelInterval * labelInterval;				int yBottom = clipY + height + labelInterval + 1;
				for ( int y = yTop; y < yBottom - 1; y += labelInterval ) // -1 prevents label on last grid line
				{					int xCoord = xMid / m_cellSize - 1;
					float cellData = m_gridModel.getCellValue( xCoord, y / m_cellSize );					
					// Don't draw label if the cell is displaying a value already					if ( !(m_cellSize >= 28 && cellData > 0 && cellData != 0.5) ) {						String label = Integer.toString( (m_gridSize / 2) - (y / m_cellSize) );
						int rightJustifyOffset = g2.getFontMetrics().stringWidth( label );						g2.drawString( label, xMid - rightJustifyOffset - 1, y - 1 );					}
				}							}
		
			// X-Axis			if ( y1 < yMid + fontHeight && y2 > yMid ) 
			{
				// Thick axis lines				g2.setColor( Color.black );
				g2.drawLine( clipX, yMid, clipX + width, yMid );				g2.drawLine( clipX, yMid + 1, clipX + width, yMid + 1 );
								// Interval labels
				int xLeft = clipX / labelInterval * labelInterval; // get leftmost whole x in clipping region				int xRight = clipX + width + labelInterval + 1; // get rightmost whole x in clipping region
				for ( int x = xLeft; x < xRight - 1; x += labelInterval ) // -1 prevents label on last grid line
				{					String label = Integer.toString( (x / m_cellSize) - (m_gridSize / 2) );
					int rightJustifyOffset = g2.getFontMetrics().stringWidth( label );					g2.drawString( label, x /*- rightJustifyOffset - 1*/ + 2, yMid + fontHeight - 3 );
				}			}		}
	}		MouseInputAdapter installMouseAdapter()	{		return new MouseInputAdapter() 		{
			int m_dragStartX, m_dragStartY;				
			public void mouseClicked( MouseEvent e )
			{				if ( e.getClickCount() == 2 ) {
					scrollToPointCenter( e.getX(), e.getY() );				}
			}							public void mouseDragged( MouseEvent e )			{
				// Get current mouse position 				int dragStartX = e.getX();				int dragStartY = e.getY();									// Get distance from mouse position at last drag event
				int deltaX = m_dragStartX - dragStartX;
				int deltaY = m_dragStartY - dragStartY;
					
				// Choose dominating delta if shift key is pressed
				if ( (e.getModifiers() & InputEvent.SHIFT_MASK) != 0 ) {
					if ( Math.abs( deltaX ) > Math.abs( deltaY ) ) deltaY = 0;
					else deltaX = 0;				}					
				// Move current view by delta
				// Note: moving one direction at a time drastically improves speed of scroll
				// because the resulting invalidated rects are much smaller than when combined				Rectangle rect = m_gridView.getVisibleRect();
				int locX = (int)rect.getX();
				int locY = (int)rect.getY();				rect.setLocation( locX, locY + deltaY );
				m_gridView.scrollRectToVisible( rect );				rect.setLocation( locX + deltaX, locY + deltaY );
				m_gridView.scrollRectToVisible( rect );									// Reinitialize start of next drag operation (taking into account scroll delta)
				m_dragStartX = dragStartX + deltaX;				m_dragStartY = dragStartY + deltaY;			}
				public void mousePressed( MouseEvent e )			{
				// Initialize start of drag operation				m_dragStartX = e.getX();
				m_dragStartY = e.getY();
			}		};	}		public int zoomTo( int newSize )	{		if ( newSize < CELL_SIZE_MIN ) newSize = CELL_SIZE_MIN;
		else if ( newSize > CELL_SIZE_MAX ) newSize = CELL_SIZE_MAX;		
		// Save view center location
		Rectangle rect = getVisibleRect();		int centerX = (int)(rect.getX() + rect.getWidth() / 2) / m_cellSize;		int centerY = (int)(rect.getY() + rect.getHeight() / 2) / m_cellSize;
		m_cellSize = newSize;		m_lineInterval = LINED_GRID_CELL_INTERVAL * m_cellSize;
		setPreferredSize( new Dimension( m_gridSize * m_cellSize, m_gridSize * m_cellSize ) );		revalidate();
		
		// Restore original center point		scrollToPointCenter( centerX * m_cellSize, centerY * m_cellSize );				return newSize;
	}
		public int zoomIn( )	{
		return zoomTo( m_cellSize + 1 );
	}		public int zoomInMax()	{
		return zoomTo( CELL_SIZE_MAX );	}
		public int zoomOut( )	{		return zoomTo( m_cellSize - 1 );
	}
	
	public int zoomOutMax()
	{		return zoomTo( CELL_SIZE_MIN );
	}	
	public void scrollToOriginCorner()
	{
		if ( DEBUG_TRACE ) System.err.println( "GridView.scrollToOriginCorner()" );
				Rectangle rect = getVisibleRect();
		int width = (int)rect.getSize().getWidth();
		int height = (int)rect.getSize().getHeight();
				int m = m_gridSize * m_cellSize / 2;		int x = width > m ? m * 2 - width : m - m_lineInterval;		int y = height > m ? height - (m * 2) : m - (int)rect.getHeight() + m_lineInterval;
		rect.setLocation( x, y );
		scrollRectToVisible( rect );
	}
	
	public void scrollToMapCenter()	{		if ( DEBUG_TRACE ) System.err.println( "GridView.scrollToMapCenter()" );		// Get cell in center of map
		int centerX = (int)(m_mapRegion.getX() + m_mapRegion.getWidth() / 2) * m_cellSize;		int centerY = (int)(m_mapRegion.getY() + m_mapRegion.getHeight() / 2) * m_cellSize;
				scrollToPointCenter( centerX, centerY );
	}		public void scrollToRobotCenter()	{		scrollToPointCenter( m_robotPos.x * m_cellSize, m_robotPos.y * m_cellSize );	}
		public void scrollToPointCenter( int x, int y )	{		if ( DEBUG_TRACE ) System.err.println( "GridView.scrollToPointCenter()" );		// Get window size		Rectangle rect = getVisibleRect();
		int width = (int)rect.getWidth();
		int height = (int)rect.getHeight();				// Get rect surrounding map center		int xUL = x - width / 2;		int yUL = y - height / 2;		rect.setLocation( xUL, yUL );
		scrollRectToVisible( rect );		
	}
}
