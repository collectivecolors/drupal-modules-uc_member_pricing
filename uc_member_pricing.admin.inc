<?php
//$Id$

//------------------------------------------------------------------------------
// Menu callbacks
//------------------------------------------------------------------------------

function uc_member_pricing_list_prices( $node ) {
  module_load_include( 'inc', 'uc_member_pricing', 'uc_member_pricing.api' );
              
  $price_rules = uc_member_pricing_select_rules( 
    array( 'nid' => $node->nid ), TRUE );
    
  return theme( 'uc_member_pricing_rule_listing', $node, $price_rules );
}

//------------------------------------------------------------------------------

function uc_member_pricing_add_price_form( $form_state, $node ) {
  module_load_include( 'inc', 'uc_member_pricing', 'uc_member_pricing.api' );  
  return uc_member_pricing_rule_form( $node, NULL );
}

//------------------------------------------------------------------------------

function uc_member_pricing_edit_price_form( $form_state, $node, $price_rule ) {
  module_load_include( 'inc', 'uc_member_pricing', 'uc_member_pricing.api' );  
  return uc_member_pricing_rule_form( $node, $price_rule );
}


//------------------------------------------------------------------------------

function uc_member_pricing_delete_price_form( $form_state, $node, $price_rule ){
  module_load_include( 'inc', 'uc_member_pricing', 'uc_member_pricing.api' );
  
  $form[ 'nid' ] = array( 
  	'#type' => 'hidden', 
  	'#value' => $node->nid,
  );
  $form[ 'ppid'] = array(
    '#type' => 'hidden',
    '#value' => $price_rule[ 'ppid' ],
  );
  
  $form[ '#submit' ][ ] = 'uc_member_pricing_rule_delete_confirm_submit';

  return confirm_form( $form,
    t( 'Are you sure you want to delete this rule?' ),
		"node/$node->nid/edit/prices", 
    t( 'This action cannot be undone.' ),
    t( 'Delete' ), 
    t( 'Cancel' ) 
  );
}

//------------------------------------------------------------------------------
// Form handlers
//------------------------------------------------------------------------------

function uc_member_pricing_rule_form( $node, $price_rule ) {
  
  $form = array();
  
  // Form inputs
  // 1. type [ radio buttons ] - type
  //  * username [ textfield / autocomplete ] - uid
  //  * role name [ select box ] - rid
  // 2. price [ textfield ] - [+|-] percent% | number
  
  drupal_add_js( drupal_get_path( 'module', 'uc_member_pricing' ) . '/uc_member_pricing.js' );
  
  $form[ '#node' ]       = $node;
  $form[ '#price_rule' ] = $price_rule;
  
  $form[ 'rule_type' ] = array(
  	'#type' => 'radios',
    '#id' => 'rule_type',
  	'#title' => t( 'Pricing rule type' ),
  	'#default_value' => ( 
      $price_rule && $price_rule[ 'type' ] == UCMP_TYPE_ROLE ? 1 : 0 
    ),
  	'#options' => array( t( 'User' ), t( 'Role' ) ),
    '#weight' => -10,
  );
  
  $form['price'] = array(
    '#type' => 'textfield',
  	'#id' => 'rule_price',
    '#title' => t('Price'),
    '#maxlength' => 60,
    '#default_value' => ( $price_rule ? $price_rule[ 'price' ] : '' ),
    '#weight' => 10,
    '#description' => t(
    	'You may enter a specific price ( 25.50 ) or an offset from the default'
      . ' sell price ( +|-10 ).  You may also enter a percentage offset from'
      . ' the default ( +|-5% ).  Remember, if you are using an offset,'
      . ' you must specify a plus ( + ) or ( - ) before the value.  If you are'
      . ' entering a percentage then specify ( % ) after the numeric offset.'
    ),
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#validate' => array( 'uc_member_pricing_rule_form_validate' ),
    '#submit' => array( 'uc_member_pricing_rule_form_submit' ),
    '#value' => t( 'Save rule' ),
    '#weight' => 12,
  );
  
  if ( $price_rule ) {
    $form[ 'delete' ] = array(
      '#type' => 'submit',
      '#submit' => array( 'uc_member_pricing_rule_delete_submit' ),
      '#value'=> t( 'Delete rule' ),
      '#weight' => 14,
    );
  }
  
  $form = array_merge(
    $form,
    uc_member_pricing_user_form( $price_rule ),
    uc_member_pricing_role_form( $price_rule ) 
  );
  
  return $form;
}

