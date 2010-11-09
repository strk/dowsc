<?php

/**
 * @file
 * OpenLayers Views Vector Layer Type
 */
class dowsc_layer extends openlayers_layer_type {
  /**
   * Provide initial values for options.
   */
  function options_init() {
    return array(
      //'layer_type' => 'dowsc_layer',
      //'layer_handler' => 'wms'
    );
  }

  /**
   * Options form which generates layers
   */
  function options_form() {
    return array();
  }

  function get_base_url() {
    return 'http://lafont.gvm.ies.jrc.it/maps/mapserv.cgi?map=../mapfiles/acpmap_static.map&';
  }

  function getViewData($view_info) {
    $view_name = $view_info['view'];
    $display_name = $view_info['display'];

    if ($view = views_get_view($view_name)) {
      $args = array();
      // Retrieve args/filters from current view
      // to pass on to our vector layer.
      $current_view = views_get_current_view();
      if (isset($current_view)) {
        $args = $current_view->args;
        if (isset($current_view->exposed_input)) {
          $view->set_exposed_input($current_view->exposed_input);
        }
      }

      $data = $view->execute_display($display_name, $args);

      // Get dynamic title from view
      //$this->title = $view->get_title();

      $view->destroy();
    }
    return isset($data) ? $data : array();
  }

  function setupLayer(&$conf) {

    $config = $this->getViewData($conf['views']);

    $wms_cfg = $config['wmscfg'];

    $layers = is_array($wms_cfg['layers']) ?
        array_keys($wms_cfg['layers']) : array();
    $wmsurl = base_path() . drupal_get_path('module', 'dowsc')
            . '/img/bomb.png';

    // We expect a single layer
    $abstract = $wms_cfg['layers'][$layers[0]]['abstract'];

    try {
      $wmsurl = dowsc_get_wms_base_url($wms_cfg);
      //echo "WMSURL: $wmsurl";
    } catch (Exception $e) {
      watchdog('dowsc_layer', $e->getMessage(), array(), WATCHDOG_ERROR);
    }

    // Specify layers
    $conf['options']['layers'] = $layers;

    // And base url
    $conf['base_url'] = $wmsurl; 

    // Add abstract
    $conf['params']['abstract'] = $abstract;

    // Let view params set/override layer params
    $conf['params'] = array_merge($conf['params'], $config['params']);

    // Cleanup unneeded things
    unset($conf['views']);
    unset($conf['baselayer']);
  }

  /**
   * Render.
   */
  function render(&$map) {

    // TODO: tweak $this->data, must have:
    //
    //  - layer_handler (string, for JS class handling this)
    //  - isBaseLayer (bool)
    //  - projection (array, with string)
    //  - base_url (string)
    //  - params 
    //     - buffer
    //     - ratio
    //     - singleTile
    //  - options
    //     - layers
    $this->data['layer_handler'] = 'wms';
    $this->data['params'] = array(
      'queryable' => true,
      'hasLegend' => true,
      'isBaseLayer' => false, // this is for OpenLayers
      'buffer' => 0,
      'ratio' => 1,
      'singleTile' => true, 
    );
    $this->data['options'] = array(
      'layers' => array('base'),
      //'maxExtent' => array(-130, -90, 230, 90), // needed ?
      'transparent' => true, 
      'exceptions' => 'application/vnd.ogc.se_inimage',
      'format' => 'image/png',
    );

    $this->setupLayer($this->data);

    //echo "<PRE>data: "; var_dump($this->data); echo "</PRE>";

    drupal_add_js(drupal_get_path('module', 'openlayers')
      . '/includes/layer_types/js/wms.js');

  }
}
