<?php
/**
 * @file
 * Houses utility functions for the Serial solution pack.
 */

function islandora_serial_object_build_intermediates(&$form, &$form_state, $type) {
  $intermediates = array();
  if ($type == 'ingest') {
    $intermediates = isset($form_state['islandora_serial_object_intermediates']) ? $form_state['islandora_serial_object_intermediates'] : array();
  }
  else {
    // @TODO: Grab from existing.
  }
  $form['islandora_serial_object_intermediates']['#theme'] = 'islandora_serial_object_intermediates_table';
  foreach ($intermediates as $item) {
    $form['islandora_serial_object_intermediates']['intermediate_' . $item['id']] = array(
      'title' => array(
        '#type' => 'textfield',
      ),
      'volume' => array(
        '#type' => 'textfield',
      ),
      'issue' => array(
        '#type' => 'textfield',
      ),
      'year' => array(
        '#type' => 'textfield',
      ),
      'pid' => array(
        //'#type' => 'hidden',
        '#type' => 'textfield',
        '#length' => 3,
        '#default_value' => isset($item['pid']) ? $item['pid'] : FALSE,
        '#attributes' => array(
          'class' => array('islandora-serial-object-intermediate-pid'),
        ),
      ),
      'id' => array(
        //'#type' => 'hidden',
        '#type' => 'textfield',
        '#length' => 3,
        '#default_value' => isset($item['id']) ? $item['id'] : FALSE,
        '#attributes' => array(
          'class' => array('islandora-serial-object-intermediate-id'),
        ),
      ),
      'weight' => array(
        '#type' => 'weight',
        '#title' => t('Weight'),
        '#default_value' => isset($item['weight']) ? $item['weight'] : FALSE,
        '#delta' => 10,
        '#title_display' => 'invisible',
        '#attributes' => array(
          'class' => array('islandora-serial-object-intermediate-weight'),
        ),
      ),
      'depth' => array(
        '#type' => 'hidden',
        '#value' => isset($item['depth']) ? $item['depth'] : FALSE,
      ),
    );
  }
}

function islandora_serial_object_sort_intermediates($intermediates) {
  foreach ($intermediates as $key => $value) {
    if ($value['pid'] != 0) {
      $intermediates[$key]['depth'] = islandora_serial_object_sort_get_depth($value['pid'], $intermediates);
    }
    else {
      $intermediates[$key]['depth'] = 0;
    }
  }
  dsm($intermediates);
  return $intermediates;
}

function islandora_serial_object_sort_get_depth($pid, $intermediates, &$depth = 1) {
  $intermediate_parent_id = "intermediate_$pid";
  if ($intermediates[$intermediate_parent_id]['pid'] != 0) {
    dsm($intermediates, 'array');
    dsm($intermediate_parent_id, 'parent id');
    $depth++;
    dsm($depth, 'depth');
    islandora_serial_object_sort_get_depth($intermediates[$intermediate_parent_id]['pid'], $intermediates, $depth);
  }
  else {
    dsm($depth, 'to return');
    return $depth;
  }
}