import java.io.*;import java.awt.Rectangle;import java.awt.Point;import java.util.*;import java.net.*;

class GridModel
{
	//////
	// Hook into MapperDll.lib
	// Native methods are implemented in JniListener.cpp
	
	static {
        System.loadLibrary("Mapper");
    }
	native void setMapUpdateListener( MapUpdateListener listener );
	native int openLiveConnection( String outFilename, boolean wander );
	native int openFileConnection( String inFilename );
	native int resetFileConnection();	
	native void setSonarModel( int sonarModel );	native int getSonarModel();
	native void setSonarEnabled( int sonarNumber, boolean enabled );	native int getSonarBeta();
	native int setSonarBeta( int beta );
	native float getMaxOccupied();
	native float setMaxOccupied( float maxOcc );
	native float getMaxOccupiedII();
	native float setMaxOccupiedII( float maxOcc );
	native int getRegionIWidth();
	native int setRegionIWidth( int width );
	native void setIgnoreObstructed( boolean ignore );	native boolean getIgnoreObstructed();	native void setIgnoreOutOfRange( boolean ignore );	native boolean getIgnoreOutOfRange();	native int getOutOfRangeConversion();	native int setOutOfRangeConversion( int r );
	
	native int getCellSize();
	native int setCellSize( int size );	native int getLocalMapSize();
	native int setLocalMapSize( int size );
	native boolean getLocalizationEnabled();
	native void setLocalizationEnabled( boolean enabled );
	native float getObstructedCertainty();
	native float setObstructedCertainty( float certainty );
	native int getMotionMinHeight();
	native int setMotionMinHeight( int height );
	native int getMotionMinWidth();
	native int setMotionMinWidth( int width );
	native int getMotionUnitDistance();
	native int setMotionUnitDistance( int distance );
	native int getMotionUnitTurn();
	native int setMotionUnitTurn( int turn );
	native float getMotionGaussSigma();	native float setMotionGaussSigma( float sigma );	native float getMotionBendFactor();	native float setMotionBendFactor( float factor );	
	native void runMapper();
	native String stepSonarMapper();	native String stepLogMapper();
	native String localize();
	native String integrate();
	native void clearMap();
	native void emptyMap();
	native int saveMap( String filename );
	
	public native void saveSettings();
	
	interface MapUpdateListener
	{
		public void mapUpdateFired( String event );
	}
	
	class MapUpdateThread extends Thread implements MapUpdateListener
	{
		private boolean m_connected = false;
		
		MapUpdateThread( String dataFilename, boolean wander )
		{
			// Let us know as each sonar reading is processed
			setMapUpdateListener( this );
			
			// Open the sonar file for writing
			m_connected = openLiveConnection( dataFilename, wander ) == 0;
		}
		
		public void run() 
		{
			if ( !m_connected ) {
				System.err.println( "MapUpdateThread.run(): No connection has been established." );
				return;
			}
			else System.err.println( "MapUpdateThread.run(): Connection established." );
			
			// Engage simulator/robot feed
			runMapper();
		}
		
		public boolean hasConnection()
		{
			return m_connected;
		}

		public void mapUpdateFired( String event )
		{
			// Pass the event on for model and view updates
			update( event );
		}
	}
	
	//
	//////
	
	public static class Cell
	{
		public int x, y;
		public float pr;
		
		Cell()
		{
			x = y = 0;
			pr = 0.0f;
		}
		
		Cell( int x_, int y_, float pr_ )
		{
			x = x_;
			y = y_;
			pr = pr_;
		}
	}
	
	public static class RangeReading
	{
		public int deviceId, range;
		public int poseX, poseY, poseTh;
		public Cell[] fill;
		public Rectangle boundary;
	}
	
	public static class ChangeEvent
	{
		RangeReading m_rangeReading;
		
		public ChangeEvent( RangeReading rangeReading )
		{
			m_rangeReading = rangeReading;
		}
		
		public RangeReading getRangeReading()
		{
			return m_rangeReading;
		}
	}

	public interface ChangeListener
	{
		public void changePerformed( ChangeEvent event );
	}
	
	public static class UndoData
	{
		public RangeReading rangeReading;
		public Cell[] fill;
	}

