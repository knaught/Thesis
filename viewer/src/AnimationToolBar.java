import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

class AnimationToolBar extends JToolBar
{
	private final int m_speeds[] = { 5000, 4000, 3000, 2000, 1000, 500, 250, 120, 60, 30, 1 };
	private MapViewer m_viewer;
	private int m_timerDelay = m_speeds.length - 1;
	private Action m_runAction;
	private boolean m_needsReset;
	private JButton m_runButton;
	
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

	{
		m_needsReset = mapComplete;
	}
		m_runAction = new AbstractAction( "Play" ) {
				if ( m_timerRunning ) {
					m_runButton.setIcon( m_playIcon );
				}
				else {
					m_runButton.setIcon( m_pauseIcon );
					timer.schedule( 
							public void run() {
								if ( m_needsReset ) {
									m_needsReset = !m_viewer.getView().reset();
								}
								if ( m_viewer.getView().update() == null ) {
									// Reset everything
								}
							}
						}, 0, m_speeds[m_timerDelay] );
				}
			}
		actionMap.put( "run", m_runAction );
				if ( m_timerRunning ) m_runAction.actionPerformed( null );
				// Otherwise take one step
					if ( m_needsReset ) {
					}
			}
		m_stepAction.putValue( Action.ACTION_COMMAND_KEY, "step" );
		actionMap.put( "step", m_stepAction );
				if ( m_timerRunning ) m_runAction.actionPerformed( null );
				// Show map
				if ( m_needsReset ) {
					m_needsReset = !m_viewer.getView().reset();
				}
				m_viewer.getView().updateAll();
				Date end = new Date();
				System.err.println( "Build Time: " + (end.getTime() - start.getTime()) + " ms" );
			}
		};
		actionMap.put( "speedInstant", speedInstant );
			
		// Speed animation one step
				speedAt( m_timerDelay + 1 );
			}
		};
			
			}
		};
			
		// Animate with shortest possible delay between steps
				speedAt( m_speeds.length-1 );
			}
		};
			
			}
		};
			
		AbstractAction resetMap = new AbstractAction( "Reset Map" ) {
			}
		};
		actionMap.put( "resetMap", resetMap );
			
		AbstractAction clearMap = new AbstractAction( "Clear Map" ) {
			}
		};
		actionMap.put( "clearMap", clearMap );
			
				if ( !m_needsReset ) return;
				// Clear and get new localized map
				m_viewer.getView().clear();
				m_viewer.getView().updateLocalized();
			}
		};
		rotate.putValue( Action.ACTION_COMMAND_KEY, "rotate" );
		actionMap.put( "rotate", rotate );
		AbstractAction undo = new AbstractAction( "Undo" ) {
			}
		};
		actionMap.put( "undo", undo );
			
		AbstractAction redo = new AbstractAction( "Redo" ) {
			}
		};
		actionMap.put( "redo", redo );
				
		inputMap.put( KeyStroke.getKeyStroke( "SPACE" ), "step" );
		inputMap.put( KeyStroke.getKeyStroke( "ctrl SPACE" ), "run" );
			InputEvent.CTRL_MASK | InputEvent.SHIFT_MASK | InputEvent.ALT_MASK ), "speedInstant" );
			InputEvent.CTRL_MASK | InputEvent.SHIFT_MASK | InputEvent.ALT_MASK ), "resetMap" );
		inputMap.put( KeyStroke.getKeyStroke( KeyEvent.VK_BACK_SPACE, 0 ), "undo" );
		m_timerDelay = speed;
		m_speedSlider.setValue( speed );
		if ( m_timerRunning ) {
			m_runAction.actionPerformed( null );
		}
}