//------------------------------------------------------------------------------

function uc_member_pricing_user_form( $price_rule ) {
  
  $form = array();
  
  // Form inputs
  // * username [ textfield / autocomplete ] - uid
  
  if ( $price_rule 
      && $price_rule[ 'type' ] == UCMP_TYPE_USER 
      && array_key_exists( 'xid', $price_rule ) ) {
         
    $account = user_load( $price_rule[ 'xid' ] );
  }
  
  $form['user_name'] = array(
    '#type' => 'textfield',
  	'#id' => 'rule_user',
    '#title' => t('User name'),
    '#maxlength' => 60,
    '#autocomplete_path' => 'user/autocomplete',
    '#default_value' => $account ? $account->name : '',
    '#weight' => -1,
  );
  
  return $form;  
}

//------------------------------------------------------------------------------

function uc_member_pricing_role_form( $price_rule ) {
  
  $form = array();
  
  // Form inputs
  // * role name [ select box ] - rid
  
  if ( $price_rule 
      && $price_rule[ 'type' ] != UCMP_TYPE_ROLE
      && array_key_exists( 'xid', $price_rule ) ) {
         
    $rid = $price_rule[ 'xid' ];
  }
  
  $options = array();
  foreach ( user_roles() as $rid => $name ) {
    $options[ $rid ] = $name;
  }
  
  $form['user_role'] = array(
  	'#type' => 'select',
    '#id' => 'rule_role',
  	'#title' => t('User role'),
  	'#default_value' => ( $rid ? $rid : NULL ),
  	'#options' => $options,
  );
  
  return $form;    
}

//------------------------------------------------------------------------------

function uc_member_pricing_rule_form_validate( $form, &$form_state ) {
  $node       = $form[ '#node' ];
  $price_rule = $form[ '#price_rule' ];
  
  if ( $form_state[ 'values' ][ 'rule_type' ] ) {
    // Role based rule.
    $type = UCMP_TYPE_ROLE;
    $xid  = $form_state[ 'values' ][ 'user_role' ];
  }
  else {
    // User based rule.
    $type = UCMP_TYPE_USER;
    $xid  = $form_state[ 'values' ][ 'user_name' ];

    if ( !$xid || !( $account = user_load( array( 'name' => $xid ) ) ) ) {
      form_set_error( 'user_name', 
        t( 'The username %name does not exist.', array('%name' => $xid ) ) );
    }
    
    $xid = $account->uid;
  }
    
  // If adding new price rule, make sure it doesn't exist yet.
  if ( is_null( $price_rule ) ) {
    
    $existing_rule = uc_member_pricing_select_rules( 
      array( 
      	'nid' => $node->nid,
        'type' => $type,
        'xid'  => $xid 
      )
    );
    
    if ( $existing_rule ) {
      form_set_error( '', 
        t( 'This rule already exists.'
        	 . '  Go to the !edit_link to edit.',
        	array( '!edit_link' => l( t( 'edit page' ), 
        			"node/$node->nid/edit/prices/edit/" . $existing_rule[ 'ppid' ] 
        	  ) 
        	) 
        ) 
      ); 
    }
  }
  else {
    // Adjust form values for submit ( to save some processing time ).
    $form_state[ 'values' ][ 'ppid' ] = $price_rule[ 'ppid' ];
  }
  
  $price = _uc_member_pricing_validate_price($form_state['values'][ 'price' ]);
  
  if ( $price === FALSE ) {
    form_set_error( 'price', 
      t( 'You have entered an invalid price offset.'
      	. ' See the description under the price field for the proper syntax.' 
      ) 
    );
  }
  
  // Adjust form values for submit ( to save some processing time ).
  $form_state[ 'values' ][ 'nid' ]       = $node->nid;
  $form_state[ 'values' ][ 'rule_type' ] = $type;
  $form_state[ 'values' ][ 'xid' ]       = $xid;
  $form_state[ 'values' ][ 'price' ]     = $price;
}

//------------------------------------------------------------------------------

