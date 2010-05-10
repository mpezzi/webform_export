<?php

/**
 * Override variables in webform-export-status.tpl.php
 */
function template_preprocess_webform_export_status(&$vars, $hook) {
  static $added;
  
  if ( !isset($added) ) {
    drupal_add_css(drupal_get_path('module', 'webform_export') .'/webform_export.css');
    drupal_add_css(drupal_get_path('module', 'webform_export') .'/js/jquery-visualize/visualize.jQuery.css');
    drupal_add_js(drupal_get_path('module', 'webform_export') .'/js/jquery-visualize/excanvas.compiled.js');
    drupal_add_js(drupal_get_path('module', 'webform_export') .'/js/jquery-visualize/visualize.jQuery.js');
    drupal_add_js(drupal_get_path('module', 'webform_export') .'/webform_export.js');
    $added = TRUE;
  }
  
  $vars['values'] = array(
    'total' => _webform_export_submissions_count_total($vars['node']),
    'unique' => _webform_export_submissions_count_unique($vars['node']),
    'optin' => _webform_export_submissions_count_optin($vars['node']),
  );
  
  $options = array(
    'type' => 'bar',
    'width' => 500,
    'height' => 200,
    'pieMargin' => 10,
  );
  
  drupal_add_js(array('webform_export' => array('.webform-export-status table' => $options)), 'setting');
}