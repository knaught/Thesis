import java.awt.*;
import java.awt.event.*;
import javax.swing.border.*;
import java.util.*;
import java.lang.reflect.*;

class PreferencesPanel extends JDialog
{
	PreferencesPanel( JFrame frame, GridView gridView ) 
	{
		super( frame, "Preferences Panel" );
	
		m_gridView = gridView;
		installActionBindings( m_gridView );
	
        JTabbedPane tabbedPane = new JTabbedPane();
		tabbedPane.addTab( "Mapping", null, mappingPanel(),
			"Configure various bayesian certainty grid and sonar mapping parameters" );
		tabbedPane.addTab( "Sonars", null, sonarEnablePanel(),
			"Enable and disable selected sonars" );
		tabbedPane.addTab( "Localization", null, localizationPanel(),
			"Configure various localization related parameters" );
		tabbedPane.addTab( "Viewer", null, viewerPanel(), 
			"Configure various viewer options" );
		
		tabbedPane.setMnemonicAt( 0, KeyEvent.VK_A );
		tabbedPane.setMnemonicAt( 1, KeyEvent.VK_S );
		tabbedPane.setMnemonicAt( 2, KeyEvent.VK_L );

		setSize( new Dimension( 400, 400 ) );
		setContentPane( tabbedPane );
		
			}
		} );
		tabbedPane.getInputMap().put( KeyStroke.getKeyStroke( "typed p" ), "closeWindow" );
		tabbedPane.getInputMap().put( KeyStroke.getKeyStroke( KeyEvent.VK_ESCAPE, 0 ), "closeWindow" );
	}


	void installActionBindings( JComponent actionComponent )
		
		// Singe cell sonar model
		} );
		
		// Axis sonar model
		} );
		
		// Cone cell sonar model
		} );

		// Ignore out of range readings
				m_gridView.getModel().setIgnoreOutOfRange( box.isSelected() );
			}
		} );
	
				m_gridView.getModel().setIgnoreObstructed( box.isSelected() );
			}
		} );

			}
		} );
		// Enable all sonars
				Iterator boxes = m_sonarGroup.iterator();
					JCheckBox box = (JCheckBox)boxes.next();
					box.setSelected( true );
				}
		} );
		
		// Disable all sonars
				Iterator boxes = m_sonarGroup.iterator();
					JCheckBox box = (JCheckBox)boxes.next();
					box.setSelected( false );
				}
		} );
		// Toggle all sonars
				Iterator boxes = m_sonarGroup.iterator();
					JCheckBox box = (JCheckBox)boxes.next();
					box.setSelected( !selected );
				}
		} );
	}
	JPanel sonarEnablePanel()
	{
		// Enable Sonars
		JPanel sonarPanel = new JPanel();
		GridBagLayout sonarBag = new GridBagLayout();
		GridBagConstraints sonarConstraints = new GridBagConstraints();
		sonarPanel.setLayout( sonarBag );
		String sonarConfig[] = {
			"3@3:0 4@4:0",
			"2@2:1 5@5:1",
			"1@1:2 6@6:2",
			"_@0:3",
			"_@0:4",
			"0@0:5 7@7:5",
			"_@0:6",
			"_@0:7",
			"15@0:8 8@7:8",
			"_@0:9",
			"_@0:10",
			"14@1:11 9@6:11",
			"13@2:12 10@5:12",
			"12@3:13 11@4:13",
		};
		
		sonarConstraints.fill = GridBagConstraints.BOTH;
		sonarConstraints.weightx = 1.0;
		sonarConstraints.gridheight = 1;
		sonarConstraints.gridwidth = 1;
		
		JComponent component;
		m_sonarGroup = new Vector( 16 );
		for ( int i = 0; i < sonarConfig.length; ++i ) {
			String elem[] = sonarConfig[i].split( " " );
			for ( int j = 0; j < elem.length; ++j ) 
			{
				String e[] = elem[j].split( "@" );
				String pos[] = e[1].split( ":" );				
				sonarConstraints.gridx = Integer.parseInt( pos[0], 10 );
				sonarConstraints.gridy = Integer.parseInt( pos[1], 10 );
				sonarConstraints.gridheight = 1;
				
				if ( e[0].equals( "_" ) ) component = new JLabel( " " );
				else {
					JCheckBox box = new JCheckBox( e[0], true );
					box.setAction( m_gridView.getActionMap().get( "enableSonar" ) );
					box.setText( e[0] );
					box.setFocusable( false );
					m_sonarGroup.add( box );
					component = box;
				}
				
				sonarBag.setConstraints( component, sonarConstraints );
				sonarPanel.add( component );
			}
		}
		
		JPanel sonarControlPanel = new JPanel();
		sonarControlPanel.setLayout( new GridLayout( 4, 1, 0, 5 ) );
		sonarControlPanel.add( new NoFocusButton( m_gridView.getActionMap().get( "enableAllSonars" ) ) );
		sonarControlPanel.add( new NoFocusButton( m_gridView.getActionMap().get( "disableAllSonars" ) ) );
		sonarControlPanel.add( new NoFocusButton( m_gridView.getActionMap().get( "toggleAllSonars" ) ) );
	
		// Wrap grid in flow layout so BorderLayout won't stretch buttons
		JPanel scpLayout = new JPanel();
		scpLayout.add( sonarControlPanel );
		
		JPanel centerPanel = new JPanel();
		centerPanel.setLayout( new BorderLayout() );
		centerPanel.add( scpLayout );
		centerPanel.add( new JLabel( "<html><font size=+0>Enable/Disable Sonars</font></html>", JLabel.CENTER ), BorderLayout.NORTH );
		// TODO: figure out how to get scpLayout to stretch to fill the border layout center
		
		sonarConstraints.gridx = 1;
		sonarConstraints.gridy = 3;
		sonarConstraints.gridwidth = 6;
		sonarConstraints.gridheight = 8;
		sonarBag.setConstraints( centerPanel, sonarConstraints );
		sonarPanel.add( centerPanel );
		
		return sonarPanel;
	}
	
	JPanel mappingPanel()
	{
		// Separate parameters panel that contains cell size, beta, max occupied
		JPanel panel = new JPanel();
		panel.setLayout( new GridLayout( 9, 2, 5, 5 ) );
		GridModel model = m_gridView.getModel();
		
		panel.add( new JLabel( "Sonar Model" ) );
		panel.add( sonarModelPanel() );
		addNewField( panel, "cellSize", "Cell Size (mm)", KeyEvent.VK_S, model.getCellSize(), model ).setEnabled( false );
		addNewField( panel, "sonarBeta", "Beta Width (degrees)", KeyEvent.VK_B, model.getSonarBeta(), model );
		addNewField( panel, "maxOccupied", "Max Occupied, Region I", KeyEvent.VK_C, model.getMaxOccupied(), model );
		addNewField( panel, "maxOccupiedII", "Max Occupied, Region II", KeyEvent.VK_U, model.getMaxOccupiedII(), model );
		addNewField( panel, "regionIWidth", "Region I Width (mm)", KeyEvent.VK_W, model.getRegionIWidth(), model );
		addNewField( panel, "ignoreOutOfRange", "Ignore Out of Range Readings", KeyEvent.VK_R, model.getIgnoreOutOfRange(), model );
		addNewField( panel, "outOfRangeConversion", "Convert Out of Range To (mm)", KeyEvent.VK_V, model.getOutOfRangeConversion(), model );
		addNewField( panel, "ignoreObstructed", "Ignore Obstructed Readings", KeyEvent.VK_O, model.getIgnoreObstructed(), model );

		// Put it all together
		JPanel outerPanel = new JPanel();
		outerPanel.setLayout( new BorderLayout() );
		outerPanel.setBorder( BorderFactory.createEmptyBorder( 10, 10, 10, 10 ) );
		outerPanel.add( panel );
		
		return outerPanel;
	}
	
		JRadioButton cellButton = new JRadioButton( m_gridView.getActionMap().get( "cellModel" ) );
		cellButton.setFocusable( false );
		cellButton.setMnemonic( KeyEvent.VK_C );
		JRadioButton axisButton = new JRadioButton( m_gridView.getActionMap().get( "axisModel" ) );
		axisButton.setFocusable( false );
		axisButton.setMnemonic( KeyEvent.VK_A );
		JRadioButton coneButton = new JRadioButton( m_gridView.getActionMap().get( "coneModel" ) );
		coneButton.setFocusable( false );
		coneButton.setMnemonic( KeyEvent.VK_N );
		switch ( m_gridView.getModel().getSonarModel() ) {
			case 0:
				cellButton.setSelected( true );
				break;
			case 1:
				axisButton.setSelected( true );
				break;
			case 2:
				coneButton.setSelected( true );
				break;
		}
		ButtonGroup modelGroup = new ButtonGroup();
		modelGroup.add( cellButton );
		modelGroup.add( axisButton );
		modelGroup.add( coneButton );
		
		JPanel sonarModelPanel = new JPanel();
		sonarModelPanel.add( cellButton );
		sonarModelPanel.add( axisButton );
		sonarModelPanel.add( coneButton );
		
		JPanel panel = new JPanel();