	public static final int NO_DEVICE = 0;
	public static final int STATIC_DEVICE = 1;
	public static final int SONAR_DEVICE = 2;	public static final int LOG_DEVICE = 3;	public static final int DATAGRAM_DEVICE = 4;	public static final int GRID_SIZE = 2000;	public static final int GRID_MID = 2000 / 2;	public static final int SOURCE_NONE = 0; // no data file
	public static final int SOURCE_LIVE = 1; // in which case data file is written to
	public static final int SOURCE_RECORDED = 2; // in which case data file is read from
	public static final int SOURCE_STATIC = 3; // not range data, but grid data file
	public static final int SONAR_MODEL_CELL = 0;
	public static final int SONAR_MODEL_AXIS = 1;
	public static final int SONAR_MODEL_CONE = 2;
	public static final int MAX_UNDO_HISTORY = 160;
		
	
	private int m_rangeDevice;
	private int m_gridSize;	private Rectangle m_gridDimension;
	private float m_gridData[][];
	private Vector m_changeListeners;	private MapUpdateThread m_mapUpdateThread;	private int m_dataSource;
	private Stack m_redoHistory; // accumulates update strings
	private Stack m_undoHistory; // accumulates vector of grid upates for each update string
	private String m_filename;
	
	private DatagramSocket m_dataSocket;
	private DatagramPacket m_packet;	private byte m_datagramBuffer[] = new byte[4096];	
	GridModel( String dataFilename ) throws FileNotFoundException
	{		init( dataFilename );
	}
	
	GridModel( String hostName, int controlPort, int dataPort )
	{		init( null );				try {
			m_dataSocket = new DatagramSocket();
			m_packet = new DatagramPacket( m_datagramBuffer, m_datagramBuffer.length );
					// Initiate handshake			System.out.println( "Sending greeting to host " + hostName + " on port " + dataPort );
			String handshake = new String( "Hello" );					InetAddress address = InetAddress.getByName( hostName );			DatagramPacket packet = new DatagramPacket( handshake.getBytes(), handshake.length(), address, dataPort );
			m_dataSocket.send( packet );
			    
			// get/display response
			m_dataSocket.receive( m_packet );
			String reply = new String( m_packet.getData(), 0, m_packet.getLength() );
			System.out.println( "Remote View Server says: " + reply );
		
			m_rangeDevice = DATAGRAM_DEVICE;			m_dataSource = SOURCE_LIVE;
		}
		catch( IOException e ) {			System.err.println( "GridModel.GridModel( String, int ): " + e );			m_rangeDevice = NO_DEVICE;
		}	}
	
	void init( String dataFilename )
	{
		// data filename extensions
		// gd: static grid data
		// sd: recorded or live sonar range data
		// cd: recorded or live camera range data
				m_gridSize = GRID_SIZE;		m_gridDimension = new Rectangle();
		m_changeListeners = new Vector();		m_undoHistory = new Stack();		m_redoHistory = new Stack();		m_filename = dataFilename;		
		try {			initData( dataFilename );
		}
		catch( FileNotFoundException e ) {			System.err.println( "GridModel.init(): " + e );
		}	}		public int getDataSource()	{		return m_dataSource;	}
	
	String getFilename()
	{
		return m_filename;
	}		public int getRangeDevice()	{
		return m_rangeDevice;	}
	
	public int getGridSize()
	{
		return m_gridSize;
	}		public Rectangle getDimension()	{		return m_gridDimension;	}
	