function uc_member_pricing_rule_form_submit( $form, &$form_state ) {
  
  $nid = $form_state[ 'values' ][ 'nid' ];
  
  uc_member_pricing_save_rule( array(
    'ppid'  => $form_state[ 'values' ][ 'ppid' ],
    'nid'   => $nid,
    'type'  => $form_state[ 'values' ][ 'rule_type' ],
    'xid'   => $form_state[ 'values' ][ 'xid' ],
    'price' => $form_state[ 'values' ][ 'price' ]
  ) );
  
  $form_state[ 'redirect' ] = "node/$nid/edit/prices";
}

//------------------------------------------------------------------------------

function uc_member_pricing_rule_delete_submit( $form, &$form_state ) {
  $node       = $form[ '#node' ];
  $price_rule = $form[ '#price_rule' ];
  
  $form_state[ 'redirect' ] = "node/$node->nid/edit/prices/delete/" 
                            . $price_rule[ 'ppid' ];
}

//------------------------------------------------------------------------------

function uc_member_pricing_rule_delete_confirm_submit( $form, &$form_state ) {
  
  $nid = $form_state[ 'values' ][ 'nid' ];
  
  if ( $form_state['values']['confirm'] ) {
    uc_member_pricing_remove_rule( $form_state[ 'values' ][ 'ppid' ] );  
  }
  
  $form_state[ 'redirect' ] = "node/$nid/edit/prices";
}

//------------------------------------------------------------------------------
// Theme functions
//------------------------------------------------------------------------------

function theme_uc_member_pricing_rule_listing( $node, $price_rules ) {
  
  $header = array( t( 'Type' ), t( 'Applies to' ), t( 'Price' ), '' );
  $rules  = _uc_member_pricing_separate_prices( $price_rules );
  
  foreach ( $rules as $nid => $node_rules ) {
	  foreach ( $node_rules as $type => $type_rules ) {
	    $rows = array( );
	    
	    foreach ( $type_rules as $rule ) {
	      $row = array( );
	      
        switch ( $rule[ 'type' ] ) {
      
          case UCMP_TYPE_USER:
            $type = t( 'User rule' );
        
            $account  = user_load( array( 'uid' => $rule[ 'xid'] ) );
            $for_name = $account->name;        
            break;
        
          case UCMP_TYPE_ROLE:
            $type = t( 'Role based rule' );
        
            $roles = user_roles();
            $for_name = $roles[ $rule[ 'xid' ] ];
            break;
        }
        
        $row[ ] = $type;               
        $row[ ] = $for_name;
        $row[ ] = $rule[ 'price' ];
    
        $row[ ] = l( t( 'edit' ), 
    			"node/$node->nid/edit/prices/edit/" . $rule[ 'ppid' ], 
          array( 'class' => 'edit-rule-link' ) 
        ) . ' | '
        . l( t( 'delete' ),
      		"node/$node->nid/edit/prices/delete/" . $rule[ 'ppid' ],
          array( 'class' => 'delete-rule-link' )
        );
    
	      $rows[ ] = $row;
	    }
	    $output .= theme('table', $header, $rows);
	  }
  }  
  
  $output .= l( t( 'Add new price rule' ), 
  	"node/$node->nid/edit/prices/add", 
    array( 'class' => 'add-rule-link') 
  );
  
  return $output;
}

//------------------------------------------------------------------------------
// Internal utility functions
//------------------------------------------------------------------------------

function _uc_member_pricing_validate_price( $price ) {

  $price = preg_replace( '/\s+$/', '', $price );
  
  // If it is a integer or decimal number, let it pass.
  if ( is_numeric( $price ) ) {
    return $price;
  }
  // If it is a percentage, test for integer or decimal number.
  elseif ( preg_match( '/\%$/', $price ) ) {
    $test = preg_replace( '/\%$/', '', $price );
    
    if ( is_numeric( $test ) ) {
      return $price;
    }
  }
  
  return FALSE;
}

//------------------------------------------------------------------------------
// Internal utilities
//------------------------------------------------------------------------------

function _uc_member_pricing_separate_prices( $pricing_rules ) {
  $rules = array( );
	
	foreach ( $pricing_rules as $rule ) {
	  $rules[ $rule[ 'nid' ] ][ $rule[ 'type' ] ][ ] = $rule;  
	}
	
	return $rules;  
}

//------------------------------------------------------------------------------