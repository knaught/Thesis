/* MapViewer.java
 * Created on May 25, 2004
 */

import javax.swing.*;
import java.awt.event.*;
import java.io.*;

public class MapViewer extends JFrame
{
	 * @param args[0] name of the sonar data file that is to be read from or written to.
	 * If the file exists, it is read from and the user controls the map rendering speed.
	 * If it doesn't exist, a live connection to the simulator or robot is attempted
	 * and if successful, live sonar data is written to it and simultaneously rendered
	 * at a constant speed.
	 */
	public static void main( String[] args )
	{
		String dataFilename = null;
		
			new MapViewer( null );
			File file = new File( args[0] );
				dataFilename = file.getCanonicalPath();
			}
			catch( IOException x ) {
			}
			new MapViewer( dataFilename );
			break;
		case 2:
			System.out.println( "Usage:  java MapViewer [filename|server controlPort dataPort]" );
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////
	
	public MapViewer( String serverName, int controlPort, int dataPort )
	{
		m_initialFilename = null;
		init();
	}
	
	public MapViewer( String filename )
	{
		super();
		try {
		}
		catch ( FileNotFoundException e ) {
			System.err.println( "MapViewer.MapViewer( String ): " + e );
		}
		m_initialFilename = filename;
		init();
	}
	{
		addWindowListener( new WindowAdapter() {
			public void windowClosed( WindowEvent e ) {
				m_gridModel.saveSettings();
			}
		setDefaultCloseOperation( DISPOSE_ON_CLOSE );
		Rectangle bounds = getGraphicsConfiguration().getBounds();
		setBounds( bounds.width/2 - w/2 + 90, // 30 accommodates prefs panel
				   bounds.height/2 - h/2, w, h );
		m_prefsDialog = new PreferencesPanel( this, m_gridView );
		m_prefsDialog.setLocation( 10, getBounds().y );
			
		JToolBar toolPanel = new JToolBar();
		toolPanel.add( m_fileToolBar = new FileToolBar( this ) );
		toolPanel.add( new ViewToolBar( this ) );
				
		JPanel statusPanel = new JPanel();
		statusPanel.setLayout( new BorderLayout() );
		statusPanel.add( m_controlPanel, BorderLayout.SOUTH );
			
		contentPane.setLayout( new BorderLayout() );
		contentPane.add( new JScrollPane( m_gridView ) );
		int viewMode = m_gridView.getMode();
		boolean anim = viewMode == GridView.MODE_ANIM || viewMode == GridView.MODE_LIVE;
		m_animationToolBar.setEnabled( anim ); 
			
		boolean saveEnabled = m_gridModel.getRangeDevice() == GridModel.SONAR_DEVICE;
		m_fileToolBar.setSaveEnabled( saveEnabled );
		m_menuPanel.setSaveEnabled( saveEnabled );
		//m_prefsDialog.show();
		setVisible( true ); // show();
		if ( m_gridView.getMode() == GridView.MODE_STATIC ) m_gridView.scrollToMapCenter();
		else m_gridView.scrollToOriginCorner();
	
	JDialog getPrefsDialog()
	{
		return m_prefsDialog;
	}
	
	GridView getView()
	{
	}
		return m_initialFilename;
	
	boolean openFile( String filename )
	{
		try {
				setTitle( ( filename == null ? "No data file loaded" : filename ) + " - AFIT RobotMapper" );
			}
			m_gridView.init();
			boolean anim = m_gridView.getMode() == GridView.MODE_ANIM;
			m_animationToolBar.setEnabled( anim ); 
			boolean saveEnabled = m_gridModel.getRangeDevice() == GridModel.SONAR_DEVICE;
			m_fileToolBar.setSaveEnabled( saveEnabled );
			m_menuPanel.setSaveEnabled( saveEnabled );
			System.err.println( "MapViewer.openFile(): " + e );
			return false;
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
	
	private StatusPanel m_controlPanel;
	private JDialog m_prefsDialog;
	private String m_initialFilename;
	private JToolBar m_animationToolBar;
	private MenuPanel m_menuPanel;