	public float getCellValue( int x, int y )
	{		if ( x < 0 || x > m_gridSize || y < 0 || y > m_gridSize ) {			System.err.println( "InvalidCellCoordinate in GridModel.getCellValue(): (" + x + "," + y + ")" );			return 0;		}		
		return m_gridData[x][y];
	}
		/**
	 * This is called by GridView.update() in response to a user-requested or animated step.
	 */
	public RangeReading update( )
	{
		// Catch redo here
		if ( m_redoHistory.size() > 0 ) {
			return update( (RangeReading)m_redoHistory.pop() );
		}				switch ( m_rangeDevice ) {			case SONAR_DEVICE: return update( stepSonarMapper() );
			case LOG_DEVICE: return update( stepLogMapper() );
			case DATAGRAM_DEVICE: return update( datagramUpdate() );
		}
				return null;
	}		public String datagramUpdate()	{		try {			m_dataSocket.receive( m_packet );
		}
		catch( IOException e ) {
			System.err.println( "GridModel.datagramUpdate(): " + e );
			return "";
		}
		
		String reply = new String( m_packet.getData(), 0, m_packet.getLength() );//		System.err.println( "GridModel.datagramUpdate(): " + reply );		// Terminate viewer if user requested Quit
		if ( reply.compareTo( "quit" ) == 0 ) System.exit(0);		return reply.compareTo( "reset" ) == 0 ? "" : reply;
	}
			/**
	 * This is called by GridView.updateLocalized() in response to user-requested localization.
	 */
	public void updateLocalized( )
	{
		System.err.println( "GridModel.updateLocalized() is no longer supported." );		/*
		// Redraw localized maps in order of generation		String logEntries = localize( );		String logEntry[] = logEntries.split( "\n\n" );		for ( int i = 0; i < logEntry.length; ++i ) {			update( logEntry[i] );		}
		
		// Integrate (overlap) local maps		update( integrate() );		*/
	}
	
	
	public RangeReading update( RangeReading rangeReading )
	{		UndoData undoData = new UndoData();		undoData.rangeReading = rangeReading;		undoData.fill = new Cell[rangeReading.fill.length];
		for ( int i = 0; i < rangeReading.fill.length; ++i ) {			Cell c = rangeReading.fill[i];
			undoData.fill[i] = new Cell( c.x, c.y, m_gridData[c.x][c.y] );			m_gridData[c.x][c.y] = c.pr;
		}
		m_undoHistory.push( undoData );
				return rangeReading;
	}
			/**
	 * Called by update() and MapUpdateThread.mapUpdateFired().
	 * 
	 * A data set consists of three elements, each terminated by a newline:
	 * <ol>
	 *   <li>PoseX PoseY PoseTh DeviceId Range
	 *   <li>Series of seven region boundary coordinate pairs, in alphabitical order
	 *       Region I is defined by B-C-D-E-F-G.
	 *       Region II is defined by A-B-C-D. 
	 *   <li>Series of coordinate-probability pairs that reflect the changes made to the grid
	 * </ol>
	 */
	public RangeReading update( String logEntry )
	{//		System.err.println( "GridModel.update( String ) : " + logEntry );				boolean assertFailed = false; // DEBUG		
		if ( logEntry == null || logEntry.length() == 0 ) return null;
		
		UndoData undoData = new UndoData();		RangeReading rangeReading = new RangeReading();
		try 
		{			String logData[] = logEntry.split( "\n" );
			if ( logData.length == 0 || logData.length > 2 ) {
				System.err.println( "GridModel.update( String ): Invalid log data length of " + logData.length
					+ " for log entry '" + logEntry.substring( 0, logEntry.length() < 100 ? logEntry.length() : 100 ) + "'" );				return null;			}
						//////			// Robot pose							String poseData[] = logData[0].split( " " );			if ( poseData.length != 5 ) {
				System.err.println( "GridModel.update( String ): Invalid pose data length of " + logData.length +
									" in log entry: \n" + logEntry );				return null;			}
			rangeReading.poseX = GRID_MID + Integer.parseInt( poseData[0] );
			rangeReading.poseY = GRID_MID - Integer.parseInt( poseData[1] );			rangeReading.poseTh = Integer.parseInt( poseData[2] );			rangeReading.deviceId = Integer.parseInt( poseData[3] );			rangeReading.range = Integer.parseInt( poseData[4] );				
			// Mark robot path
			if ( rangeReading.poseX < 0 || rangeReading.poseX >= GRID_SIZE ||
				 rangeReading.poseY < 0 || rangeReading.poseY >= GRID_SIZE ) {				System.err.println( "GridModel.update( String ): Pose out of bounds\n" + 					logEntry.substring( 0, logEntry.indexOf( '\n' ) ) );
				return rangeReading;
			}
			float undoPathValue = m_gridData[rangeReading.poseX][rangeReading.poseY];			m_gridData[rangeReading.poseX][rangeReading.poseY] = -1;
			
			// Initialize the clipping region surrounding the grid update
			rangeReading.boundary = new Rectangle( rangeReading.poseX, rangeReading.poseY - 1, 1, 1 );
											//////			// Region fills			String regionData[] = {};
			if ( logData.length == 2 ) regionData = logData[1].split( ";" );
			
			rangeReading.fill = new Cell[regionData.length + 1];
			undoData.fill = new Cell[regionData.length + 1];			for ( int i = 0; i < regionData.length; ++i )
			{
				String pointData[] = regionData[i].split( " " );				Cell cell = rangeReading.fill[i] = new Cell();
				cell.x = GRID_MID + Integer.parseInt( pointData[0] );
				cell.y = GRID_MID - Integer.parseInt( pointData[1] );				cell.pr = Float.parseFloat( pointData[2] );				
				// Skip out of bounds				if ( cell.x < 0 || cell.x > GridModel.GRID_SIZE ||					 cell.y < 0 || cell.y > GridModel.GRID_SIZE ) continue;
						
				undoData.fill[i] = new Cell( cell.x, cell.y, m_gridData[cell.x][cell.y] );
										// Don't overwrite a robot path cell
				if ( m_gridData[cell.x][cell.y] != -1 ) {
					m_gridData[cell.x][cell.y] = cell.pr;				}
										// Grow the clipping region surrounding fill
				rangeReading.boundary.add( cell.x, cell.y - 1 );			}						// Include robot path as well
			undoData.fill[regionData.length] = 
				new Cell( rangeReading.poseX, rangeReading.poseY, undoPathValue );			rangeReading.fill[regionData.length] = 				new Cell( rangeReading.poseX, rangeReading.poseY, -1 );
		}		catch( NumberFormatException e )		{			System.err.println( "GridModel.update( String ): " + e + "\nLog entry:\n" + logEntry + "\n" );			// Ignore and proceed
		}
		catch( Exception e )
		{
			System.err.println( "GridModel.update( String ): " + e + "\nLog entry:\n" + logEntry + "\n" );			return null;
		}		
		rangeReading.poseX -= GRID_SIZE / 2;		rangeReading.poseY -= GRID_SIZE / 2;
				fireChangeEvent( rangeReading );				undoData.rangeReading = rangeReading;
		m_undoHistory.push( undoData );		if ( m_undoHistory.size() > MAX_UNDO_HISTORY ) m_undoHistory.removeElementAt( 0 );
		
