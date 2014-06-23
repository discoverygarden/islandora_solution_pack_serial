<?php
/**
 * @file
 * Create variable intermediate field forms
 */

/**
 * Form to create variable fields for intermediate objects.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 *
 * @return array
 *   A renderable array containing the Drupal form.
 */
function islandora_serial_object_create_fields_for_intermediates_form($form, &$form_state) {
  $rows = array();
  $form['#tree'] = TRUE;
  $form['intermediate_table'] = array(
    'title' => array(
      '#markup' => t('Serial Level Configuration'),
      '#prefix' => '<h2>',
      '#suffix' => '</h2>',
    ),
    'description' => array(
      '#markup' => t("This form defines default structural values for each level of the serial object's hierarchy. These values are used when generating the MODS datastream for intermediate objects at the specified level."),
    ),
    'rows' => array(
      '#theme' => 'islandora_serial_object_intermediate_levels_table',
    ),
    '#prefix' => '<div id="islandora-serial-object-intermediate-table-level-wrapper">',
    '#suffix' => '</div>',
    'remove_selected' => array(
      '#type' => 'button',
      '#value' => t('Remove selected'),
      '#ajax' => array(
        'callback' => 'islandora_serial_object_ajax_intermediate_level',
        'wrapper' => 'islandora-serial-object-intermediate-table-level-wrapper',
      ),
    ),
    'add_level' => array(
      '#type' => 'button',
      '#value' => t('Add a level'),
      '#ajax' => array(
        'callback' => 'islandora_serial_object_ajax_intermediate_level',
        'wrapper' => 'islandora-serial-object-intermediate-table-level-wrapper',
      ),
    ),
  );
  if (isset($form_state['triggering_element'])) {
    $last_parent = end($form_state['triggering_element']['#parents']);
    $rows = isset($form_state['islandora_serial_object_intermediate_levels']) ? $form_state['islandora_serial_object_intermediate_levels'] : array();
    // Update what might have changed since the last AJAX request now.
    if (isset($form_state['values']['intermediate_table']['rows'])) {
      foreach ($form_state['values']['intermediate_table']['rows'] as $key => $values) {
        foreach ($values as $type => $value) {
          $rows[$key][$type] = $value;
        }
      }
    }
    if ($last_parent == 'add_level') {
      // Add on our new row.
      $next_level = isset($form_state['islandora_serial_object_intermediate_level']) ? $form_state['islandora_serial_object_intermediate_level'] + 1 : 1;
      $form_state['islandora_serial_object_intermediate_level'] = $next_level;
      $rows["$next_level"] = array(
        'level' => count($rows) + 1,
        'checkbox' => FALSE,
        'type' => FALSE,
        'caption' => FALSE,
      );
    }
    // Remove selected.
    elseif ($last_parent == 'remove_selected') {
      // Determine what rows need to be removed.
      if (isset($form_state['values']['intermediate_table']['rows'])) {
        $removed = FALSE;
        foreach ($form_state['values']['intermediate_table']['rows'] as $key => $values) {
          if ($values['checkbox']) {
            unset($rows[$key]);
            $removed = TRUE;
          }
        }
        // If we removed something need to update the markup for the key.
        if (!empty($rows) && $removed) {
          $count = 1;
          foreach ($rows as &$values) {
            $values['level'] = $count;
            $count++;
          }
        }
      }
    }
    islandora_serial_object_construct_intermediate_levels($form['intermediate_table']['rows'], $rows);
  }
  $form_state['islandora_serial_object_intermediate_levels'] = $rows;
  return $form;
}

/**
 * Validation for creating variable fields for intermediate objects.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_serial_object_create_fields_for_intermediates_form_validate($form, &$form_state) {
  $last_parent = end($form_state['triggering_element']['#parents']);
  if ($last_parent == 'submit' || $last_parent == 'next') {
    if (isset($form_state['values']['intermediate_table']['rows'])) {
      foreach ($form_state['values']['intermediate_table']['rows'] as $key => $values) {
        foreach ($values as $value_key => $value) {
          if ($value_key != 'checkbox' && trim($value) === '') {
            form_error($form['intermediate_table']['rows'][$key][$value_key], t('The @type field cannot be empty!', array('@type' => $value_key)));
          }
        }
      }
    }
    else {
      form_error($form['intermediate_table'], t('Must define at least one level!'));
    }
  }
}

/**
 * Submit handler for creating variable fields for intermediate objects.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_serial_object_create_fields_for_intermediates_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $object = islandora_ingest_form_get_object($form_state);
  $level_data = array();
  foreach ($form_state['values']['intermediate_table']['rows'] as $row) {
    $level_data[$row['level_value']] = array(
      'level' => $row['level_value'],
      'type' => $row['type'],
      'caption' => $row['caption'],
    );
    // Update the form state in case we previoius back.
    $form_state['islandora_serial_object_intermediate_levels'][$row['level_value']]['level'] = $row['level_value'];
    $form_state['islandora_serial_object_intermediate_levels'][$row['level_value']]['type'] = $row['type'];
    $form_state['islandora_serial_object_intermediate_levels'][$row['level_value']]['caption'] = $row['caption'];
  }
  $json_levels = json_encode($level_data);
  // Need to construct a new datastream to store these in as JSON.
  $ds = $object->constructDatastream('SERIAL-LEVELS', 'M');
  $ds->label = 'Hierarchical Information for the Serial Object';
  $ds->mimetype = 'application/json';
  $ds->setContentFromString($json_levels);
  $object->ingestDatastream($ds);
}

/**
 * Submit handler for creating variable fields for intermediate objects.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_serial_object_create_fields_for_intermediates_form_undo_submit($form, &$form_state) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $object = islandora_ingest_form_get_object($form_state);
  if (isset($object['SERIAL-LEVELS'])) {
    $object->purgeDatastream('SERIAL-LEVELS');
  }
}

/**
 * AJAX callback for intermediate level.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 *
 * @return array
 *   The portion of the form where the AJAX is to re-render.
 */
function islandora_serial_object_ajax_intermediate_level(&$form, &$form_state) {
  return $form['intermediate_table'];
}

/**
 * Helper function to construct the markup for the rows in the form.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $rows
 *   An array of arrays containing:
 *   - level: An integer denoting what level this element is currently at.
 *   - checkbox: An integer denoting whether the checkbox is checked or not.
 *   - type: The string value of the current type value for MODS.
 *   - caption: The current value for the caption. *
 */
function islandora_serial_object_construct_intermediate_levels(&$form, $rows) {
  foreach ($rows as $key => $values) {
    $form[$key] = array(
      'checkbox' => array(
        '#type' => 'checkbox',
        '#default_value' => $values['checkbox'],
      ),
      'level' => array(
        '#markup' => t('Level @level', array('@level' => $values['level'])),
      ),
      'level_value' => array(
        '#type' => 'value',
        '#value' => $values['level'],
      ),
      'type' => array(
        '#type' => 'textfield',
        '#size' => 30,
        '#default_value' => $values['type'],
      ),
      'caption' => array(
        '#type' => 'textfield',
        '#size' => 30,
        '#default_value' => $values['caption'],
      ),
    );
  }
}
