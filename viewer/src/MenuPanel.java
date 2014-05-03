import javax.swing.*;
import java.awt.event.*;
import java.util.*;

class MenuPanel extends JMenuBar
{
	class MenuItem extends JMenuItem
	{
		MenuItem( String actionKey, int mneumonic )
		{
			super( m_actionMap.get( actionKey ) );
			Action action = m_actionMap.get( actionKey );
			if ( action != null ) {
				String cmdKey = (String)action.getValue( Action.ACTION_COMMAND_KEY );
				Icon icon = cmdKey == null ? null : MapViewer.createImageIcon( 
					MapViewer.ImagePath + cmdKey + "SmallIcon.gif" );
				setIcon( icon );
			}
			setMnemonic( mneumonic );
		}
	}
	
	MenuPanel( MapViewer viewer )
	{
		m_menuMap = new HashMap();
		m_actionMap = viewer.getView().getActionMap();

		JMenu fileMenu = new JMenu( "File" );
		fileMenu.setMnemonic( KeyEvent.VK_F );
		fileMenu.add( m_saveMenu = new MenuItem( "saveMap", KeyEvent.VK_S ) );
		fileMenu.add( new MenuItem( "openFile", KeyEvent.VK_O ) );
		fileMenu.add( new MenuItem( "prefs", KeyEvent.VK_P ) );
		m_menuMap.put( "File", fileMenu );
		
		JMenu buildMenu = new JMenu( "Build" );
		buildMenu.setMnemonic( KeyEvent.VK_B );
		buildMenu.add( new MenuItem( "run", KeyEvent.VK_P ) );
		buildMenu.add( new MenuItem( "step", KeyEvent.VK_S ) );
		buildMenu.add( new MenuItem( "undo", KeyEvent.VK_U ) );
		buildMenu.add( new MenuItem( "redo", KeyEvent.VK_E ) );
		buildMenu.add( new MenuItem( "clearMap", KeyEvent.VK_C ) );
		buildMenu.add( new MenuItem( "resetMap", KeyEvent.VK_R ) );
		buildMenu.add( new MenuItem( "speedInstant", KeyEvent.VK_A ) );
		buildMenu.add( new MenuItem( "rotate", KeyEvent.VK_T ) );
		m_menuMap.put( "Build", buildMenu );
		
		JMenu viewMenu = new JMenu( "View" );
		viewMenu.setMnemonic( KeyEvent.VK_V );
		viewMenu.add( new MenuItem( "zoomIn", KeyEvent.VK_I ) );
		viewMenu.add( new MenuItem( "zoomOut", KeyEvent.VK_O ) );
		viewMenu.add( new MenuItem( "centerMap", KeyEvent.VK_C ) );
		viewMenu.add( new MenuItem( "centerRobot", KeyEvent.VK_R ) );
		m_menuMap.put( "View", viewMenu );
		
		add( fileMenu );
		add( viewMenu );
		add( buildMenu );
	}
	
	JMenu getMenu( String name )
	{
		return (JMenu)m_menuMap.get( name );
	}
	
	void setSaveEnabled( boolean enabled )
	{
		m_saveMenu.setEnabled( enabled );
	}
	
	private ActionMap m_actionMap;
	private HashMap m_menuMap;
	private MenuItem m_saveMenu;
}