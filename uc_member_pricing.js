// $Id: uc_member_pricing.js,v 1.1.2.1 2009/11/30 18:03:16 awebb Exp $

Drupal.behaviors.uc_member_pricing_rule_edit_toggle = function( context ) {
	
	// Toggle form elements based upon rule type radio button selection.
	var toggle_form_type = function( ) {
		var value = $('input[name="rule_type"]:checked').val();
		
		switch ( value ) {
		case '0': // User based rule.
			$( '#rule_role-wrapper' ).hide( );
			$( '#rule_role' ).attr( 'disabled', 'disabled' );
			
			$( '#rule_user-wrapper' ).show( );
			$( '#rule_user' ).removeAttr( 'disabled' );
			break;
			
		case '1': // Role based rule.
			$( '#rule_user-wrapper' ).hide( );
			$( '#rule_user' ).attr( 'disabled', 'disabled' );
			
			$( '#rule_role-wrapper' ).show( );
			$( '#rule_role' ).removeAttr( 'disabled' );
			break;		
		}		
	}
	
	// Start process.
	$('input[name="rule_type"]').change( toggle_form_type );
	toggle_form_type( );
}