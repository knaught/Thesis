/* GridView.java
 * Created on May 25, 2004
 */

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

public class GridView extends JPanel
{
	public static final int MODE_ANIM = 1; // retrieve range data from file at regular intervals
	public static final int CELL_SIZE = 4;
	public static final int CELL_SIZE_MIN = 2;
	public static final int CELL_SIZE_MAX = 30;
	public static final int LINED_GRID_CELL_INTERVAL = 10;

	private int m_mode;
	private Rectangle m_mapRegion;
	private GridModel.Cell m_robotPos;
	private boolean m_trackRobot = false;
	private boolean m_drawGrid = true;
	private boolean m_drawAxis = true;
	private int m_trackCounter = 0;
	
	GridView( GridModel gridModel )
	{
		if ( DEBUG_TRACE ) System.err.println( "GridView.GridView()" );
		m_gridModel = gridModel;
		
		setPreferredSize( new Dimension( m_gridSize * m_cellSize, m_gridSize * m_cellSize ) );
		setDoubleBuffered( true );
		setFocusable( true );
		MouseInputAdapter mouseAdapter = installMouseAdapter();
		
		repaint();
	
	public void init()
	{
		m_mapRegion = m_gridModel.getDimension();
		m_gridSize = m_gridModel.getGridSize();

		switch ( m_gridModel.getDataSource() ) {
			case GridModel.SOURCE_LIVE:
				scrollToOriginCorner();
				m_mode = MODE_STATIC;
				scrollToMapCenter();
				break;

	}
		if ( DEBUG_TRACE ) System.err.println( "GridView.getModel()" );
		return m_gridModel;
	
	public int getMode()
	{
	}
		m_mapRegion = region;
		m_drawGrid = visible;
		repaint();
	
	public boolean getGridVisible()
	{
		return m_drawGrid;
	}
	
	 * This is called by an AnimationToolBar AbstractAction each time the user steps through 
	 * the map.
	 */
	public GridModel.RangeReading update()
	{
		if ( DEBUG_TRACE ) System.err.println( "GridView.update(): mode " + m_mode );
		
		if ( m_mode == MODE_ANIM || m_mode == MODE_LIVE ) {
			{
				Rectangle gridRegion = rangeReading.boundary;
				Rectangle clientRegion = new Rectangle( 
					(int)gridRegion.getY() * m_cellSize - 1,
				m_robotPos.x = rangeReading.poseX + GridModel.GRID_SIZE / 2;
				m_robotPos.y = rangeReading.poseY + GridModel.GRID_SIZE / 2;
				
				if ( m_trackRobot && m_trackCounter++ % (1280 / m_cellSize) == 0 ) scrollToRobotCenter();
				return rangeReading;
		}
	}
		if ( DEBUG_TRACE ) System.err.println( "GridView.updateAll()" );
		GridModel.RangeReading rangeReading;
		{
		}
		repaint();
		m_mode = MODE_ANIM;
		if ( DEBUG_TRACE ) System.err.println( "GridView.updateLocalized()" );
		m_gridModel.updateLocalized( );
		m_mapRegion = new Rectangle( 250, 250, 0, 0 );
		m_mode = MODE_ANIM;
	
		
		repaint();
		return true;
		if ( DEBUG_TRACE ) System.err.println( "GridView.clear()" );
		m_mapRegion = new Rectangle( 250, 250, 0, 0 );
		repaint();
//		m_mapRegion = m_gridModel.undo().boundary;
		repaint();

	public void paintComponent( Graphics g )
	{
		super.paintComponent( g );
		drawData( (Graphics2D)g );
	}
	
	void drawData( Graphics2D g2 )
	{
		//////
		// Extract clipping region
		Rectangle clipRect = g2.getClipBounds();
		int clipX = (int)clipRect.getX();
		int clipY = (int)clipRect.getY();
		int width = (int)clipRect.getWidth();
		int height = (int)clipRect.getHeight();
		// Validate clipping region is within bounds of map
		int clientHeight = m_gridSize * m_cellSize;
		if ( clipX > clientWidth || clipY > clientHeight ) {
			g2.clearRect( clipX, clipY, width, height );
		if ( clipX + width > clientWidth ) width = clientWidth - clipX;
		if ( clipY + height > clientHeight ) height = clientHeight - clipY;

		// Convert to upper-left, lower-right coordinates, rounded to cell size
		int x1 = clipX / m_cellSize * m_cellSize;
		int y1 = (clipY / m_cellSize - 1) * m_cellSize;
		if ( y1 < 0 ) y1 = 0;
		int y2 = Math.min( ((clipY + height) / m_cellSize + 2) * m_cellSize, m_gridSize * m_cellSize );

		
			g2.setColor( new Color( 0.9f, 0.9f, 0.9f ) ); //Color.lightGray ); //new Color( 0.90f, 0.90f, 0.90f ) );
			g2.setColor( Color.lightGray );
			for ( int x = x1 / m_lineInterval * m_lineInterval; x < x2; x += lineInterval ) { 
			}
		
		// Draw the map
		for ( int x = x1; x < x2; x += m_cellSize )
		{
			for ( int y = y1; y < y2; y += m_cellSize )
			{
				float cellData = m_gridModel.getCellValue( x / m_cellSize, y / m_cellSize );
//				if ( cellData != 0.5 )
//				{
					g2.setColor( Color.white ); //new Color( 0.99f, 0.99f, 0.99f ) );
					
					if ( cellData < 0 ) g2.setColor( Color.BLUE );
					//else g2.setColor( new Color( cellData, 0, 0, cellData ) );					
					
					else if ( cellData > 0.95 ) 
						g2.setColor( new Color( 1.0f - cellData, 1.0f - cellData, 1.0f - cellData, 1.0f ) ); // black
					else if ( cellData > 0.7 ) g2.setColor( new Color( (float)(1.0 - cellData + 0.7), 0, 0, cellData ) ); // red
					else if ( cellData != 0.5 ) g2.setColor( new Color( cellData / 2, (float)(0.7 - cellData + 0.3), 0, cellData ) ); // green-brown
					
					if ( m_drawGrid ) {
							// y - m_cellSize causes cell to be drawn above line
					}
					else {
					
					if ( m_cellSize >= 28 && cellData > 0 && cellData != 0.5 ) {
						java.text.DecimalFormat probFormat = new java.text.DecimalFormat();
						int centerHorizOffset = m_cellSize / 2 - g2.getFontMetrics().stringWidth( label ) / 2;
						//int centerVertOffset = m_cellSize / 2 - g2.getFontMetrics().getHeight() / 2;
						g2.setColor( Color.black );
//				}
			}
		}
		//////
		if ( m_drawAxis )
		{
			int fontHeight = g2.getFontMetrics().getHeight();
				m_cellSize < 4 ? m_lineInterval * 5 : 
				m_cellSize < 6 ? m_lineInterval * 2 : 
				m_cellSize < 10 ? m_lineInterval :
			if ( x1 <= xMid && xMid - fontWidth * 4 < x2 ) 
				// Thick axis lines
				g2.drawLine( xMid, clipY, xMid, clipY + height );
				g2.drawLine( xMid + 1, clipY, xMid + 1, clipY + height );
				int yTop = clipY / labelInterval * labelInterval;
				for ( int y = yTop; y < yBottom - 1; y += labelInterval ) // -1 prevents label on last grid line
				{
					float cellData = m_gridModel.getCellValue( xCoord, y / m_cellSize );
					// Don't draw label if the cell is displaying a value already
						int rightJustifyOffset = g2.getFontMetrics().stringWidth( label );
				}
		
			// X-Axis
			{
				// Thick axis lines
				g2.drawLine( clipX, yMid, clipX + width, yMid );
				
				int xLeft = clipX / labelInterval * labelInterval; // get leftmost whole x in clipping region
				for ( int x = xLeft; x < xRight - 1; x += labelInterval ) // -1 prevents label on last grid line
				{
					int rightJustifyOffset = g2.getFontMetrics().stringWidth( label );
				}
	}
			int m_dragStartX, m_dragStartY;
			public void mouseClicked( MouseEvent e )
			{
					scrollToPointCenter( e.getX(), e.getY() );
			}
				// Get current mouse position 
				int deltaX = m_dragStartX - dragStartX;
				int deltaY = m_dragStartY - dragStartY;
					
				// Choose dominating delta if shift key is pressed
				if ( (e.getModifiers() & InputEvent.SHIFT_MASK) != 0 ) {
					if ( Math.abs( deltaX ) > Math.abs( deltaY ) ) deltaY = 0;
					else deltaX = 0;
				// Move current view by delta
				// Note: moving one direction at a time drastically improves speed of scroll
				// because the resulting invalidated rects are much smaller than when combined
				int locX = (int)rect.getX();
				int locY = (int)rect.getY();
				m_gridView.scrollRectToVisible( rect );
				m_gridView.scrollRectToVisible( rect );
				m_dragStartX = dragStartX + deltaX;
	
				// Initialize start of drag operation
				m_dragStartY = e.getY();
			}
		else if ( newSize > CELL_SIZE_MAX ) newSize = CELL_SIZE_MAX;
		// Save view center location
		Rectangle rect = getVisibleRect();

		setPreferredSize( new Dimension( m_gridSize * m_cellSize, m_gridSize * m_cellSize ) );
		
		// Restore original center point
	}
	
		return zoomTo( m_cellSize + 1 );
	}
		return zoomTo( CELL_SIZE_MAX );
	
	}
	
	public int zoomOutMax()
	{
	}
	public void scrollToOriginCorner()
	{
		if ( DEBUG_TRACE ) System.err.println( "GridView.scrollToOriginCorner()" );
		
		int width = (int)rect.getSize().getWidth();
		int height = (int)rect.getSize().getHeight();
		
		rect.setLocation( x, y );

	}
	
	public void scrollToMapCenter()
		int centerX = (int)(m_mapRegion.getX() + m_mapRegion.getWidth() / 2) * m_cellSize;
		
	}
	
		int width = (int)rect.getWidth();
		int height = (int)rect.getHeight();

	}
}