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


function uc_member_pricing_add_price_form( $node ) {
  module_load_include( 'inc', 'uc_member_pricing', 'uc_member_pricing.api' );
  
  return uc_member_pricing_rule_form( NULL );
}


function uc_member_pricing_edit_price_form( $price_rule ) {
  module_load_include( 'inc', 'uc_member_pricing', 'uc_member_pricing.api' );
  
  return uc_member_pricing_rule_form( $price_rule );
}


function uc_member_pricing_delete_price_form( $price_rule ) {
  krumo($node);
  krumo($price_rule);  
}


//------------------------------------------------------------------------------
// Form handlers
//------------------------------------------------------------------------------

function uc_member_pricing_rule_form( $price_rule ) {
  
  $form = array();
  
  // Form inputs
  // 1. type [ radio buttons ] - type
  //  * username [ textfield / autocomplete ] - uid
  //  * role name [ select box ] - rid
  // 2. price [ textfield ] - [+|-] percent% | number

  $form[ 'rule_type' ] = array(
  	'#type' => 'radios',
    '#id' => 'rule_type',
  	'#title' => t( 'Pricing rule type' ),
  	'#default_value' => ( 
      $price_rule && $price_rule[ 'type' ] == TYPE_ROLE ? 1 : 0 
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
  );
  
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
      && $price_rule[ 'type' ] != TYPE_USER 
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
      && $price_rule[ 'type' ] != TYPE_ROLE
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
// Theme functions
//------------------------------------------------------------------------------


function theme_uc_member_pricing_rule_listing( $node, $price_rules ) {
  $output = '';

  krumo( $price_rules );
  
  return $output;
}