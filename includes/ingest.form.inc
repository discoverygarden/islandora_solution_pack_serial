<?php
/**
 * @file
 * Holds ingest steps for the Islandora Serial solution pack.
 */

/**
 * The ingest form to add intermediate children.
 *
 * @param array $form
 * @param array $form_state
 * @return array
 */
function islandora_serial_object_add_intermediate_form($form, &$form_state) {
  module_load_include('inc', 'islandora_serial_object', 'includes/utilities');
  if (isset($form_state['triggering_element']) && end($form_state['triggering_element']['#parents']) == 'add_intermediate_object') {
   if (isset($form_state['values']['table_wrapper']['islandora_serial_object_intermediates'])) {
      $existing_intermediates = $form_state['values']['table_wrapper']['islandora_serial_object_intermediates'];
      $existing_intermediates = islandora_serial_object_sort_intermediates($existing_intermediates);
    }
    else {
      $existing_intermediates = array();
    }
    $new_intermediate_id = count($existing_intermediates) + 1;
    $existing_intermediates['intermediate_' . $new_intermediate_id] = array(
      'id' => $new_intermediate_id,
      'pid' => 0,
    );
    $form_state['islandora_serial_object_intermediates'] = $existing_intermediates;
  }
  $form['#tree'] = TRUE;
  $form['table_wrapper'] = array(
    '#prefix' => '<div id="islandora-serial-object-intermediate-table-wrapper">',
    '#suffix' => '</div>',
    'add_intermediate_object' => array(
      '#type' => 'button',
      '#value' => t('Add another'),
      '#ajax' => array(
        'callback' => 'islandora_serial_add_intermediate_object',
        'wrapper' => 'islandora-serial-object-intermediate-table-wrapper',
      ),
      '#weight' => 10,
    ),
  );
  islandora_serial_object_build_intermediates($form['table_wrapper'], $form_state, 'ingest');
  return $form;
}

function islandora_serial_add_intermediate_object($form, &$form_state) {
  return $form['table_wrapper'];
}