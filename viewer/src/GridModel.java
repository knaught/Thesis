import java.io.*;

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
	native void setSonarModel( int sonarModel );
	native void setSonarEnabled( int sonarNumber, boolean enabled );
	native int setSonarBeta( int beta );
	native float getMaxOccupied();
	native float setMaxOccupied( float maxOcc );
	native float getMaxOccupiedII();
	native float setMaxOccupiedII( float maxOcc );
	native int getRegionIWidth();
	native int setRegionIWidth( int width );
	native void setIgnoreObstructed( boolean ignore );
	
	native int getCellSize();
	native int setCellSize( int size );
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
	native float getMotionGaussSigma();
	native void runMapper();
	native String stepSonarMapper();

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
	public static final int SONAR_DEVICE = 2;
	public static final int SOURCE_LIVE = 1; // in which case data file is written to
	public static final int SOURCE_RECORDED = 2; // in which case data file is read from
	public static final int SOURCE_STATIC = 3; // not range data, but grid data file
	public static final int SONAR_MODEL_CELL = 0;
	public static final int SONAR_MODEL_AXIS = 1;
	public static final int SONAR_MODEL_CONE = 2;
	public static final int MAX_UNDO_HISTORY = 160;
		
	
	private int m_rangeDevice;
	private int m_gridSize;
	private float m_gridData[][];
	private Vector m_changeListeners;
	private Stack m_redoHistory; // accumulates update strings
	private Stack m_undoHistory; // accumulates vector of grid upates for each update string
	private String m_filename;
	
	private DatagramSocket m_dataSocket;
	private DatagramPacket m_packet;
	GridModel( String dataFilename ) throws FileNotFoundException
	{
	}
	
	GridModel( String hostName, int controlPort, int dataPort )
	{
			m_dataSocket = new DatagramSocket();
			m_packet = new DatagramPacket( m_datagramBuffer, m_datagramBuffer.length );
		
			String handshake = new String( "Hello" );		
			m_dataSocket.send( packet );
			    
			// get/display response
			m_dataSocket.receive( m_packet );
			String reply = new String( m_packet.getData(), 0, m_packet.getLength() );
			System.out.println( "Remote View Server says: " + reply );
		
			m_rangeDevice = DATAGRAM_DEVICE;
		}
		catch( IOException e ) {
		}
	
	void init( String dataFilename )
	{
		// data filename extensions
		// gd: static grid data
		// sd: recorded or live sonar range data
		// cd: recorded or live camera range data
		
		m_changeListeners = new Vector();
		try {
		}
		catch( FileNotFoundException e ) {
		}
	
	String getFilename()
	{
		return m_filename;
	}
		return m_rangeDevice;
	
	public int getGridSize()
	{
		return m_gridSize;
	}
	
	public float getCellValue( int x, int y )
	{
		return m_gridData[x][y];
	}
	
	 * This is called by GridView.update() in response to a user-requested or animated step.
	 */
	public RangeReading update( )
	{
		// Catch redo here
		if ( m_redoHistory.size() > 0 ) {
			return update( (RangeReading)m_redoHistory.pop() );
		}
			case LOG_DEVICE: return update( stepLogMapper() );
			case DATAGRAM_DEVICE: return update( datagramUpdate() );
		}
		
	}
		}
		catch( IOException e ) {
			System.err.println( "GridModel.datagramUpdate(): " + e );
			return "";
		}
		
		String reply = new String( m_packet.getData(), 0, m_packet.getLength() );
		if ( reply.compareTo( "quit" ) == 0 ) System.exit(0);
	}
		
	 * This is called by GridView.updateLocalized() in response to user-requested localization.
	 */
	public void updateLocalized( )
	{
		System.err.println( "GridModel.updateLocalized() is no longer supported." );
		// Redraw localized maps in order of generation
		
		// Integrate (overlap) local maps
	}
	
	
	public RangeReading update( RangeReading rangeReading )
	{
		for ( int i = 0; i < rangeReading.fill.length; ++i ) {
			undoData.fill[i] = new Cell( c.x, c.y, m_gridData[c.x][c.y] );
		}
		m_undoHistory.push( undoData );
		
	}
	
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
	{
		if ( logEntry == null || logEntry.length() == 0 ) return null;
		
		UndoData undoData = new UndoData();
		try 
		{
			if ( logData.length == 0 || logData.length > 2 ) {
				System.err.println( "GridModel.update( String ): Invalid log data length of " + logData.length
					+ " for log entry '" + logEntry.substring( 0, logEntry.length() < 100 ? logEntry.length() : 100 ) + "'" );
			
				System.err.println( "GridModel.update( String ): Invalid pose data length of " + logData.length +
									" in log entry: \n" + logEntry );
			rangeReading.poseX = GRID_MID + Integer.parseInt( poseData[0] );
			rangeReading.poseY = GRID_MID - Integer.parseInt( poseData[1] );
			// Mark robot path
			if ( rangeReading.poseX < 0 || rangeReading.poseX >= GRID_SIZE ||
				 rangeReading.poseY < 0 || rangeReading.poseY >= GRID_SIZE ) {
				return rangeReading;
			}
			float undoPathValue = m_gridData[rangeReading.poseX][rangeReading.poseY];
			
			// Initialize the clipping region surrounding the grid update
			rangeReading.boundary = new Rectangle( rangeReading.poseX, rangeReading.poseY - 1, 1, 1 );
				
			if ( logData.length == 2 ) regionData = logData[1].split( ";" );
			
			rangeReading.fill = new Cell[regionData.length + 1];
			undoData.fill = new Cell[regionData.length + 1];
			{
				String pointData[] = regionData[i].split( " " );
				cell.x = GRID_MID + Integer.parseInt( pointData[0] );
				cell.y = GRID_MID - Integer.parseInt( pointData[1] );
				// Skip out of bounds
						
				undoData.fill[i] = new Cell( cell.x, cell.y, m_gridData[cell.x][cell.y] );
						
				if ( m_gridData[cell.x][cell.y] != -1 ) {
					m_gridData[cell.x][cell.y] = cell.pr;
						
				rangeReading.boundary.add( cell.x, cell.y - 1 );
			undoData.fill[regionData.length] = 
				new Cell( rangeReading.poseX, rangeReading.poseY, undoPathValue );
		}
		}
		catch( Exception e )
		{
			System.err.println( "GridModel.update( String ): " + e + "\nLog entry:\n" + logEntry + "\n" );
		}
		rangeReading.poseX -= GRID_SIZE / 2;
		
		m_undoHistory.push( undoData );
		
		if ( assertFailed ) return null;
	public boolean reset()
		// Clear grid
		empty();
		return true;
		
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

		// But may also be static if given grid data
			if ( !fileExists ) {
				throw new FileNotFoundException( 
					"GridModel.GridModel(): Cannot find file " + dataFilename + "." );
			m_dataSource = SOURCE_STATIC;
			return true;
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
		
		return true;
	
	public void clear()
	{
		m_gridData = new float[m_gridSize][m_gridSize];
		for ( int i = 0; i < m_gridSize; ++i ) {
			for ( int j = 0; j < m_gridSize; ++j ) m_gridData[i][j] = 0.5f;
		}
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
		
			m_gridData[c.x][c.y] = c.pr; 
		m_redoHistory.push( undo.rangeReading );
		
	}
	{
	}
	
	
		ChangeEvent event = new ChangeEvent( rangeReading );
			((ChangeListener)i.next()).changePerformed( event );
	}
		{
			BufferedReader reader = new BufferedReader( new FileReader( filename ) );
			String bounds = getNextUncommentedLine( reader );
			String bound[] = bounds.replaceAll( "[(),]", "" ).split( " " );
			int mid = GRID_SIZE / 2;
			if ( ulx < 0 || uly < 0 || lrx > GRID_SIZE-1 || lry > GRID_SIZE-1 ) {
				throw new Exception( "Invalid bounds specification.  Bounds exceed maximum grid size." );
			}
			m_gridDimension = new Rectangle( ulx, uly, lrx - ulx, lry - uly );
			// Read grid data
			// Note: m_gridData must be pre-initialized!
			for ( int i = uly; i <= lry && (row = getNextUncommentedLine( reader )) != null; ++i )
			{
				// Split column data into an array
				String data[] = row.split( " " );
				
				// Store data in grid
				for ( int j = ulx; j <= lrx; ++j )
				{
					try {
						m_gridData[j][i] = Float.parseFloat( data[j - ulx] );
					catch( NumberFormatException e ) {
						m_gridData[j][i] = -1;
						System.err.println( "GridModel.loadStaticData() data[" + i + "][" + j + "] = " + 
					}
				}
		}
		catch( Exception e ) {
			System.err.println( "GridModel.loadStaticData(): " + e );
		}
	}
	
	String getNextUncommentedLine( BufferedReader reader )
	{
		String line;
				line = reader.readLine();
			} catch ( IOException e ) {
				return null;
			}
		} while ( line.charAt( 0 ) == '%' );
		
	}
		// Save old value
		m_gridData[c.x][c.y] = -1;
}