import java.awt.*;
import java.awt.event.*;import java.util.*;
import javax.swing.*;
import javax.swing.event.*;

class ViewToolBar extends JToolBar
{
	private MapViewer m_viewer;
	private JSlider m_zoomSlider;
	
	ViewToolBar( MapViewer viewer )
	{
		m_viewer = viewer;
		
		installKeyBindings( m_viewer.getView() );
		setFocusable( false );
		
		ActionMap actionMap = m_viewer.getView().getActionMap();
		
		add( new JLabel( "Zoom" ) );
		m_zoomSlider = new JSlider( GridView.CELL_SIZE_MIN, GridView.CELL_SIZE_MAX );
		m_zoomSlider.setFocusable( false );
		m_zoomSlider.setValue( GridView.CELL_SIZE );
		m_zoomSlider.setMajorTickSpacing( 1 );
		m_zoomSlider.setPaintTicks( true );
		m_zoomSlider.setSnapToTicks( true );
		m_zoomSlider.setToolTipText( "Adjusts zoom level." );
		m_zoomSlider.addChangeListener( new ChangeListener() {
			public void stateChanged(ChangeEvent e) 
			{
				JSlider source = (JSlider)e.getSource();
				if ( !source.getValueIsAdjusting() ) 
				{
					m_viewer.getView().zoomTo( (int)source.getValue() );
				}
			}
		} );
		add( m_zoomSlider );
		add( new NoFocusButton( m_viewer.getView().getActionMap().get( "zoomOut" ) ) );
		add( new NoFocusButton( m_viewer.getView().getActionMap().get( "zoomIn" ) ) );
		add( new NoFocusButton( m_viewer.getView().getActionMap().get( "centerMap" ) ) );
		add( new NoFocusButton( m_viewer.getView().getActionMap().get( "centerRobot" ) ) );
	}
		void installKeyBindings( JComponent keyComponent )	{		// Get input/action maps		InputMap inputMap = keyComponent.getInputMap();		ActionMap actionMap = keyComponent.getActionMap();
			
		// Zoom in
		AbstractAction zoomIn = new AbstractAction( "Zoom In" ) {			public void actionPerformed( ActionEvent e ) {
				int zoomLevel;				if ( (e.getModifiers() & ActionEvent.SHIFT_MASK) != 0 ) zoomLevel = m_viewer.getView().zoomInMax();
				else zoomLevel = m_viewer.getView().zoomIn();
				m_zoomSlider.setValue( zoomLevel );			}
		};
		zoomIn.putValue( Action.ACTION_COMMAND_KEY, "zoomIn" );		zoomIn.putValue( Action.SHORT_DESCRIPTION, "Zoom in incrementally, or hold Shift for maximum zoom" );		actionMap.put( "zoomIn", zoomIn );		actionMap.put( "zoomInMax", new AbstractAction( "Zoom In Max" ) {			public void actionPerformed( ActionEvent e ) {				m_zoomSlider.setValue( m_viewer.getView().zoomInMax() );
			}
		} );		
		// Zoom out
		AbstractAction zoomOut = new AbstractAction( "Zoom Out" ) {			public void actionPerformed( ActionEvent e ) {
				int zoomLevel;				if ( (e.getModifiers() & ActionEvent.SHIFT_MASK) != 0 ) zoomLevel = m_viewer.getView().zoomOutMax();
				else zoomLevel = m_viewer.getView().zoomOut();				m_zoomSlider.setValue( zoomLevel );			}
		};		zoomOut.putValue( Action.ACTION_COMMAND_KEY, "zoomOut" );		zoomOut.putValue( Action.SHORT_DESCRIPTION, "Zoom out incrementally, or hold Shift for minimum zoom" );		actionMap.put( "zoomOut", zoomOut );		actionMap.put( "zoomOutMax", new AbstractAction( "Zoom Out Max" ) {			public void actionPerformed( ActionEvent e ) {				m_zoomSlider.setValue( m_viewer.getView().zoomOutMax() );
			}
		} );		
		// Center map
		AbstractAction centerMap = new AbstractAction( "Center Map" ) {			public void actionPerformed( ActionEvent e ) {				m_viewer.getView().scrollToMapCenter();			}
		};		centerMap.putValue( Action.ACTION_COMMAND_KEY, "centerMap" );		centerMap.putValue( Action.SHORT_DESCRIPTION, "Center the map in the window" );		actionMap.put( "centerMap", centerMap );
		
		// Center on robot
		AbstractAction centerRobot = new AbstractAction( "Center Robot" ) {			public void actionPerformed( ActionEvent e ) {				m_viewer.getView().scrollToRobotCenter();			}
		};		centerRobot.putValue( Action.ACTION_COMMAND_KEY, "centerRobot" );		centerRobot.putValue( Action.SHORT_DESCRIPTION, "Center the robot in the window" );		actionMap.put( "centerRobot", centerRobot );

		inputMap.put( KeyStroke.getKeyStroke( "typed =" ), "zoomIn" );		inputMap.put( KeyStroke.getKeyStroke( "typed -" ), "zoomOut" );		inputMap.put( KeyStroke.getKeyStroke( "typed +" ), "zoomInMax" );		inputMap.put( KeyStroke.getKeyStroke( "typed _" ), "zoomOutMax" );		inputMap.put( KeyStroke.getKeyStroke( "typed c" ), "centerMap" );		inputMap.put( KeyStroke.getKeyStroke( "typed r" ), "centerRobot" );	}}