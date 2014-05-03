import java.awt.*;
import java.awt.event.*;import java.util.*;
import javax.swing.*;
import javax.swing.event.*;

class AnimationToolBar extends JToolBar
{
	private final int m_speeds[] = { 5000, 4000, 3000, 2000, 1000, 500, 250, 120, 60, 30, 1 };
	private MapViewer m_viewer;	private boolean m_timerRunning = false;
	private int m_timerDelay = m_speeds.length - 1;
	private Action m_runAction;	private Action m_stepAction;
	private boolean m_needsReset;
	private JButton m_runButton;	private ImageIcon m_playIcon;	private ImageIcon m_pauseIcon;	private Vector m_controls = new Vector();	private JSlider m_speedSlider;
	
	AnimationToolBar( MapViewer viewer )
	{
		m_viewer = viewer;
		
		installKeyBindings( m_viewer.getView() );
		setFocusable( false );
		
		m_playIcon = MapViewer.createImageIcon( MapViewer.ImagePath + "runIcon.gif" );
		m_pauseIcon = MapViewer.createImageIcon( MapViewer.ImagePath + "pauseIcon.gif" );
		
		installButton( "resetMap" );
		m_runButton = installButton( "run" );
		installButton( "step" );
		installButton( "undo" );
		installButton( "redo" );
		installButton( "speedInstant" );
		installButton( "clearMap" );

		add( new JLabel( "Speed" ) );
		m_speedSlider = new JSlider( 0, 10 );
		m_speedSlider.setFocusable( false );
		m_speedSlider.setValue( m_timerDelay );
		m_speedSlider.setMajorTickSpacing( 1 );
		m_speedSlider.setPaintTicks(true);
		m_speedSlider.setSnapToTicks(true);
		m_speedSlider.setToolTipText( "Speeds (Shift-Right) and slows (Shift-left) animation." +
			"Use Ctrl-Right/Left to jump to maximum/minimum speed." );
		m_speedSlider.addChangeListener( new ChangeListener() {
			public void stateChanged(ChangeEvent e) 
			{
				JSlider source = (JSlider)e.getSource();
				if ( !source.getValueIsAdjusting() ) 
				{
					speedAt( source.getValue() );
				}
			}
		} );
		m_controls.addElement( m_speedSlider );
		add( m_speedSlider );
		
		installButton( "rotate" );
	}
	
	NoFocusButton installButton( String key )
	{
		NoFocusButton b = new NoFocusButton( m_viewer.getView().getActionMap().get( key ) );
		m_controls.addElement( b );
		add( b );
		return b;
	}
	