		if ( assertFailed ) return null;				return rangeReading;	}	
	public boolean reset()	{
		// Clear grid		if ( m_dataSource == SOURCE_RECORDED && resetFileConnection() != 0 ) return false;
		empty();
		return true;	}		public boolean initData( String dataFilename ) throws FileNotFoundException	{		reset();
				// Determine whether data file exists
		boolean fileExists = false;
		if ( dataFilename == null ) {
			m_dataSource = SOURCE_NONE;
			m_rangeDevice = NO_DEVICE;
			return true;
		}
		else {
			File dataFile = new File( dataFilename );
			fileExists = dataFile.exists();
		}
		
		// Initially, data source is recorded or live
		m_dataSource = fileExists ? SOURCE_RECORDED : SOURCE_LIVE;

		// But may also be static if given grid data		if ( dataFilename.endsWith( ".gd" ) ) {
			if ( !fileExists ) {
				throw new FileNotFoundException( 
					"GridModel.GridModel(): Cannot find file " + dataFilename + "." );			}
			m_dataSource = SOURCE_STATIC;			m_rangeDevice = STATIC_DEVICE;			loadStaticData( dataFilename );
			return true;		}
		else if ( dataFilename.endsWith( ".sd" ) ) m_rangeDevice = SONAR_DEVICE;
		else if ( dataFilename.endsWith( ".cd" ) ) m_rangeDevice = LOG_DEVICE;		
		// If the data file doesn't exist, assume is to be created from a live data feed.
		// This file creation is accomplished by the native library, which also notifies any 
		// registered listeners of each range reading as it is processed.
		if ( m_dataSource == SOURCE_RECORDED ) {
			if ( openFileConnection( dataFilename ) != 0 ) {
				throw new FileNotFoundException( 
					"GridModel.GridModel(): Native library unable to open file " + dataFilename + "." );
			}
		}
		else if ( m_dataSource == SOURCE_LIVE ) {
			m_mapUpdateThread = new MapUpdateThread( dataFilename, false );
			if ( m_mapUpdateThread.hasConnection() ) m_mapUpdateThread.start();
			else System.err.println( "GridModel.GridModel(): Unable to establish a connection with the simulator or robot." );
			m_dataSource = SOURCE_LIVE;
		}
		
		return true;	}
	
	public void clear()
	{		clearMap();
		m_gridData = new float[m_gridSize][m_gridSize];
		for ( int i = 0; i < m_gridSize; ++i ) {
			for ( int j = 0; j < m_gridSize; ++j ) m_gridData[i][j] = 0.5f;
		}		m_undoHistory.removeAllElements();
		fireChangeEvent( new RangeReading() );
	}
	
	public void empty()
	{
		clear();
		m_redoHistory.removeAllElements();
		emptyMap();
	}
	
	public RangeReading undo()
	{
		if ( m_undoHistory.size() == 0 ) return null;
				UndoData undo = (UndoData)m_undoHistory.pop();		for ( int i = 0; i < undo.fill.length; ++i ) {			Cell c = undo.fill[i];
			m_gridData[c.x][c.y] = c.pr; 		}
		m_redoHistory.push( undo.rangeReading );
				return undo.rangeReading;
	}		public void setSonar( int sonarNumber, boolean enabled )
	{		setSonarEnabled( sonarNumber, enabled );
	}
		public void addChangeListener( ChangeListener listener )	{		if ( !m_changeListeners.contains( listener ) ) m_changeListeners.add( listener );	}		public void removeChangeListener( ChangeListener listener )	{		m_changeListeners.removeElement( listener );	}
		void fireChangeEvent( GridModel.RangeReading rangeReading )	{
		ChangeEvent event = new ChangeEvent( rangeReading );		Iterator i = m_changeListeners.iterator();		while ( i.hasNext() ) {
			((ChangeListener)i.next()).changePerformed( event );		}
	}		void loadStaticData( String filename )	{		try
		{
			BufferedReader reader = new BufferedReader( new FileReader( filename ) );						// Get grid dimensions
			String bounds = getNextUncommentedLine( reader );			if ( bounds == null ) throw new IOException( "Unable to read from " + filename );
			String bound[] = bounds.replaceAll( "[(),]", "" ).split( " " );			if ( bound.length != 4 ) throw new Exception( "Invalid bounds specification" );
			int mid = GRID_SIZE / 2;			int ulx = mid + Integer.parseInt( bound[0] );			int uly = mid - Integer.parseInt( bound[1] );			int lrx = mid + Integer.parseInt( bound[2] );			int lry = mid - Integer.parseInt( bound[3] );
			if ( ulx < 0 || uly < 0 || lrx > GRID_SIZE-1 || lry > GRID_SIZE-1 ) {
				throw new Exception( "Invalid bounds specification.  Bounds exceed maximum grid size." );
			}
			m_gridDimension = new Rectangle( ulx, uly, lrx - ulx, lry - uly );			
			// Read grid data
			// Note: m_gridData must be pre-initialized!			String row = new String();
			for ( int i = uly; i <= lry && (row = getNextUncommentedLine( reader )) != null; ++i )
			{
				// Split column data into an array
				String data[] = row.split( " " );
				
				// Store data in grid
				for ( int j = ulx; j <= lrx; ++j )
				{
					try {
						m_gridData[j][i] = Float.parseFloat( data[j - ulx] );					}
					catch( NumberFormatException e ) {
						m_gridData[j][i] = -1;
						System.err.println( "GridModel.loadStaticData() data[" + i + "][" + j + "] = " + 							data[j - ulx] + ": " + e );
					}
				}			}
		}
		catch( Exception e ) {
			System.err.println( "GridModel.loadStaticData(): " + e );
		}
	}
	
	String getNextUncommentedLine( BufferedReader reader )
	{
		String line;		do {			try {
				line = reader.readLine();
			} catch ( IOException e ) {				System.err.println( "GridModel.getNextUncommentedLine(): " + e );
				return null;
			}			if ( line == null ) return null;
		} while ( line.charAt( 0 ) == '%' );
				return line;
	}		void addUndoable( Stack s, Cell c )	{
		// Save old value		s.push( new Cell( c.x, c.y, m_gridData[c.x][c.y] ) );		// Write new value
		m_gridData[c.x][c.y] = -1;	}
}