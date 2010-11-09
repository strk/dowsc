<?php

require_once "dowsc_style.inc";

class dowsc_style_chart_AdvancedFieldRenderer implements AbstractExtractor {
  function __construct($handler) {
    $this->handler = $handler;
  }
  function extract($item) {
    return $this->handler->advanced_render($item);
  }
}

class dowsc_style_chart extends dowsc_style {
  function option_definition() {
    $options = parent::option_definition();
    $options['nclasses'] = array('default' => 5);
    $options['hue'] = array('default' => 0.1);
    $options['minsize'] = array('default' => '20');
    $options['maxsize'] = array('default' => '50');
    $options['rendered_slicing'] = array('default' => 1);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $fields = $this->getFieldOptions();

    $form['slice_field'] = array(
        '#type' => 'select',
        '#title' => t('Slicing field'),
        '#description' => t('Specify the field that will be used to determine chart composition (one slice per distinct value).'),
        '#options' => $fields,
        '#default_value' => $this->options['slice_field'],
      );

    $form['rendered_slicing'] = array(
        '#type' => 'checkbox',
        '#title' => t('Use rendered value of slicing field as slice name.'),
        '#description' => t('If checked, the value taken in consideration for slicing is the _rendered_ value, that is the one affected by any rewrite you might have defined for the field'),
        '#default_value' => $this->options['rendered_slicing'],
      );

    $form['slice_colors'] = array(
        '#type' => 'textarea',
        '#title' => t('Slice colors'),
        '#description' => t('Specify the color of each slice. One slice per line, in the form "&lt;name&gt;|&lt;color&gt;". Slice name is the slicing field value (rendered, if checked). Color is in #rrggbb form. Example:')
        . "<br>A|#FF0000<br>B|#00FF00<br>C|#0000FF",
        '#default_value' => $this->options['slice_colors'],
      );

    $form['minsize'] = array(
        '#type' => 'textfield',
        '#title' => t('Min pie radius'),
        '#description' => t('Specify the minimun radius of the pie chart, in pixels. Actual radius will be proportional to the sum of slice values'),
        '#default_value' => $this->options['minsize'],
      );

    $form['maxsize'] = array(
        '#type' => 'textfield',
        '#title' => t('Min pie radius'),
        '#description' => t('Specify the minimun radius of the pie chart, in pixels. Actual radius will be proportional to the sum of slice values'),
        '#default_value' => $this->options['maxsize'],
      );

  }

  function getSlicingClassifier($agg) {
    $field_id = $this->options['slice_field'];
    if ( empty($field_id) ) {
      throw new Exception("dowsc_style_chart: slice_field not set in view options");
    }

    //echo "Slicing on: " . $field_id . "\n";
    $field_name = $this->getFieldAlias($field_id);

    $rendered_slicing = $this->options['rendered_slicing'];

    if ( $rendered_slicing ) {
      $handler = $this->getFieldHandler($field_id);
      $extractor = new dowsc_style_chart_AdvancedFieldRenderer($handler);
    } else {
      $extractor = new ObjectPropValueExtractor($field_name);
    }

    $classifier = new SingleExtractedValueClassifier($agg,$extractor);
    $factory = new ClassifyingClassFactory($classifier);

    return $factory;
  }

 
  // Return an array of field "labels" ordered after the given field "values"
  function findFieldLabels($fieldvalues) {

    // When we render, there's no point in messing with allowed values
    if ( $this->options['rendered_slicing'] ) return $fieldvalues;

    #echo '<pre>'; echo "Fieldnames: " . join(',', $fieldvalues) . "\n";

    $slicing_field_id = $this->options['slice_field'];

    // Use allowed values, if applicable
    // TODO: use rewritten value instead !!
    // (so it's possible to rewrite from view config)!!
    $finfo = $this->getFieldInfo($slicing_field_id);
    $args = $finfo['access arguments'];
    $args = $args[1]['allowed_values'];
    $arg_lines = split("\n", $args);
    $val_names = array();
    foreach ($arg_lines as $line) {
      $idval = explode("|", chop($line));
      $val_names[trim($idval[0])] = trim($idval[1]);
    }

    $fieldlabels = array();
    for ($i=0; $i<count($fieldvalues); ++$i) {
      $nam = $fieldvalues[$i];
      if ( $val_names[$nam] ) $nam = $val_names[$nam];
      $fieldlabels[] = $nam;
    }

    return $fieldlabels;
  }

