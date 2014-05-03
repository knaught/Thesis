import java.awt.*;
import java.awt.event.*;import java.util.*;
import javax.swing.*;

class StatusPanel extends JPanel implements GridModel.ChangeListener
{
	private MapViewer m_viewer;
	private Vector m_sonarGroup;

	JLabel m_rangeDeviceId = new JLabel();
	JLabel m_rangeReading = new JLabel();
	JLabel m_poseX = new JLabel();
	JLabel m_poseY = new JLabel();
	JLabel m_poseTh = new JLabel();
												  
	StatusPanel( MapViewer viewer )
	{
		m_viewer = viewer;
		setFocusable( false );
		setLayout( new BorderLayout() );
		
		m_viewer.getView().getModel().addChangeListener( this );

		JPanel panel = new JPanel( new GridLayout( 1, 5 ) {
			public Dimension preferredLayoutSize( Container parent ) 
			{
				// TODO: Figure out how to achieve this "look" without such poor technique!
				Dimension dimension = new Dimension( 600, 20 );
				return dimension;
			}
		} );

		FlowLayout flowLeft = new FlowLayout( FlowLayout.LEFT );
			
		JPanel panelPoseX = new JPanel( flowLeft );
		panelPoseX.add( new JLabel( "Pose X: " ) );
		panelPoseX.add( m_poseX );
		panel.add( panelPoseX );
			
		JPanel panelPoseY = new JPanel( flowLeft );
		panelPoseY.add( new JLabel( "Y: " ) );
		panelPoseY.add( m_poseY );
		panel.add( panelPoseY );
			
		JPanel panelPoseTh = new JPanel( flowLeft );
		panelPoseTh.add( new JLabel( "Theta: " ) );
		panelPoseTh.add( m_poseTh );
		panel.add( panelPoseTh );
			
		JPanel panelSonar = new JPanel( flowLeft );
		panelSonar.add( new JLabel( "Sonar Number: " ) );
		panelSonar.add( m_rangeDeviceId );
		panel.add( panelSonar );
			
		JPanel panelRange = new JPanel( flowLeft );
		panelRange.add( new JLabel( "Range: " ) );
		panelRange.add( m_rangeReading );
		panel.add( panelRange );
			
		JPanel wrapper = new JPanel(); // prevents stretching to fill window
		wrapper.add( panel );
		add( wrapper, BorderLayout.CENTER );
	}
		
	void update( GridModel.RangeReading rangeReading )
	{
		m_rangeDeviceId.setText( Integer.toString( rangeReading.deviceId ) );
		m_rangeReading.setText( Integer.toString( rangeReading.range ) );
		m_poseX.setText( Integer.toString( rangeReading.poseX ) );
		m_poseY.setText( Integer.toString( rangeReading.poseY ) );
		m_poseTh.setText( Integer.toString( rangeReading.poseTh ) );
	}
	
	public void changePerformed( GridModel.ChangeEvent event )
	{		// pass on to data panel		update( event.getRangeReading() );
	}}
