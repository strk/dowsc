<?php

/* Base class for dowsc view style plugins */
abstract class dowsc_style extends views_plugin_style {
  function option_definition() {
    $options = parent::option_definition();
    $options['level'] = array("default" => "country");
    $options['agg_func'] = array('default' => 'count');
    $options['abstract'] = array('default' => '');
    return $options;
  }

  function getFieldOptions() {
    $handlers = $this->display->handler->get_handlers('field');
    $fields = array();
    foreach ($handlers as $field_id => $handler) {
      $fields[$field_id] = $handler->definition['title'];
    }
    return $fields;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['grouping']['#title'] = t('Layer features identifier');
    $form['grouping']['#description'] = t('Specify the field by which the OWS layer features are identified. This will be ISO2 code for countries and ID for regions.');
    $form['grouping']['#default_value'] = $this->options['grouping']; 
    $form['grouping']['#required'] = true;

    $levels = array('country' => 'Country', 'region' => 'Region');
    $form['level'] = array(
        '#type' => 'select',
        '#title' => t('Layer'),
        '#description' => t('Choose a service layer to configure.'),
        '#options' => $levels,
        '#default_value' => $this->options['level'],
      );

    $fields = $this->getFieldOptions();

    $form['agg_field'] = array(
        '#type' => 'select',
        '#title' => t('Aggregated field'),
        '#description' => t('Specify the field containing the values you want to pass to the aggregator function.'),
        '#options' => $fields,
        '#default_value' => $this->options['agg_field'],
      );

    $agg_funx = array('count' => 'Count', 'sum' => 'Sum');
    $form['agg_func'] = array(
        '#type' => 'select',
        '#title' => t('Aggregator function'),
        '#description' => t('Specify the type of aggregation you want to perform on the aggregated field.'),
        '#options' => $agg_funx,
        '#default_value' => $this->options['agg_func'],
      );

    $form['abstract'] = array(
        '#type' => 'textarea',
        '#title' => t('Layer abstract'),
        '#description' => t('Specify an abstract for the resulting layer'),
        '#default_value' => $this->options['abstract']
      );

    $form['info_link_href'] = array(
        '#type' => 'textfield',
        '#title' => t('Feature info link'),
        '#description' => t('Specify an URL to link features to in GetFeatureInfo HTML responses. The URL can contain the following keyword substitutions:')
          . '<ul>'
          . '<li>'
          . '[BASEURL] ' . t('Base url to this drupal site')
          . '</li>'
          . '<li>'
          . '[ID] '
            . t('Feature identifier value (see Layer Feature identifier)')
          . '</li>'
          . '<li>'
          . '[NAME] ' . t('Feature name')
          . '</li>'
          . '</ul>'
          . t('<b>NOTE</b> that case matters!')
        ,
        '#default_value' => $this->options['info_link_href']
      );

    $form['info_link_name'] = array(
        '#type' => 'textfield',
        '#title' => t('Name of feature info link'),
        '#description' => t('Give a name to use for the feature information link.'),
        '#default_value' => $this->options['info_link_name']
      );

  }

  function render($projects) {

    //echo "<PRE>Projects[0]:\n"; var_dump($projects[0]);

    // Check for live preview.
    if (!empty($this->view->live_preview)) {
      return t('DOWS views are not compatible with live preview.');
    }

    // If we call getData before style is configured
    // an exception will be thrown due to not found
    // grouping name.
    // FIXME: make sure defaults are always used
    //        or just catch the exception and
    //        log to watchdog
    $data = $this->getData($projects);

    //echo "\nData\n"; var_dump($data['data']);


    return $data;
  }

  function getLayerParams() {
    return array();
  }

  function getData($projects) {

    // TODO: use some caching here
    $wmscfg = $this->buildData($projects);

    $params = $this->getLayerParams();

    $data = array(
      'wmscfg' => $wmscfg,
      // NOTE: can add params here
      'params' => $params
    );

    return $data;
  }

  function getFieldInfo($field_id) {
    if ( empty($field_id) ) {
      throw new Exception("dowsc_style_chart: "
         . " getFieldInfo called with empty arg");
    }

    if ( empty($this->field_info) ) {
      // Get list of fields in this view
      $handlers = $this->display->handler->get_handlers('field');
      $fields = array();
      foreach ($handlers as $id => $handler) {
        $fields[$id] = $handler->definition;
        $fields[$id]['field_alias'] = $handler->field_alias;
      }
      $this->field_info = $fields;
    }

    $finfo = $this->field_info[$field_id];
    if ( empty($finfo) ) {
      throw new Exception("Could not find info for field " . $field_id);
    }

    return $finfo;
  }

  function getFieldAlias($field_id) {

    $finfo = $this->getFieldInfo($field_id);
    $ret = $finfo['field_alias'];

    if ( empty($ret) ) {
      throw new Exception("Could not find alias of field " . $field_id);
    }
    return $ret;
  }

  function getFieldHandler($field_id) {

    if ( empty($field_id) ) {
      throw new Exception("dowsc_style_chart: "
         . " getFieldHandler called with empty arg");
    }

    $handlers = $this->display->handler->get_handlers('field');
    return $handlers[$field_id];
  }


  function getClassFactory() {

    $agg = $this->options['agg_func'];

    switch ($agg) {
      case 'count':
        $classFactory = new CounterClassFactory();
        break;
      case 'sum':
        $field_id = $this->options['agg_field'];
        if ( empty($field_id) ) {
          throw new Exception("dowsc_style: agg_field not set in view options");
        }
        $field_name = $this->getFieldAlias($field_id);
        $classFactory = new ExtractedValueSummerClassFactory(
          new ObjectPropValueExtractor($field_name));
        break;
      default:
        throw new Exception("Unknown aggregator " . $agg);
    }
    return $classFactory;
  }

  function getClassifier($agg) {
    $field_id = $this->options['grouping'];
    if ( empty($field_id) ) {
      throw new Exception("dowsc_style: grouping not set in view options");
    }

    //echo "Grouping on: " . $field_id . "\n";
    $field_name = $this->getFieldAlias($field_id);

    $classifier = new SingleExtractedValueClassifier($agg, 
      new ObjectPropValueExtractor($field_name));
    return $classifier;
  }
 
  function getFeatureInfoConfig() {
    $info = array();

    if ( !empty($this->options['info_link_href']) ) {
      $href_tpl = $this->options['info_link_href'];
      // Substitute [BASEURL]
      $href_tpl = str_replace('[BASEURL]', $GLOBALS['base_url'], $href_tpl);
      $info['link_href'] = $href_tpl;
    }

    if ( !empty($this->options['info_link_name']) ) {
      $info['link_name'] = $this->options['info_link_name'];
    }

    return $info;
  }

  abstract function buildData($projects);

}


