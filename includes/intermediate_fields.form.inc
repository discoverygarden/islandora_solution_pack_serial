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
 * @param string $form_type
 *   The type whether it be at ingest or on the manage screen.
 *
 * @return array
 *   A renderable array containing the Drupal form.
 */
function islandora_serial_object_create_fields_for_intermediates_form($form, &$form_state, $form_type = 'ingest') {
  form_load_include($form_state, 'inc', 'islandora_serial_object', 'includes/intermediate_fields.form');
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
      '#type' => 'submit',
      '#value' => t('Remove selected'),
      '#submit' => array('islandora_serial_object_intermediate_rebuild'),
    ),
    'add_level' => array(
      '#type' => 'submit',
      '#value' => t('Add a level'),
      '#submit' => array('islandora_serial_object_intermediate_rebuild'),
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
  elseif ($form_type == 'manage') {
    $object = menu_get_object('islandora_object', 2);
    if (isset($object['SERIAL-LEVELS'])) {
      $levels = json_decode($object['SERIAL-LEVELS']->content);
      $next_level = 0;
      foreach ($levels as $level) {
        $rows[$level->level] = array(
          'level' => $level->level,
          'checkbox' => FALSE,
          'type' => $level->type,
          'caption' => $level->caption,
        );
        $next_level++;
      }
      // Set the existing level so we have a starting point to add to.
      $form_state['islandora_serial_object_intermediate_level'] = $next_level;
    }
    islandora_serial_object_construct_intermediate_levels($form['intermediate_table']['rows'], $rows);
  }
  $form_state['islandora_serial_object_intermediate_levels'] = $rows;
  if (empty($rows)) {
    $form['intermediate_table']['remove_selected']['#access'] = FALSE;
  }
  $form_state['islandora_serial_object_intermediate_type'] = $form_type;
  if ($form_type != 'ingest') {
    module_load_include('inc', 'islandora_serial_object', 'includes/manage');
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Update'),
      '#validate' => array('islandora_serial_object_create_fields_for_intermediates_form_validate'),
      '#submit' => array(
        'islandora_serial_object_create_fields_for_intermediates_form_submit',
      ),
    );
    if (isset($form_state['islandora_serial_object_intermediate_levels']) && !empty($form_state['islandora_serial_object_intermediate_levels'])) {
      $form['submit']['#value'] = t('Next');
      $form['submit']['#submit'][] = 'islandora_serial_object_manage_steps_rebuild';
    }
  }
  else {
    // Need to go re-build the ingest steps because we have a conditional step
    // that could be fired that would be the last step.
    drupal_static_reset('islandora_ingest_form_get_steps');
  }
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
        if (trim($values['type']) === '') {
          form_error($form['intermediate_table']['rows'][$key]['type'], t('The type field cannot be empty!'));
        }
      }
    }
  }
}

/**
 * Rebuilds the form when either the add or remove button is selected.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_serial_object_intermediate_rebuild(&$form, &$form_state) {
  $form_state['rebuild'] = TRUE;
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
  if (isset($form_state['values']['intermediate_table']['rows'])) {
    module_load_include('inc', 'islandora', 'includes/ingest.form');
    if ($form_state['islandora_serial_object_intermediate_type'] == 'ingest') {
      $object = islandora_ingest_form_get_object($form_state);
    }
    else {
      $object = menu_get_object('islandora_object', 2);
      $form_state['islandora_serial_object_created_levels'] = TRUE;
    }
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
    $new_ds = TRUE;
    if (!isset($object['SERIAL-LEVELS'])) {
      $ds = $object->constructDatastream('SERIAL-LEVELS', 'M');
      $ds->label = 'Hierarchical Information for the Serial Object';
      $ds->mimetype = 'application/json';
    }
    else {
      $ds = $object['SERIAL-LEVELS'];
      $new_ds = FALSE;
    }
    $ds->setContentFromString($json_levels);
    if ($new_ds) {
      $object->ingestDatastream($ds);
    }
  }
  elseif ($form_state['islandora_serial_object_intermediate_type'] == 'manage') {
    // We have no levels, let's see if we need to delete the existing ds.
    $object = menu_get_object('islandora_object', 2);
    if (isset($object['SERIAL-LEVELS'])) {
      $object->purgeDatastream('SERIAL-LEVELS');
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
  return $form;
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
