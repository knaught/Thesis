/* MapViewer.java
 * Created on May 25, 2004 * Last revised Sep 27, 2004
 */
import java.awt.*;
import javax.swing.*;
import java.awt.event.*;import javax.swing.event.*;
import java.io.*;

public class MapViewer extends JFrame
{	/**
	 * @param args[0] name of the sonar data file that is to be read from or written to.
	 * If the file exists, it is read from and the user controls the map rendering speed.
	 * If it doesn't exist, a live connection to the simulator or robot is attempted
	 * and if successful, live sonar data is written to it and simultaneously rendered
	 * at a constant speed.
	 */
	public static void main( String[] args )
	{
		String dataFilename = null;
				switch ( args.length ) {		case 0:
			new MapViewer( null );			break;		case 1:
			File file = new File( args[0] );			try {
				dataFilename = file.getCanonicalPath();
			}
			catch( IOException x ) {				System.err.println( "MapViewer.main(): " + x );				return;
			}
			new MapViewer( dataFilename );
			break;
		case 2:			new MapViewer( args[0], Integer.parseInt( args[1] ), Integer.parseInt( args[1] ) + 1 );			break;		default:
			System.out.println( "Usage:  java MapViewer [filename|server controlPort dataPort]" );		}
	}
		
	//////////////////////////////////////////////////////////////////////////////////////////////			public static String ImagePath = "..\\images\\";
	
	public MapViewer( String serverName, int controlPort, int dataPort )
	{		super();		m_gridModel = new GridModel( serverName, controlPort, dataPort );		setTitle( "Remote Client Viewer - AFIT RobotMapper" );
		m_initialFilename = null;
		init();
	}
	
	public MapViewer( String filename )
	{
		super();
		try {			m_gridModel = new GridModel( filename );
		}
		catch ( FileNotFoundException e ) {
			System.err.println( "MapViewer.MapViewer( String ): " + e );
		}		setTitle( ( filename == null ? "No data file loaded" : filename ) + " - AFIT RobotMapper" );
		m_initialFilename = filename;
		init();
	}		public void init()
	{
		addWindowListener( new WindowAdapter() {
			public void windowClosed( WindowEvent e ) {
				m_gridModel.saveSettings();				System.exit( 0 );
			}		} );
		setDefaultCloseOperation( DISPOSE_ON_CLOSE );				// Center the window on the screen		int w = 1024; // desired height/width of window		int h = 768;
		Rectangle bounds = getGraphicsConfiguration().getBounds();
		setBounds( bounds.width/2 - w/2 + 90, // 30 accommodates prefs panel
				   bounds.height/2 - h/2, w, h );	  		m_gridView = new GridView( m_gridModel );		
		m_prefsDialog = new PreferencesPanel( this, m_gridView );
		m_prefsDialog.setLocation( 10, getBounds().y );
			
		JToolBar toolPanel = new JToolBar();
		toolPanel.add( m_fileToolBar = new FileToolBar( this ) );
		toolPanel.add( new ViewToolBar( this ) );		toolPanel.add( m_animationToolBar = new AnimationToolBar( this ) );
				
		JPanel statusPanel = new JPanel();
		statusPanel.setLayout( new BorderLayout() );		statusPanel.add( toolPanel );		m_controlPanel = new StatusPanel( this );
		statusPanel.add( m_controlPanel, BorderLayout.SOUTH );
					Container contentPane = getContentPane();
		contentPane.setLayout( new BorderLayout() );		contentPane.add( m_menuPanel = new MenuPanel( this ), BorderLayout.NORTH );
		contentPane.add( new JScrollPane( m_gridView ) );		contentPane.add( statusPanel, BorderLayout.SOUTH );
		int viewMode = m_gridView.getMode();
		boolean anim = viewMode == GridView.MODE_ANIM || viewMode == GridView.MODE_LIVE;
		m_animationToolBar.setEnabled( anim ); 		m_menuPanel.getMenu( "Build" ).setEnabled( anim ); 
			
		boolean saveEnabled = m_gridModel.getRangeDevice() == GridModel.SONAR_DEVICE;
		m_fileToolBar.setSaveEnabled( saveEnabled );
		m_menuPanel.setSaveEnabled( saveEnabled );
		//m_prefsDialog.show();		m_gridView.requestFocusInWindow();
		setVisible( true ); // show();
		if ( m_gridView.getMode() == GridView.MODE_STATIC ) m_gridView.scrollToMapCenter();
		else m_gridView.scrollToOriginCorner();	}
	
	JDialog getPrefsDialog()
	{
		return m_prefsDialog;
	}
	
	GridView getView()
	{		return m_gridView;
	}		String getDataPath()	{
		return m_initialFilename;	}
	
	boolean openFile( String filename )
	{
		try {			m_gridView.reset();			if ( m_gridModel.initData( filename ) ) {
				setTitle( ( filename == null ? "No data file loaded" : filename ) + " - AFIT RobotMapper" );
			}
			m_gridView.init();
			boolean anim = m_gridView.getMode() == GridView.MODE_ANIM;
			m_animationToolBar.setEnabled( anim ); 			m_menuPanel.getMenu( "Build" ).setEnabled( anim ); 
			boolean saveEnabled = m_gridModel.getRangeDevice() == GridModel.SONAR_DEVICE;
			m_fileToolBar.setSaveEnabled( saveEnabled );
			m_menuPanel.setSaveEnabled( saveEnabled );		}		catch ( Exception e ) {
			System.err.println( "MapViewer.openFile(): " + e );
			return false;		}		return true;
	}	

    /** Returns an ImageIcon, or null if the path was invalid. */
    static ImageIcon createImageIcon( String path ) {
        java.net.URL imgURL = AnimationToolBar.class.getResource( path );
		if ( imgURL != null ) {
            return new ImageIcon( imgURL );
        } else {
			System.err.println( "MapViewer.createImageIcon(): Couldn't find file " + path );
            return null;
        }
    }
		private GridModel m_gridModel;
	private StatusPanel m_controlPanel;	private GridView m_gridView;
	private JDialog m_prefsDialog;
	private String m_initialFilename;
	private JToolBar m_animationToolBar;	private FileToolBar m_fileToolBar;
	private MenuPanel m_menuPanel;}
