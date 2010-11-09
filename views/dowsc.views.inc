<?php
// $Id: openlayers_views.views.inc,v 1.3.2.3 2010/02/03 19:08:13 tmcw Exp $

/**
 * @file
 * This file holds main views functions for the openlayers_views module.
 *
 * @ingroup openlayers
 */

/**
 * Implementation of hook_views_plugins().
 */
function dowsc_views_plugins() {
  return array(
    'module' => 'openlayers_views',
    'display' => array(
      'dowsc_display' => array(
        'title' => t('Dynamic OWS data'),
        'help' => t('Prepares data for configuration of dynamic OWS layers.'),
        'handler' => 'dowsc_display',
        'path' => drupal_get_path('module', 'dowsc') .'/views',
        'uses hook menu' => FALSE,
        'use ajax' => FALSE,
        'use pager' => FALSE,
        'accept attachments' => FALSE,
        'admin' => t('Dynamic OWS data'),
      ),
    ),
    'style' => array(
      'dowsc_style_theme' => array(
        'title' => t('Thematic WMS layer'),
        'help' => t('Provides a dynamic thematic layer to OpenLayers module.'),
        'handler' => 'dowsc_style_theme',
        'path' => drupal_get_path('module', 'dowsc') .'/views',
        //'theme' => 'dwms',
        //'theme path' => drupal_get_path('module', 'dowsc') .'/theme',
        'uses fields' => TRUE,
        'uses row plugin' => FALSE,
        'uses options' => TRUE,
        'uses grouping' => TRUE,
        'type' => 'dowsc_display',
        //'type' => 'normal',
        'even empty' => TRUE,
      ),
      'dowsc_style_chart' => array(
        'title' => t('Charting WMS layer'),
        'help' => t('Provides a dynamic chart layer to OpenLayers module.'),
        'handler' => 'dowsc_style_chart',
        'path' => drupal_get_path('module', 'dowsc') .'/views',
        //'theme' => 'dwms',
        //'theme path' => drupal_get_path('module', 'dowsc') .'/theme',
        'uses fields' => TRUE,
        'uses row plugin' => FALSE,
        'uses options' => TRUE,
        'uses grouping' => TRUE,
        'type' => 'dowsc_display',
        //'type' => 'normal',
        'even empty' => TRUE,
      ),
    ),
  );
}
