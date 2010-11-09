<?php

require_once "dowsc_style.inc";

class dowsc_style_theme extends dowsc_style {
  function option_definition() {
    $options = parent::option_definition();
    $options['nclasses'] = array('default' => 5);
    $options['hue'] = array('default' => 0.1);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);


    $fields = $this->getFieldOptions();

    $form['agg_val_name'] = array(
        '#type' => 'textfield',
        '#title' => t('Aggregated value name'),
        '#description' => t('Specify the name you want to give to the aggregated value.'),
        '#default_value' => $this->options['agg_val_name'],
      );

    $form['nclasses'] = array(
        '#type' => 'textfield',
        '#title' => t('Number of classes'),
        '#description' => t('Select the number of classes to use in classification.'),
        '#default_value' => $this->options['nclasses'],
      );

    $form['hue'] = array(
        '#type' => 'textfield',
        '#title' => t('Color hue'),
        '#description' => t('Select the color hue to use in thematization.'),
        '#default_value' => $this->options['hue'],
      );
  }

  function buildData($projects) {

    $by = $this->options['level'];
    $data = array();

    try {
      $factory = $this->getClassFactory();
      $classifier = $this->getClassifier($factory);

      foreach($projects as $project) {
          $classifier->put($project);
      }

      $classes = $classifier->getClasses();

      foreach ($classes as $name => $class) {
          $value = $class->getState();
          $data[$name] = $value;
      }
    } catch (Exception $e) {
      watchdog('dowsc', $e->getMessage(), array(), WATCHDOG_ERROR);
    }

    $nclasses = $this->options['nclasses'];
    $hue = $this->options['hue'];
    $abstract = $this->options['abstract'];

    $cfg = array(
      'layers' => array(
        $by => array(
          // NOTE: 'abstract' this is not supported by DOWS, at
          //       time of writing, but will be used by
          //       dowsc_layer
          //
          'abstract' => $abstract,
          'featureinfo' => $this->getFeatureInfoConfig(),
          'classification' => array(
            'values' => $data,
            'nclasses'  => $nclasses,
            'style' => array (
              'hue' => $hue,
              'value' => 0.9,
            )
          )
        )
      )
    );

    if ( !empty($this->options['agg_val_name']) ) {
      $cfg['layers'][$by]['classification']['fieldnames'] =
        array($this->options['agg_val_name']);
    }

    return $cfg;
  }

  function getLayerParams() {
    $params = parent::getLayerParams();
    $params['singleTile'] = false;
    return $params;
  }

}
