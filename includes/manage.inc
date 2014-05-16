<?php
/**
 * @file
 * Holds ingest steps for the Islandora Serial solution pack.
 */

/**
 * Form to add and re-order intermediate children.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 * @param string $type
 *   Where this form is being used, either 'ingest' or 'manage'.
 *
 * @return array
 *   A renderable array containing the Drupal form.
 */
function islandora_serial_object_manage_intermediates_form($form, &$form_state, $type) {
  module_load_include('inc', 'islandora_serial_object', 'includes/utilities');
  if (isset($form_state['triggering_element'])) {
    $last_parent = end($form_state['triggering_element']['#parents']);
    if ($last_parent == 'add_intermediate_object' || $last_parent == 'next') {
      if (isset($form_state['values']['table_wrapper']['islandora_serial_object_intermediates'])) {
        // Update the objects in the form state before we go sort the existing.
        $form_state['islandora_serial_object_intermediates']['intermediates'] = array();
        foreach ($form_state['values']['table_wrapper']['islandora_serial_object_intermediates'] as $id => $values) {
          $form_state['islandora_serial_object_intermediates']['intermediates'][$values['pid']][$id] = $values;
        }
        $existing_intermediates = islandora_serial_object_sort_intermediates($form_state['islandora_serial_object_intermediates']);
      }
      else {
        $existing_intermediates = array(
          'last_id' => 0,
          'intermediates' => array(),
        );
      }
      if ($last_parent == 'add_intermediate_object') {
        // New objects will always be appended with no parent to start.
        $pid = 0;
        $new_intermediate_id = $existing_intermediates['last_id'] + 1;
        $existing_intermediates['last_id'] = $new_intermediate_id;

        // Make sure all new items are appended on after the highest max weight
        // for ordering purposes.
        $map_function = function ($element) {
          $weight_map_function = function($element) {
            return $element['weight'];
          };
          return max(array_map($weight_map_function, $element));
        };
        $max_weight = empty($existing_intermediates['intermediates']) ? -10 : max(array_map($map_function, $existing_intermediates['intermediates']));
        $existing_intermediates['intermediates'][$pid][$new_intermediate_id] = array(
          'id' => $new_intermediate_id,
          'pid' => $pid,
          'weight' => $max_weight + 1,
        );
      }
      $form_state['islandora_serial_object_intermediates'] = $existing_intermediates;
    }
  }
  $form['#tree'] = TRUE;
  $form['table_wrapper'] = array(
    '#prefix' => '<div id="islandora-serial-object-intermediate-table-wrapper">',
    '#suffix' => '</div>',
    'add_intermediate_object' => array(
      '#type' => 'button',
      '#value' => t('Add another'),
      '#ajax' => array(
        'callback' => 'islandora_serial_object_add_intermediate_object',
        'wrapper' => 'islandora-serial-object-intermediate-table-wrapper',
      ),
      '#weight' => 10,
    ),
  );
  islandora_serial_object_build_intermediates($form['table_wrapper'], $form_state, $type);
  return $form;
}

/**
 * AJAX callback for adding a new intermediate.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 *
 * @return array
 *   The portion of the form to be rendered.
 */
function islandora_serial_object_add_intermediate_object($form, &$form_state) {
  return $form['table_wrapper'];
}

/**
 * Validation for the manage intermediates form.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_serial_object_manage_intermediates_form_validate(&$form, &$form_state) {
  dsm($form_state);
  module_load_include('inc', 'islandora_serial_object', 'includes/utilities');
  // We need to trigger a form rebuild before validation so we can save
  // all of the fun AJAX changes that had occurred.
  $form = drupal_rebuild_form('islandora_ingest_form', $form_state, $form);
  foreach ($form_state['values']['table_wrapper']['islandora_serial_object_intermediates'] as $id => $intermediate) {
    if (empty($intermediate['title']) && empty($intermediate['volume']) && empty($intermediate['issue']) && empty($intermediate['volume'])) {
      if (isset($form_state['islandora_serial_object_intermediates']['intermediates'][$id])) {
        // Need to see if these children are empty as well.
        $empty_children = TRUE;
        foreach ($form_state['islandora_serial_object_intermediates']['intermediates'][$id] as $vals) {
          if (!empty($vals['title'])|| !empty($vals['volume']) || !empty($vals['issue']) || !empty($vals['year'])) {
            $empty_children = FALSE;
            break;
          }
        }
        if (!$empty_children) {
          // Because of the above form rebuild, form_set_error does not append
          // the error class on the correct elements.
          foreach ($form['table_wrapper']['islandora_serial_object_intermediates'][$id] as $field_name => &$field) {
            $fields_to_mark = array(
              'title',
              'volume',
              'issue',
              'year',
            );
            if (in_array($field_name, $fields_to_mark)) {
              if (!isset($field['attributes']['class'])) {
                $field['#attributes']['class'] = array('error');
              }
              else {
                $field['#attributes']['class'][] = 'error';
              }
            }
          };
          form_error($form['table_wrapper']['islandora_serial_object_intermediates'][$id]['title'], t('An empty intermediate may not have any populated children!'));
        }
      }
    }
    elseif (empty($intermediate['title']) && (!empty($intermediate['volume']) || !empty($intermediate['issue']) || !empty($intermediate['year']))) {
      form_error($form['table_wrapper']['islandora_serial_object_intermediates'][$id]['title'], t('A title is required!'));
      if (!isset($form['table_wrapper']['islandora_serial_object_intermediates'][$id]['title']['#attributes']['class'])) {
        $form['table_wrapper']['islandora_serial_object_intermediates'][$id]['title']['#attributes']['class'] = array('error');
      }
      else {
        $form['table_wrapper']['islandora_serial_object_intermediates'][$id]['title']['attributes']['class'][] = 'error';
      }
    }
  }
}

/**
 * Submit handler for the manage intermediates form.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_serial_object_manage_intermediates_form_submit(&$form, &$form_state) {
  if (isset($form_state['islandora_serial_object_intermediates'])) {
    module_load_include('inc', 'islandora_serial_object', 'includes/utilities');
    $object = islandora_ingest_form_get_object($form_state);
    $shared_storage = &islandora_ingest_form_get_shared_storage($form_state);
    // Get an identifier for our object at this time so the intermediates can
    // reference the root object correctly.
    $collection = $shared_storage['collection'];
    // Set the objects namespace.
    $policy = new CollectionPolicy($collection['COLLECTION_POLICY']->content);
    $models = $policy->getContentModels();
    $model = $models[reset($shared_storage['models'])];
    $namespace = $model['namespace'];
    $object->id = $object->repository->getNextIdentifier($namespace, variable_get('islandora_basic_collection_generate_uuid', FALSE));
    $filtered_children = $form_state['islandora_serial_object_intermediates']['intermediates'];
    foreach ($form_state['islandora_serial_object_intermediates']['intermediates'] as $parent_id => $children) {
      foreach ($children as $id => $child) {
        if (empty($child['title']) && empty($child['volume']) && empty($child['issue']) && empty($child['year'])) {
          unset($filtered_children[$parent_id][$id]);
        }
      }
    }
    $flat_structure = islandora_serial_object_get_intermediates_as_tree($filtered_children, $object->id);
  }
}
