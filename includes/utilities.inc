<?php
/**
 * @file
 * Houses utility functions for the Serial solution pack.
 */

/**
 * Appends on the markup for intermediate management table to a form.
 *
 * @param array $form
 *   A reference to an array containing a Drupal form.
 * @param array $form_state
 *   A reference to an array containing the Drupal form state.
 * @param string $type
 *   Where this form is being used, either 'ingest' or 'manage'.
 */
function islandora_serial_object_build_intermediates(&$form, &$form_state, $type) {
  if ($type == 'ingest') {
    $intermediates = isset($form_state['islandora_serial_object_intermediates']) ? $form_state['islandora_serial_object_intermediates'] : FALSE;
  }
  else {
    // @TODO: Grab from existing.
  }
  $form['islandora_serial_object_intermediates']['#theme'] = 'islandora_serial_object_intermediates_table';
  if ($intermediates) {
    islandora_serial_object_construct_markup($form['islandora_serial_object_intermediates'], $intermediates['intermediates'], 0);
  }
}

/**
 * Sorts the intermediates array into the correct ordering.
 *
 * @param array $intermediates
 *   An array containing:
 *   -last_id: The last id of the inserted intermediate, used when adding new
 *   ids.
 *   -intermediates: An array of associative arrays where the key is the pid
 *   (parent id) of the intermediate. They key of Each associative array is the
 *   id of the intermediate and contains:
 *     -title: A string describing the object.
 *     -volume: A string identifying the volume.
 *     -issue: A string identifying the issue.
 *     -year: A string identifying the year.
 *     -pid: A string that stores reference to which parent the intermediate is
 *     part of.
 *     -id: A unique string used for identification.
 *     -weight: A string referencing how far down in the order the intermedaite
 *     is.
 *     -depth: A string describing how far "idented" the object is.
 *
 * @return array
 *   A sorted array in the form described above.
 */
function islandora_serial_object_sort_intermediates($intermediates) {
  islandora_serial_object_get_tree($intermediates['intermediates'], 0);
  return $intermediates;
}

/**
 * Sorts the intermediates into the correct form for rendering.
 *
 * @param array $children
 *   An array of associative arrays where the key is the pid
 *   (parent id) of the intermediate. They key of Each associative array is the
 *   id of the intermediate and contains:
 *     -title: A string describing the object.
 *     -volume: A string identifying the volume.
 *     -issue: A string identifying the issue.
 *     -year: A string identifying the year.
 *     -pid: A string that stores reference to which parent the intermediate is
 *     part of.
 *     -id: A unique string used for identification.
 *     -weight: A string referencing how far down in the order the intermedaite
 *     is.
 *     -depth: A string describing how far "idented" the object is.
 * @param string $parent_id
 *   The pid (parent id) of where to start traversing.
 * @param int $depth
 *   The depth of the current child, used for indentation.
 */
function islandora_serial_object_get_tree(&$children, $parent_id, $depth = 0) {
  if (isset($children[$parent_id])) {
    uasort($children[$parent_id], 'islandora_serial_object_stable_drupal_sort_weight');
    foreach ($children[$parent_id] as &$child) {
      $child['depth'] = $depth;
      islandora_serial_object_get_tree($children, $child['id'], $depth + 1);
    }
  }
}

/**
 * Creates the entries in the form for each child to be rendered as table rows.
 *
 * @param array $form
 *   A reference to an array containing a Drupal form.
 * @param array $children
 *   An array of associative arrays where the key is the pid
 *   (parent id) of the intermediate. They key of Each associative array is the
 *   id of the intermediate and contains:
 *     -title: A string describing the object.
 *     -volume: A string identifying the volume.
 *     -issue: A string identifying the issue.
 *     -year: A string identifying the year.
 *     -pid: A string that stores reference to which parent the intermediate is
 *     part of.
 *     -id: A unique string used for identification.
 *     -weight: A string referencing how far down in the order the intermedaite
 *     is.
 *     -depth: A string describing how far "idented" the object is.
 * @param string $parent_id
 *   The pid (parent id) of where to start traversing.
 */
function islandora_serial_object_construct_markup(&$form, $children, $parent_id) {
  if (isset($children[$parent_id])) {
    foreach ($children[$parent_id] as $child) {
      $form[$child['id']] = array(
        'title' => array(
          '#type' => 'textfield',
          '#default_value' => isset($child['title']) ? $child['title'] : FALSE,
        ),
        'volume' => array(
          '#type' => 'textfield',
          '#default_value' => isset($child['volume']) ? $child['volume'] : FALSE,
          '#size' => 55,
        ),
        'issue' => array(
          '#type' => 'textfield',
          '#default_value' => isset($child['volume']) ? $child['volume'] : FALSE,
          '#size' => 55,
        ),
        'year' => array(
          '#type' => 'textfield',
          '#default_value' => isset($child['year']) ? $child['year'] : FALSE,
          '#size' => 10,
        ),
        'pid' => array(
          '#type' => 'hidden',
          '#length' => 3,
          '#default_value' => isset($child['pid']) ? $child['pid'] : FALSE,
          '#attributes' => array(
            'class' => array('islandora-serial-object-intermediate-pid'),
          ),
        ),
        'id' => array(
          '#type' => 'hidden',
          '#length' => 3,
          '#default_value' => isset($child['id']) ? $child['id'] : FALSE,
          '#attributes' => array(
            'class' => array('islandora-serial-object-intermediate-id'),
          ),
        ),
        'weight' => array(
          '#type' => 'weight',
          '#title' => t('Weight'),
          '#default_value' => isset($child['weight']) ? $child['weight'] : FALSE,
          '#delta' => 10,
          '#title_display' => 'invisible',
          '#attributes' => array(
            'class' => array('islandora-serial-object-intermediate-weight'),
          ),
        ),
        'depth' => array(
          '#type' => 'hidden',
          '#default_value' => isset($child['depth']) ? $child['depth'] : 0,
        ),
      );
      islandora_serial_object_construct_markup($form, $children, $child['id']);
    }
  }
}

/**
 * Performs a stable sort to preserve order when encountering the same weight.
 *
 * @param array $a
 *   An array describing an intermediate object containing:
 *   -title: A string describing the object.
 *   -volume: A string identifying the volume.
 *   -issue: A string identifying the issue.
 *   -year: A string identifying the year.
 *   -pid: A string that stores reference to which parent the intermediate is
 *    part of.
 *   -id: A unique string used for identification.
 *   -weight: A string referencing how far down in the order the intermedaite
 *   is.
 *   -depth: A string describing how far "idented" the object is.
 * @param array $b
 *   An array describing an intermediate object containing:
 *   -title: A string describing the object.
 *   -volume: A string identifying the volume.
 *   -issue: A string identifying the issue.
 *   -year: A string identifying the year.
 *   -pid: A string that stores reference to which parent the intermediate is
 *    part of.
 *   -id: A unique string used for identification.
 *   -weight: A string referencing how far down in the order the intermedaite
 *   is.
 *   -depth: A string describing how far "idented" the object is.
 *
 * @return int
 *   Result of where the object is to be sorted.
 */
function islandora_serial_object_stable_drupal_sort_weight($a, $b) {
  $sort_weight = drupal_sort_weight($a, $b);
  if ($sort_weight == 0) {
    $sort_weight = $a['id'] < $b['id'] ? -1 : 1;
  }
  return $sort_weight;
}
