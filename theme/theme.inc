<?php
/**
 * @file
 * Theme functions for the Islandora Serial solution pack.
 */

/**
 * Implements theme_element().
 */
function theme_islandora_serial_object_intermediates_table(array $variables) {
  $form = $variables['form'];

  $rows = array();
  foreach (element_children($form) as $id) {
    $class = array('draggable');

    $indent = theme('indentation', array('size' => $form[$id]['depth']['#value']));
    unset($form[$id]['depth']);
    $rows[] = array(
      'data' => array(
        // Include our indentation.
        $indent . drupal_render($form[$id]['title']),
        drupal_render($form[$id]['volume']),
        drupal_render($form[$id]['issue']),
        drupal_render($form[$id]['year']),
        drupal_render($form[$id]['weight']),
        array(
          'data' => drupal_render($form[$id]['id']),
          'class' => array('islandora-serial-object-hidden-column'),
        ),
        array(
          'data' => drupal_render($form[$id]['pid']),
          'class' => array('islandora-serial-object-hidden-column'),
        ),
      ),
      'class' => $class,
    );
  }

  $header = array(
    t('Title'),
    t('Volume'),
    t('Issue'),
    t('Year'),
    t('Weight'),
    array(
      'data' => t('ID'),
      'class' => array('islandora-serial-object-hidden-column'),
    ),
    array(
      'data' => t('PID'),
      'class' => array('islandora-serial-object-hidden-column'),
    ),
  );
  $table_id = 'islandora-serial-object-intermediate-table';
  $output = theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => $table_id),
    'empty' => t('No intermediate children on the serial!'),
  ));
  $output .= drupal_render_children($form);
  drupal_add_tabledrag($table_id, 'match', 'parent', 'islandora-serial-object-intermediate-pid', 'islandora-serial-object-intermediate-pid', 'islandora-serial-object-intermediate-id', FALSE);
  drupal_add_tabledrag($table_id, 'order', 'sibling', 'islandora-serial-object-intermediate-weight');
  return $output;
}
