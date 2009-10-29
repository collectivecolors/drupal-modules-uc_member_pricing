<?php
//$Id$

//------------------------------------------------------------------------------
// Database manipulation functions
//------------------------------------------------------------------------------

function uc_member_pricing_select_rules( $conditions, $force_array = FALSE ) {
  
  $where      = array();
  $where_args = array();
  
  if ( is_array( $conditions ) && count( $conditions ) ) {
    foreach ( $conditions as $key => $value ) {
      switch ( $key ) {
      
        case 'ppid':
        case 'nid':      
        case 'xid':
          $where[] = "$key = %d";
          break;
        
        case 'type':
        case 'price':
          $where[] = "$key = '%s'";        
          break;

        default:
          continue;
      }
    
      $where_args[] = $value;
    }
  }
  
  $pricing_rules = array();
  
  $results = db_query( 'SELECT * FROM {uc_member_prices}' 
                       . ( count( $where ) ? ' WHERE ' . join( ' AND ', $where ) 
                                           : '' ),
                       $where_args );
                       
  while ( $rule = db_fetch_array( $results ) ) {
    $pricing_rules[] = $rule;  
  }
  
  if ( ! $force_array && count( $pricing_rules ) <= 1 ) {
    return ( count( $pricing_rules ) ? $pricing_rules[ 0 ] : FALSE );
  }
  
  return $pricing_rules;  
}

//------------------------------------------------------------------------------

function uc_member_pricing_add_rule( $nid, $type, $xid, $price, 
                                      $skip_check = FALSE ) {  
  if ( ! $skip_check ) {
    $pricing_rule = uc_member_pricing_select_rules( 
      array( 
  			'nid' => $nid, 
    		'type' => $type,
    		'xid' => $xid 
      )
    );
  
    if ( $pricing_rule ) {
      $pricing_rule[ 'price' ] = $price;  
      return uc_member_pricing_save_rule( $pricing_rule, TRUE );
    }
  }
  
  $pricing_rule = array(
    'nid'   => $nid,
    'type'  => $type,
    'xid'   => $xid,
    'price' => $price
  );
   
  db_query( 
  	'INSERT INTO {uc_member_prices}'
  	. ' ( nid, type, xid, price )'
  	. ' VALUES ( %d, "%s", %d, "%s" )',
  	$pricing_rule[ 'nid' ], 
  	$pricing_rule[ 'type' ], 
  	$pricing_rule[ 'xid' ], 
  	$pricing_rule[ 'price' ]
  );
  
  $pricing_rule[ 'ppid' ] = db_last_insert_id( 'uc_member_prices', 'ppid' );
  
  return $pricing_rule;  
}

//------------------------------------------------------------------------------

function uc_member_pricing_save_rule( $pricing_rule, $skip_check = FALSE ) {
  
  if ( ! $skip_check ) {
    if ( $pricing_rule[ 'ppid' ] ) {
      $price_rule = uc_member_pricing_select_rules( 
        array( 
    			'ppid' => $pricing_rule[ 'ppid' ] 
        ) 
      );
    }
    else {
      $is_new = TRUE;
    }
  
    if ( $is_new || ! $price_rule ) {
      return uc_member_pricing_add_rule(
        $pricing_rule[ 'nid'],
        $pricing_rule[ 'type' ],
        $pricing_rule[ 'xid' ],
        $pricing_rule[ 'price' ],
        TRUE 
      );
    }
  }
  
  db_query( 'UPDATE {uc_member_prices}'
  					. ' SET nid = %d, type = "%s", xid = %d, price = "%s"'
  					. ' WHERE ppid = %d',
    $pricing_rule[ 'nid' ],
    $pricing_rule[ 'type' ],
    $pricing_rule[ 'xid' ],
    $pricing_rule[ 'price' ],
    $pricing_rule[ 'ppid' ] 
  );
  
  return $pricing_rule;
}

//------------------------------------------------------------------------------

function uc_member_pricing_remove_rule( $pricing_rule ) {
  
  if ( is_array( $pricing_rule ) ) {
    $ppid = $pricing_rule[ 'ppid' ];
  }
  else {
    $ppid = $pricing_rule;
  }
  
  db_query( 'DELETE FROM {uc_member_prices} WHERE ppid = %d', $ppid );
}

//------------------------------------------------------------------------------

