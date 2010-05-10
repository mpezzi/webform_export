<?php

define('WEBFORM_EXPORT_FIELDS',           1);
define('WEBFORM_EXPORT_FILE_DATE_FORMAT', 'd-m-Y--H-i-s');


/**
 * Download form.
 */
function webform_export_download_form(&$form_state, $node) {
  return array(
    'node' => array(
      '#type' => 'value',
      '#value' => $node,
    ),
    'criteria' => array(
      '#type' => 'select',
      '#title' => t('What results would you like to export?'),
      '#options' => array(
        'all' => t('All'),
        'unique' => t('Unique'),
        'unique_optin' => t('Unique Opt-in'),
      ),
    ),
    'delimiter' => array(
      '#type' => 'select',
      '#title' => t('Delimited text format'),
      '#default_value' => variable_get('webform_export_csv_delimiter', ','),
      '#options' => array(
        ',' => t('Comma (,)'),
        '\t' => t('Tab (\t)'),
        ';'  => t('Semicolon (;)'),
        ':'  => t('Colon (:)'),
        '|'  => t('Pipe (|)'),
        '.'  => t('Period (.)'),
        ' '  => t('Space ( )'),
      ),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Download'),
    ),
  );
}

/**
 * Download form callback.
 */
function webform_export_download_form_submit(&$form, &$form_state) {
  $node = $form_state['values']['node'];
  $critera = $form_state['values']['criteria'];
  $delimiter = $form_state['values']['delimiter'];
  
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
  
  // Find the appropriate export criteria.
  $function = 'webform_export_download_'. $critera;
  if ( function_exists($function) ) {
    $function($node, $delimiter, $form_state['values']);
  }
  
  // Drop view.
  db_query("DROP VIEW webform_export_submissions_view_". $node->nid);
}

/**
 * Generate a CSV file containing all submissions for a webform node.
 */
function webform_export_download_all($node, $delimiter = ',', $options = array()) {
  $file = _webform_export_filename($node, '!nid_all-!date.csv');
  _webform_export_set_header($file);
  _webform_export_as_csv("SELECT * FROM webform_export_submissions_view_". $node->nid, $delimiter, WEBFORM_EXPORT_FIELDS);
}

/**
 * Generate a CSV file containing unique submissions for a webform node.
 */
function webform_export_download_unique($node, $delimiter = ',', $options = array()) {
  $file = _webform_export_filename($node, '!nid_unique-!date.csv');
  _webform_export_set_header($file);
  _webform_export_as_csv("SELECT * FROM webform_export_submissions_view_". $node->nid ." GROUP BY email;", $delimiter, WEBFORM_EXPORT_FIELDS);
}

/**
 * Generate a CSV file containing unique opt-in submissions for a webform node.
 */
function webform_export_download_unique_optin($node, $delimiter = ',', $options = array()) {
  $file = _webform_export_filename($node, '!nid_unique_optin-!date.csv');
  _webform_export_set_header($file);
  _webform_export_as_csv("SELECT * FROM webform_export_submissions_view_". $node->nid ." WHERE opt_in_sponsor = 1 GROUP BY email;", $delimiter, WEBFORM_EXPORT_FIELDS);
}


/**
 * Output CSV version of data.
 */
function _webform_export_as_csv($sql, $delimiter, $fields = TRUE) {
  $result = db_query($sql);
  
  while ( $row = db_fetch_array($result) ) {
    if ( $fields ) {
      echo "\"". implode("\"". $delimiter ."\"", array_keys($row)) ."\"\n";
      $fields = FALSE;
    }
    
    echo "\"". implode("\"". $delimiter ."\"", $row) ."\"\n";
  }
}

/**
 * Return file name based on node information.
 */
function _webform_export_filename($node, $string) {
  return strtr($string, array('!nid' => $node->nid, '!date' => date(WEBFORM_EXPORT_FILE_DATE_FORMAT)));
}

/**
 * Return an array of webform components.
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

/**
 * Set the header.
 */
function _webform_export_set_header($file) {
  header('Content-type: application/octet-stream');
  header('Content-Disposition: attachment; filename="'. $file .'"');
}
