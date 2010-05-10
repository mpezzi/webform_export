<?php

define('WEBFORM_TMP_DIR',           '/tmp/');
define('WEBFORM_EXPORT_DIR',        '/var/www/contests/sites/default/files/results/');
define('WEBFORM_FILE_DATE_FORMAT',  'd-m-Y--H-i-s');

/**
 * Page Callback: Export page.
 */
function webform_export_export_callback($node) {
  return _webform_export_submissions($node);
}

/**
 * Page Callback: Status page.
 */
function webform_export_status_callback($node) {
  return theme('webform_export_status', $node);
}

/**
 * Calculate total entries.
 */
function _webform_export_submissions_count_total($node) {
  return db_result(db_query("SELECT COUNT(d.data) FROM {webform_submitted_data} as d INNER JOIN {webform_component} as c ON d.cid = c.cid AND d.nid = c.nid WHERE c.form_key = 'email' AND d.nid = %d", $node->nid));
}

/**
 * Calculate unique entries.
 */
function _webform_export_submissions_count_unique($node) {
  return db_result(db_query("SELECT COUNT(DISTINCT d.data) FROM {webform_submitted_data} as d INNER JOIN {webform_component} as c ON d.cid = c.cid AND d.nid = c.nid WHERE c.form_key = 'email' AND d.nid = %d", $node->nid));
}

/**
 * Calculate unique client optin entries.
 */
function _webform_export_submissions_count_optin($node) {
  
  // Create a view of all the node results.
  $sql =  "CREATE OR REPLACE VIEW webform_export_submissions_optin AS ";
  $sql .= "SELECT d.sid, ";
  
  $component_sql = array();
  foreach ( _webform_export_get_components($node) as $component ) {
    $component_sql[] = "GROUP_CONCAT( if( c.form_key = '$component->form_key', d.data, NULL ) ) AS '$component->form_key'";
  }
  
  $sql .= implode(', ', $component_sql);
  $sql .= ' FROM {webform_submitted_data} AS d';
  $sql .= ' LEFT JOIN {webform_component} AS c ON d.cid = c.cid AND d.nid = c.nid';
  $sql .= ' WHERE d.nid = '. $node->nid;
  $sql .= ' GROUP BY d.sid;';
  
  // Execute
  db_query($sql);
  
  // Find number of unique optins.
  $optins = db_result(db_query("SELECT COUNT(DISTINCT email) FROM webform_export_submissions_optin WHERE opt_in_sponsor = 1"));
  
  // Drop the view.
  db_query('DROP VIEW webform_export_submissions_optin');
  
  return $optins;
}

/**
 * Export submissions.
 */
function _webform_export_submissions($node) {
  
  // Construct SQL Query to export all results into a view.
  $sql =  "CREATE OR REPLACE VIEW webform_export_submissions_view_". $node->nid ." AS ";
  $sql .= "SELECT d.sid, s.submitted, ";
  
  // Add the approriate webform components to the SQL Query.
  $components = array();
  foreach ( _webform_export_get_components($node) as $component ) {
    $components[] = "GROUP_CONCAT( if( c.form_key = '$component->form_key', d.data, NULL ) ) AS '$component->form_key'";
  }
  
  // Construct the rest of the SQL Query.
  $sql .= implode(', ', $components);
  $sql .= ' FROM {webform_submitted_data} AS d';
  $sql .= ' LEFT JOIN {webform_component} AS c ON d.cid = c.cid AND d.nid = c.nid';
  $sql .= ' LEFT JOIN {webform_submissions} AS s ON d.sid = s.sid';
  $sql .= ' WHERE d.nid = '. $node->nid;
  $sql .= " GROUP BY d.sid;";
  
  // Create view.
  db_query($sql);
  
  $output =  _webform_export_submissions_all($node) .'<br />';
  $output .= _webform_export_submissions_unique($node) .'<br />';
  $output .= _webform_export_submissions_unique_optin($node) .'<br />';
  $output .= "Export complete.";
  
  // Drop view.
  db_query("DROP VIEW webform_export_submissions_view_". $node->nid);
  
  return $output;
}

/**
 * Export all submissions.
 */
function _webform_export_submissions_all($node) {
  
  // Set filename with !nid_all-!date.csv
  $file = strtr('!nid_all-!date.csv', array('!nid' => $node->nid, '!date' => date(WEBFORM_FILE_DATE_FORMAT)));
  $tmp = WEBFORM_TMP_DIR . $file;
  $location = WEBFORM_EXPORT_DIR . $node->nid .'/'. $file;
  
  $sql =  "SELECT * FROM webform_export_submissions_view_". $node->nid;
  $sql .= " INTO OUTFILE '". $tmp ."' FIELDS TERMINATED BY ',' ENCLOSED BY '\"' LINES TERMINATED BY '\n';";
  
  if ( db_query($sql) ) {
    _webform_export_move_file($node, $tmp, $location);
  }
  
  return $file;
}

/**
 * Export unique submissions.
 */
function _webform_export_submissions_unique($node) {
  
  // Set filename with !nid_all-!date.csv
  $file = strtr('!nid_unique-!date.csv', array('!nid' => $node->nid, '!date' => date(WEBFORM_FILE_DATE_FORMAT)));
  $tmp = WEBFORM_TMP_DIR . $file;
  $location = WEBFORM_EXPORT_DIR . $node->nid .'/'. $file;
  
  $sql =  "SELECT * FROM webform_export_submissions_view_". $node->nid;
  $sql .= " GROUP BY email";
  $sql .= " INTO OUTFILE '". $tmp ."' FIELDS TERMINATED BY ',' ENCLOSED BY '\"' LINES TERMINATED BY '\n';";
  
  if ( db_query($sql) ) {
    _webform_export_move_file($node, $tmp, $location);
  }
  
  return $file;
}

/**
 * Export unique optin submissions.
 */
function _webform_export_submissions_unique_optin($node) {
  
  // Set filename with !nid_unique_client_optin-!date.csv
  $file = strtr('!nid_unique_client_optin-!date.csv', array('!nid' => $node->nid, '!date' => date(WEBFORM_FILE_DATE_FORMAT)));
  $tmp = WEBFORM_TMP_DIR . $file;
  $location = WEBFORM_EXPORT_DIR . $node->nid .'/'. $file;
  
  $sql =  "SELECT * FROM webform_export_submissions_view_". $node->nid;
  $sql .= " WHERE opt_in_sponsor = 1";
  $sql .= " GROUP BY email";
  $sql .= " INTO OUTFILE '". $file ."' FIELDS TERMINATED BY ',' ENCLOSED BY '\"' LINES TERMINATED BY '\n';";
  
  if ( db_query($sql) ) {
    _webform_export_move_file($node, $tmp, $location);
  }
  
  return $file;
}

/**
 * Move an exported file.
 */
function _webform_export_move_file($node, $source, $dest) {
  
  // Check if folder exists.
  if ( !file_exists(WEBFORM_EXPORT_DIR . $node->nid) ) {
    mkdir(WEBFORM_EXPORT_DIR . $node->nid, 0777, TRUE);
  }
  
  // Move temp file.
  rename($source, $dest);
}

/**
 * Get a webforms components as an array.
 */
function _webform_export_get_components($node) {
  // Get components that are associated with the node.
  $result = db_query("SELECT form_key, name FROM {webform_component} WHERE nid = %d ORDER BY cid", $node->nid);
  
  $components = array();
  while ( $component = db_fetch_object($result) ) {
    $components[] = $component;
  }
  
  return $components;
}

