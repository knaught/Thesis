import java.awt.*;
import java.awt.event.*;
import java.io.*;
import javax.swing.event.*;
import javax.swing.filechooser.*;

class FileToolBar extends JToolBar
{
	public class DataFilter extends javax.swing.filechooser.FileFilter {

		String m_extensions[];
		String m_description;
		
		DataFilter( String extensions, String description )
		{
			m_extensions = extensions.split( " " );
			m_description = description;
		}
		
	    public boolean accept( File f ) 
		{
	        if ( f.isDirectory() ) return true;
	        String extension = getExtension( f );
			if ( extension == null ) return false;
			for ( int i = 0; i < m_extensions.length; ++i ) {
				if ( extension.equals( m_extensions[i] ) ) return true;
			}
			return false;
	    }

		public String getExtension( File f ) 
		{
		    String ext = null;
		    String s = f.getName();
		    int i = s.lastIndexOf('.');

		    if ( i > 0 &&  i < s.length() - 1 ) ext = s.substring(i+1).toLowerCase();
		    return ext;
		}

	    public String getDescription() {
	        return m_description;
	    }
	}

	FileToolBar( MapViewer viewer )
	{
		m_viewer = viewer;
		
		setFocusable( false );
		ActionMap actionMap = viewer.getView().getActionMap();
		installKeyBindings( viewer.getView() );
		
		add( new NoFocusButton( actionMap.get( "prefs" ) ) );
		add( m_saveButton = new NoFocusButton( actionMap.get( "saveMap" ) ) );
		add( new NoFocusButton( actionMap.get( "openFile" ) ) );
	}
		m_saveButton.setEnabled( enabled );
	
		final DataFilter sonarFilter = new DataFilter( "sd", "Sonar Data (*.sd)" );
		final DataFilter gridFilter = new DataFilter( "gd", "Grid Data (*.gd)" );
		final DataFilter coordFilter = new DataFilter( "cd", "Log Data (*.cd)" );
		final DataFilter allFilter = new DataFilter( "gd sd cd", "Sonar, Grid, or Coord Data (*.sd, *.gd, *.cd)" );
		
		final JFileChooser openChooser = new JFileChooser( m_viewer.getDataPath() );
		openChooser.addChoosableFileFilter( sonarFilter );
		openChooser.addChoosableFileFilter( gridFilter );
		openChooser.addChoosableFileFilter( coordFilter );
		openChooser.addChoosableFileFilter( allFilter );
		openChooser.setFileFilter( allFilter );
		openChooser.setAcceptAllFileFilterUsed( false );
		
		final JFileChooser saveChooser = new JFileChooser( m_viewer.getDataPath() );
		saveChooser.addChoosableFileFilter( gridFilter );
		saveChooser.setAcceptAllFileFilterUsed( false );
			
		AbstractAction saveMap = new AbstractAction( "Save Map" ) {
				GridView view = m_viewer.getView();
				if ( view.getMode() == GridView.MODE_STATIC ) {
					JOptionPane.showMessageDialog( null, "Saving a static map is not currently supported.", 
						"Save Static Map", JOptionPane.INFORMATION_MESSAGE );
				}
				else {
					if ( view.getModel().getRangeDevice() == GridModel.LOG_DEVICE ) {
						JOptionPane.showMessageDialog( null, "Saving a map built using coordinate log data is not currently supported.", 
							"Save Log Map", JOptionPane.INFORMATION_MESSAGE );
					}
					else if ( saveChooser.showSaveDialog( FileToolBar.this ) == JFileChooser.APPROVE_OPTION ) {
						File file = saveChooser.getSelectedFile();
						String fileName = file.toString();
						int extIndex = fileName.lastIndexOf( '.' );
						if ( extIndex != -1 ) fileName = fileName.substring( 0, extIndex );
						fileName += ".gd";
						view.getModel().saveMap( fileName );
					}
				}
			}
		saveMap.putValue( Action.ACTION_COMMAND_KEY, "saveMap" );
		AbstractAction openFile = new AbstractAction( "Open File" ) {
				int source = m_viewer.getView().getModel().getDataSource();
				if ( openChooser.showOpenDialog( FileToolBar.this ) == JFileChooser.APPROVE_OPTION ) {
					File file = openChooser.getSelectedFile();
					if ( file.exists() ) m_viewer.openFile( file.toString() );
				}
			}
		openFile.putValue( Action.ACTION_COMMAND_KEY, "openFile" );
		AbstractAction prefs = new AbstractAction( "Preferences" ) {
				m_viewer.getPrefsDialog().setVisible( true );
		prefs.putValue( Action.ACTION_COMMAND_KEY, "prefs" );
		
		InputMap inputMap = keyComponent.getInputMap();
	
	private NoFocusButton m_saveButton;
}