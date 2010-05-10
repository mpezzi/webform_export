Drupal.behaviors.webform_export = function() {
  jQuery.each(Drupal.settings.webform_export, function(selector, options){
    $(selector).visualize(options);
    
    // Debugger.
    function log(s) {
      if ( window.console && window.console.log ) {
        window.console.log(s);
      }
    }
  });
};