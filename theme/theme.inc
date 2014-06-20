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
    $class = isset($form[$id]['#attributes']['class']) ? $form[$id]['#attributes']['class'] : array();
    $class[] = 'draggable';
    $indent = theme('indentation', array('size' => $form[$id]['depth']['#value']));
    unset($form[$id]['depth']);
    $rows[] = array(
      'data' => array(
        // Include our indentation.
        $indent . drupal_render($form[$id]['caption']),
        drupal_render($form[$id]['number']),
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
    t('Caption'),
    t('Number'),
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
  drupal_add_tabledrag($table_id, 'match', 'parent', 'islandora-serial-object-intermediate-pid', 'islandora-serial-object-intermediate-pid', 'islandora-serial-object-intermediate-id', FALSE, $form['#levels'] - 1);
  drupal_add_tabledrag($table_id, 'order', 'sibling', 'islandora-serial-object-intermediate-weight');
  drupal_add_css(
    drupal_get_path('module', 'islandora_serial_object') .
      '/css/islandora_serial_object.css'
  );
  return $output;
}

/**
 * Implements theme_element().
 *
 * Makes the table a draggable one.
 */
function theme_islandora_serial_object_pdfs_table(array $variables) {
  $form = $variables['form'];
  $rows = array();
  foreach (element_children($form) as $id) {
    $form[$id]['weight']['#attributes']['class'] = array(
      'islandora-serial-object-pdf-weight',
    );
    $rows[] = array(
      'data' => array(
        drupal_render($form[$id]['label']),
        drupal_render($form[$id]['type']),
        isset($form[$id]['edit']) ? drupal_render($form[$id]['edit']) : '',
        drupal_render($form[$id]['weight']),
      ),
      'class' => array('draggable'),
    );
  }
  $table_id = 'islandora-serial-object-pdfs-table';
  $header = array(
    t('Label'),
    t('Type'),
    t('Operations'),
    t('Weight'),
  );
  $output = theme(
    'table', array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array('id' => $table_id),
      'empty' => t('No PDFs associated.'),
    )
  );
  $output .= drupal_render_children($form);
  drupal_add_tabledrag(
    $table_id,
    'order',
    'sibling',
    'islandora-serial-object-pdf-weight'
  );
  return $output;
}

/**
 * Implements theme_element().
 */
function theme_islandora_serial_object_intermediate_levels_table(array $variables) {
  $form = $variables['form'];
  $rows = array();
  foreach (element_children($form) as $id) {
    $rows[] = array(
      'data' => array(
        drupal_render($form[$id]['checkbox']),
        drupal_render($form[$id]['level']),
        drupal_render($form[$id]['type']),
        drupal_render($form[$id]['caption']),
      ),
    );
  }
  $output = theme('table', array(
      'header' => array(
        'checkbox' => '',
        'level' => t('Hierarchical Level'),
        'type' => t('MODS detail[@type] Value'),
        'caption' => t('Default Caption Value'),
      ),
      'rows' => $rows,
      'attributes' => array(),
      'empty' => t('No levels defined!'),
  ));
  $output .= drupal_render_children($form);
  return $output;
}
