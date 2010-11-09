<?php

/**
 * @file
 * Display plugin for dynamic OWS
 *
 * @ingroup openlayers
 */

class dowsc_display extends views_plugin_display {
  function uses_breadcrumb() { return FALSE; }
  function get_style_type() { return 'dowsc_display'; }

  function execute() {
    return $this->view->render();
  }

  function preview() {
    return '<pre>' . check_plain($this->view->render()) . '</pre>';
  }

  /**
   * Instead of going through the standard views_view.tpl.php, delegate this
   * to the style handler.
   */
  function render() {
    return $this->view->style_plugin->render($this->view->result);
  }

  function option_definition() {
    $options = parent::option_definition();

    // Prevent the title from being translated at the view level
    $options['title']['translatable'] = 0;

    $options['displays'] = array('default' => array());

    // Overrides for standard stuff:
    $options['style_plugin']['default'] = 'dowsc_style';
    $options['row_plugin']['default'] = '';
    $options['defaults']['default']['row_plugin'] = FALSE;
    $options['defaults']['default']['row_options'] = FALSE;
    return $options;
  }

  function options_summary(&$categories, &$options) {
    parent::options_summary($categories, $options);
    unset($options['header']);
    unset($options['footer']);
    unset($options['empty']);
    unset($options['exposed_block']);
    unset($options['analyze-theme']);
  }
}