  // Get configured palette (trimming slice names to help matching)
  function getConfigPalette() {

    $palette = array();

    $cfg_slice_colors = trim($this->options['slice_colors']);
    if ( empty($cfg_slice_colors) ) return $palette;

    $cfg_lines = split("\n", $cfg_slice_colors);
    foreach ($cfg_lines as $line) {
      $idval = explode("|", chop($line));
      $palette[trim($idval[0])] = trim($idval[1]);
    }

    return $palette;
  }

  /// Return palette extract for given field values.
  // 
  /// Unkonwn field values will be assigned a full-black
  /// color entry.
  ///
  function extractPalette($fieldvalues) {

    $palette = array();

    $palette_config = $this->getConfigPalette();

    // Empty config means no willint to specify any color
    if ( empty($palette_config) ) return $palette;

    $nocolor = '#000000';
    $missing = array();
    $found = 0;

    for ($i=0; $i<count($fieldvalues); ++$i) {
      $nam = trim($fieldvalues[$i]);
      if ( isset($palette_config[$nam]) ) {
        $palette[] = $palette_config[$nam];
        $found++;
      } else {
        $missing[] = $nam;
        $palette[] = $nocolor;
      }
    }

    if ( ! empty($missing) ) {
        watchdog('dowsc',
          "Piechart slices with missing color specification: !missing",
          array('!missing' => implode(', ', $missing)),
          WATCHDOG_ERROR);
    }

    return $palette;
  }

  function buildData($projects) {

    $by = $this->options['level'];
    $data = array();
    $fieldnames = array();
    $palette = array();

    try {
      $factory = $this->getClassFactory();
      $factory = $this->getSlicingClassifier($factory);
      $classifier = $this->getClassifier($factory);

      foreach($projects as $project) {
          $classifier->put($project);
      }

      $classes = $classifier->getClasses();

      $fieldnames_idx = array();

      foreach ($classes as $name => $class) {
          $slices = $class->getState();
          foreach ($slices as $slice_name => $slice_class) {
            $fieldnames_idx[$slice_name] = 1;
          }
      }

      $fieldnames = array_keys($fieldnames_idx);
      $fieldlabels = $this->findFieldLabels($fieldnames);

      // Sort alphabetically by field label
      $labels = array();
      for ($i=0; $i<count($fieldlabels); ++$i) 
        $labels[$fieldnames[$i]] = $fieldlabels[$i];
      asort($labels);
      $fieldnames = array_values($labels);
      $fieldlabels = array_keys($labels);


      foreach ($classes as $name => $class) {
          $slices = $class->getState();
          $data[$name] = array();
          for ($i=0; $i<count($fieldnames); ++$i) {
            $data[$name][] = isset($slices[$fieldnames[$i]]) ?
              $slices[$fieldnames[$i]]->getState() : 0; 
          }
      }

      // Use configured palette 
      $palette = $this->extractPalette($fieldlabels);

      //echo "<pre>Data: "; var_dump($data) . "\n"; die();
    } catch (Exception $e) {
      watchdog('dowsc', $e->getMessage(), array(), WATCHDOG_ERROR);
    }

    $minsize = $this->options['minsize'];
    $maxsize = $this->options['maxsize'];
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
            'type' => 'piechart',
            'style' => array(
              'minsize' => $minsize,
              'maxsize' => $maxsize
            ),
            'values' => $data,
            'fieldnames' => $fieldlabels,
          )
        )
      )
    );

    if ( ! empty($palette) ) {
      $cfg['layers'][$by]['classification']['palette'] = $palette;
    }

    return $cfg;
  }

  function getLayerParams() {
    $params = parent::getLayerParams();
    $params['singleTile'] = true;
    return $params;
  }

}