//		panel.setLayout( new GridLayout( 1, 2, 5, 0 ) );
//		panel.add( new JLabel( "Sonar Model" ) );
		panel.add( sonarModelPanel );
	
		return panel;
	}
	JPanel localizationPanel()
	{
		
		GridModel model = m_gridView.getModel();
		addNewField( panel, "localizationEnabled", "Enabled", KeyEvent.VK_L, model.getLocalizationEnabled(), model );
		addNewField( panel, "localMapSize", "Local Map Size (mm)", KeyEvent.VK_S, model.getLocalMapSize(), model );
		addNewField( panel, "obstructedCertainty", "Obstructed Certainty", KeyEvent.VK_C, model.getObstructedCertainty(), model );
		
		JPanel motion = new JPanel();
		motion.setLayout( new BorderLayout() );
		motion.setBorder( BorderFactory.createTitledBorder( BorderFactory.createEtchedBorder(), "Motion Model" ) );
		JPanel motionInner = new JPanel();
		motionInner.setLayout( new GridLayout( 6, 2, 5, 5 ) );
		motionInner.setBorder( BorderFactory.createEmptyBorder( 5, 5, 5, 5 ) );
		addNewField( motionInner, "motionMinHeight", "Minimum Height", KeyEvent.VK_H, model.getMotionMinHeight(), model );
		addNewField( motionInner, "motionMinWidth", "Minimum Width", KeyEvent.VK_W, model.getMotionMinWidth(), model );
		addNewField( motionInner, "motionUnitDistance", "Unit Distance", KeyEvent.VK_D, model.getMotionUnitDistance(), model );
		addNewField( motionInner, "motionUnitTurn", "Unit Turn", KeyEvent.VK_T, model.getMotionUnitTurn(), model );
		addNewField( motionInner, "motionGaussSigma", "Gaussian Sigma", KeyEvent.VK_G, model.getMotionGaussSigma(), model );
		addNewField( motionInner, "motionBendFactor", "Bend Factor", KeyEvent.VK_B, model.getMotionBendFactor(), model );
		motion.add( motionInner );

		// Put it all together
		JPanel outerPanel = new JPanel();
		outerPanel.setLayout( new BoxLayout( outerPanel, BoxLayout.Y_AXIS ) );
		outerPanel.setBorder( BorderFactory.createEmptyBorder( 10, 10, 10, 10 ) );
		outerPanel.add( panel );
		outerPanel.add( motion );
		
	}
	JPanel viewerPanel()
	{
		
		addNewField( panel, "gridVisible", "Show Grid Lines", KeyEvent.VK_G, m_gridView.getGridVisible(), m_gridView );
		addNewField( panel, "axisVisible", "Show Axis", KeyEvent.VK_A, m_gridView.getAxisVisible(), m_gridView );
		
		JPanel outerPanel = new JPanel();
		outerPanel.setLayout( new BorderLayout() );
		outerPanel.setBorder( BorderFactory.createEmptyBorder( 10, 10, 10, 10 ) );
		outerPanel.add( panel, BorderLayout.NORTH );
		
	}
		panel.add( new JLabel( label ) );
		JTextField field = new JTextField( Integer.toString( value ), 5 );
		field.addActionListener( m_gridView.getActionMap().get( actionName ) );
		panel.add( field );
		return field;
	
		addAction( actionName, label, value, responseObject );
		panel.add( new JLabel( label ) );
		JTextField field = new JTextField( Float.toString( value ), 5 );
		field.addActionListener( m_gridView.getActionMap().get( actionName ) );
		panel.add( field );
	
		addAction( actionName, label, value, responseObject );
		panel.add( new JLabel( label ) );
		JCheckBox field = new JCheckBox( "", value );
		field.addActionListener( m_gridView.getActionMap().get( actionName ) );
		field.setMnemonic( mnemonic );
		panel.add( field );
		
			{
				try {
					Class pSet[] = { int.class };
					Method set = c.getDeclaredMethod( "set" + methodName, pSet );

					try {
						int value = Integer.parseInt( field.getText() );
						Object values[] = { new Integer( value ) };
						int retValue = ((Integer)set.invoke( responseObject, values )).intValue();
							throw new NumberFormatException( value + " is an invalid value for " + label );
						}
					}
					catch ( NumberFormatException x ) {
							"Parameter Error", JOptionPane.ERROR_MESSAGE );
					field.setText( ((Integer)get.invoke( responseObject )).toString() );
				}
				catch ( Exception x ) {
				}
			}
		} );
	}
	
		
			{
				try {
					Class pSet[] = { float.class };
					Method set = c.getDeclaredMethod( "set" + methodName, pSet );

					try {
						float value = Float.parseFloat( field.getText() );
						Object values[] = { new Float( value ) };
						float retValue = ((Float)set.invoke( responseObject, values )).floatValue();
							throw new Exception( value + " is an invalid value for " + label );
						}
					}
					catch ( Exception x ) {
							"Parameter Error", JOptionPane.ERROR_MESSAGE );
					field.setText( ((Float)get.invoke( responseObject )).toString() );
				}
				catch ( Exception x ) {
				}
			}
		} );
	}

		
			{
				try {
					Class pSet[] = { boolean.class };
					Method set = c.getDeclaredMethod( "set" + methodName, pSet );
					Object values[] = { new Boolean( field.isSelected() ) };
					set.invoke( responseObject, values );
				catch ( Exception x ) {
				}
			}
		} );
	}

	private GridView m_gridView;
}