	public void setEnabled( boolean enabled )
	{
		Iterator i = m_controls.iterator();
		while ( i.hasNext() ) {
			JComponent c = (JComponent)i.next();
			c.setEnabled( enabled );
		}
	}
	public void setMapComplete( boolean mapComplete )
	{
		m_needsReset = mapComplete;
	}		void installKeyBindings( JComponent keyComponent )	{		// Get input/action maps		InputMap inputMap = keyComponent.getInputMap();		ActionMap actionMap = keyComponent.getActionMap();				// Animate and pause steps through log entries		// Note: this is a member-level Action because other Actions need to access it
		m_runAction = new AbstractAction( "Play" ) {			java.util.Timer timer;			public void actionPerformed( ActionEvent e ) {				// Pause
				if ( m_timerRunning ) {
					m_runButton.setIcon( m_playIcon );					timer.cancel();					m_timerRunning = false;
				}				// Start
				else {
					m_runButton.setIcon( m_pauseIcon );					timer = new java.util.Timer();
					timer.schedule( 						new TimerTask() {
							public void run() {								// Reset map if previously finished one
								if ( m_needsReset ) {
									m_needsReset = !m_viewer.getView().reset();
								}								// Attempt update, and if failed
								if ( m_viewer.getView().update() == null ) {
									// Reset everything									m_runButton.setIcon( m_playIcon );									timer.cancel();									m_timerRunning = false;									m_needsReset = true;
								}
							}
						}, 0, m_speeds[m_timerDelay] );					m_timerRunning = true;
				}
			}		};		m_runAction.putValue( Action.ACTION_COMMAND_KEY, "run" );		m_runAction.putValue( Action.SHORT_DESCRIPTION, "Animate map build" );		m_runAction.putValue( Action.SMALL_ICON, MapViewer.createImageIcon( MapViewer.ImagePath + "runSmallIcon.gif" ) );
		actionMap.put( "run", m_runAction );					// Step through single log entry		m_stepAction = new AbstractAction( "Step" ) {			public void actionPerformed( ActionEvent e ) {				// Pause animation if running
				if ( m_timerRunning ) m_runAction.actionPerformed( null );
				// Otherwise take one step				else {
					if ( m_needsReset ) {						m_needsReset = !m_viewer.getView().reset();
					}					m_viewer.getView().update();				}
			}		};
		m_stepAction.putValue( Action.ACTION_COMMAND_KEY, "step" );		m_stepAction.putValue( Action.SHORT_DESCRIPTION, "Build map one sonar reading at a time" );		m_stepAction.putValue( Action.SMALL_ICON, MapViewer.createImageIcon( MapViewer.ImagePath + "stepSmallIcon.gif" ) );
		actionMap.put( "step", m_stepAction );					// Show complete map instantly 		AbstractAction speedInstant = new AbstractAction( "Build All" ) {			public void actionPerformed( ActionEvent e ) {				// Pause animation if running
				if ( m_timerRunning ) m_runAction.actionPerformed( null );
				// Show map
				if ( m_needsReset ) {
					m_needsReset = !m_viewer.getView().reset();
				}				Date start = new Date();
				m_viewer.getView().updateAll();
				Date end = new Date();
				System.err.println( "Build Time: " + (end.getTime() - start.getTime()) + " ms" );				m_needsReset = true;
			}
		};		speedInstant.putValue( Action.ACTION_COMMAND_KEY, "speedInstant" );		speedInstant.putValue( Action.SHORT_DESCRIPTION, "Fast forward map to final built state" );		speedInstant.putValue( Action.SMALL_ICON, MapViewer.createImageIcon( MapViewer.ImagePath + "speedInstantSmallIcon.gif" ) );
		actionMap.put( "speedInstant", speedInstant );
			
		// Speed animation one step		AbstractAction speedUp = new AbstractAction( "Speed Up" ) {			public void actionPerformed( ActionEvent e ) {
				speedAt( m_timerDelay + 1 );
			}
		};		speedUp.putValue( Action.ACTION_COMMAND_KEY, "speedUp" );		speedUp.putValue( Action.SHORT_DESCRIPTION, "Speed up animation" );		actionMap.put( "speedUp", speedUp );
					// Animate with longest possible delay between steps		AbstractAction speedDown = new AbstractAction( "Min Speed" ) {			public void actionPerformed( ActionEvent e ) {				speedAt( m_timerDelay - 1 );
			}
		};		speedDown.putValue( Action.ACTION_COMMAND_KEY, "speedDown" );		speedDown.putValue( Action.SHORT_DESCRIPTION, "Slow down animation" );		actionMap.put( "speedDown", speedDown );
			
		// Animate with shortest possible delay between steps		AbstractAction speedMax = new AbstractAction( "Max Speed" ) {			public void actionPerformed( ActionEvent e ) {
				speedAt( m_speeds.length-1 );
			}
		};		speedMax.putValue( Action.ACTION_COMMAND_KEY, "speedMax" );		speedMax.putValue( Action.SHORT_DESCRIPTION, "Speed animation to minimum delay" );		actionMap.put( "speedMax", speedMax );
					// Animate with longest possible delay between steps		AbstractAction speedMin = new AbstractAction( "Min Speed" ) {			public void actionPerformed( ActionEvent e ) {				speedAt( 0 );
			}
		};		speedMin.putValue( Action.ACTION_COMMAND_KEY, "speedMin" );		speedMin.putValue( Action.SHORT_DESCRIPTION, "Slow animation to maximum delay" );		actionMap.put( "speedMin", speedMin );
					// Reset map
		AbstractAction resetMap = new AbstractAction( "Reset Map" ) {		public void actionPerformed( ActionEvent e ) {				m_needsReset = !m_viewer.getView().reset();
			}
		};		resetMap.putValue( Action.ACTION_COMMAND_KEY, "resetMap" );		resetMap.putValue( Action.SHORT_DESCRIPTION, "Rewind map to its original unbuilt state" );		resetMap.putValue( Action.SMALL_ICON, MapViewer.createImageIcon( MapViewer.ImagePath + "resetMapSmallIcon.gif" ) );
		actionMap.put( "resetMap", resetMap );
					// Clear map
		AbstractAction clearMap = new AbstractAction( "Clear Map" ) {		public void actionPerformed( ActionEvent e ) {				m_viewer.getView().clear();
			}
		};		clearMap.putValue( Action.ACTION_COMMAND_KEY, "clearMap" );		clearMap.putValue( Action.SHORT_DESCRIPTION, "Clear the map drawn so far without resetting the data file state" );		clearMap.putValue( Action.SMALL_ICON, MapViewer.createImageIcon( MapViewer.ImagePath + "clearMapSmallIcon.gif" ) );
		actionMap.put( "clearMap", clearMap );
					// Localize map 		AbstractAction rotate = new AbstractAction( "Rotate" ) {			public void actionPerformed( ActionEvent e ) {				// Skip if map not drawn yet
				if ( !m_needsReset ) return;				
				// Clear and get new localized map
				m_viewer.getView().clear();
				m_viewer.getView().updateLocalized();				m_needsReset = true;
			}
		};
		rotate.putValue( Action.ACTION_COMMAND_KEY, "rotate" );		rotate.putValue( Action.SHORT_DESCRIPTION, "Individually rotate the local maps that make up the global map" );		rotate.putValue( Action.SMALL_ICON, MapViewer.createImageIcon( MapViewer.ImagePath + "rotateSmallIcon.gif" ) );
		actionMap.put( "rotate", rotate );					// Undo
		AbstractAction undo = new AbstractAction( "Undo" ) {		public void actionPerformed( ActionEvent e ) {				m_viewer.getView().undo();
			}
		};		undo.putValue( Action.ACTION_COMMAND_KEY, "undo" );		undo.putValue( Action.SHORT_DESCRIPTION, "Undo a single map update" );		undo.putValue( Action.SMALL_ICON, MapViewer.createImageIcon( MapViewer.ImagePath + "undoSmallIcon.gif" ) );
		actionMap.put( "undo", undo );
					// Redo
		AbstractAction redo = new AbstractAction( "Redo" ) {		public void actionPerformed( ActionEvent e ) {				m_stepAction.actionPerformed( null );
			}
		};		redo.putValue( Action.ACTION_COMMAND_KEY, "redo" );		redo.putValue( Action.SHORT_DESCRIPTION, "Redo a single map update" );		redo.putValue( Action.SMALL_ICON, MapViewer.createImageIcon( MapViewer.ImagePath + "redoSmallIcon.gif" ) );
		actionMap.put( "redo", redo );
				
		inputMap.put( KeyStroke.getKeyStroke( "SPACE" ), "step" );
		inputMap.put( KeyStroke.getKeyStroke( "ctrl SPACE" ), "run" );		inputMap.put( KeyStroke.getKeyStroke( KeyEvent.VK_RIGHT, InputEvent.SHIFT_MASK ), "speedUp" );		inputMap.put( KeyStroke.getKeyStroke( KeyEvent.VK_LEFT, InputEvent.SHIFT_MASK ), "speedDown" );		inputMap.put( KeyStroke.getKeyStroke( KeyEvent.VK_RIGHT, InputEvent.CTRL_MASK ), "speedMax" );		inputMap.put( KeyStroke.getKeyStroke( KeyEvent.VK_LEFT, InputEvent.CTRL_MASK ), "speedMin" );		inputMap.put( KeyStroke.getKeyStroke( KeyEvent.VK_RIGHT, 
			InputEvent.CTRL_MASK | InputEvent.SHIFT_MASK | InputEvent.ALT_MASK ), "speedInstant" );		inputMap.put( KeyStroke.getKeyStroke( KeyEvent.VK_LEFT, 
			InputEvent.CTRL_MASK | InputEvent.SHIFT_MASK | InputEvent.ALT_MASK ), "resetMap" );
		inputMap.put( KeyStroke.getKeyStroke( KeyEvent.VK_BACK_SPACE, 0 ), "undo" );	}		void speedAt( int speed )	{		if ( speed < 0 ) speed = 0;		if ( speed > m_speeds.length-1 ) speed = m_speeds.length-1;
		m_timerDelay = speed;
		m_speedSlider.setValue( speed );		// Pause and restart with new speed
		if ( m_timerRunning ) {
			m_runAction.actionPerformed( null );			m_runAction.actionPerformed( null );
		}	}
}