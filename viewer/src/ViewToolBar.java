import java.awt.*;
import java.awt.event.*;
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
	
			
		// Zoom in
		AbstractAction zoomIn = new AbstractAction( "Zoom In" ) {
				int zoomLevel;
				else zoomLevel = m_viewer.getView().zoomIn();
				m_zoomSlider.setValue( zoomLevel );
		};
		zoomIn.putValue( Action.ACTION_COMMAND_KEY, "zoomIn" );
			}
		} );
		// Zoom out
		AbstractAction zoomOut = new AbstractAction( "Zoom Out" ) {
				int zoomLevel;
				else zoomLevel = m_viewer.getView().zoomOut();
		};
			}
		} );
		// Center map
		AbstractAction centerMap = new AbstractAction( "Center Map" ) {
		};
		
		// Center on robot
		AbstractAction centerRobot = new AbstractAction( "Center Robot" ) {
		};

		inputMap.put( KeyStroke.getKeyStroke( "typed =" ), "zoomIn" );