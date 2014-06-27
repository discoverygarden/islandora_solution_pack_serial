<?php
/**
 * @file
 * Handles admin settings for the serial solution pack.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_serial_object_settings_form(array $form, array &$form_state) {
  $form['islandora_serial_object_display_stubs_pdfs_in_tree'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display PDFs and sections in the jsTree view?'),
    '#default_value' => variable_get('islandora_serial_object_display_stubs_pdfs_in_tree', FALSE),
  );
  return system_settings_form($form);
}
