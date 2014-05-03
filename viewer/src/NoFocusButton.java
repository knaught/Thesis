import javax.swing.*;

class NoFocusButton extends JButton
{
	NoFocusButton( String label ) 
	{ 
		super( label ); 
	}
	
	NoFocusButton( Action action ) { 
		this( action, false );
	}
	
	NoFocusButton( Action action, boolean label ) { 
		super( action );
		String cmdKey = (String)action.getValue( Action.ACTION_COMMAND_KEY );
		Icon icon = cmdKey == null ? null : MapViewer.createImageIcon( MapViewer.ImagePath + cmdKey + "Icon.gif" );
		setIcon( icon );
		setText( label || icon == null ? (String)action.getValue( Action.NAME ) : "" );
		setToolTipText( (String)action.getValue( Action.SHORT_DESCRIPTION ) );
	}
	
	NoFocusButton( Action action, String tip ) {
		this( action );
		setToolTipText( tip );
	}
	
	public boolean isFocusable() { return false; } 
}
	
