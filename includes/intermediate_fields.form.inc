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
    '#theme' => 'islandora_serial_object_intermediate_levels_table',
    '#prefix' => '<div id="islandora-serial-object-intermediate-table-level-wrapper">',
    '#suffix' => '</div>',
  );
  if (isset($form_state['triggering_element'])) {
    $last_parent = end($form_state['triggering_element']['#parents']);
    $rows = isset($form_state['islandora_serial_object_intermediate_levels']) ? $form_state['islandora_serial_object_intermediate_levels'] : array();
    // Update what might have changed since the last AJAX request now.
    if (isset($form_state['values']['intermediate_table'])) {
      foreach ($form_state['values']['intermediate_table'] as $key => $values) {
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
      if (isset($form_state['values']['intermediate_table'])) {
        $removed = FALSE;
        foreach ($form_state['values']['intermediate_table'] as $key => $values) {
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
    islandora_serial_object_construct_intermediate_levels($form['intermediate_table'], $rows);
  }

  $form['remove_selected'] = array(
    '#type' => 'button',
    '#value' => t('Remove selected'),
    '#ajax' => array(
      'callback' => 'islandora_serial_object_ajax_intermediate_level',
      'wrapper' => 'islandora-serial-object-intermediate-table-level-wrapper',
    ),
  );
  $form['add_level'] = array(
    '#type' => 'button',
    '#value' => t('Add a level'),
    '#ajax' => array(
      'callback' => 'islandora_serial_object_ajax_intermediate_level',
      'wrapper' => 'islandora-serial-object-intermediate-table-level-wrapper',
    ),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit lulul'),
  );
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
    if (isset($form_state['values']['intermediate_table'])) {
      foreach ($form_state['values']['intermediate_table'] as $key => $values) {
        foreach ($values as $value_key => $value) {
          if ($value_key != 'checkbox' && empty($value)) {
            form_error($form['intermediate_table'][$key][$value_key], t('The @type field cannot be empty!', array('@type' => $value_key)));
          }
        }
      }
    }
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
 *   -level: An integer denoting what level this element is currently at.
 *   -checkbox: An integer denoting whether the checkbox is checked or not.
 *   -type: The string value of the current type value for MODS.
 *   -caption: The current value for the caption. *
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
