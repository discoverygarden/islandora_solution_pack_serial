<?php

/**
 * @file
 * Generates derivatives for serial objects and their children.
 */

/**
 * Updates the intermediate object's label from two MODS fields.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object in Fedora.
 *
 * @param bool $force
 *   Whether the derivative generation is being forced.
 *
 * @return array
 *   An array describing the outcome of the label updation.
 *
 * @see hook_islandora_derivative()
 */
function islandora_serial_object_create_intermediate_label(AbstractObject $object, $force = FALSE) {
  $levels = islandora_serial_object_find_root_levels($object->id);
  if ($levels['success']) {
    module_load_include('inc', 'islandora_serial_object', 'includes/utilities');
    $type = $levels['type'];
    $mods = islandora_serial_object_filter_mods_data(islandora_serial_object_retrieve_mods_data($object->id, $type));
    $label = implode(' ', $mods);
    if ($object->label != $label) {
      $object->label = $label;
    }
    return array(
      'success' => TRUE,
      'messages' => array(
        array(
          'message' => 'Modified the label for @obj from MODS.',
          'message_sub' => array(
            '@obj' => $object->id,
          ),
          'type' => 'dsm',
        ),
      ),
    );
  }
  else {
    return array(
      'success' => FALSE,
      'messages' => array(),
    );
  }
}

/**
 * Helper function to go upwards to find the root levels.
 *
 * @param string $pid
 *   The pid of the object we are attempting to traverse up from.
 *
 * @param int $counter
 *   The depth of traversal to be used when grabbing levels.
 *
 * @return array
 *   An array describing the outcome of the levels traversal containing:
 *   -success: A bool denoting whether or not we were successful
 *   -message (Optional): A string denoting why the operation failed.
 *   -type (Optional): A string denoting the type attribute to be used in
 *   XPathing, only present when the traversal was successful.
 */
function islandora_serial_object_find_root_levels($pid, $counter = 0) {
  $object = islandora_object_load($pid);
  if ($object) {
    if (!in_array('islandora:rootSerialCModel', $object->models)) {
      $relationships = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
      // If there is no isMemberOf relationship perhaps this is the root.
      foreach ($relationships as $relationship) {
        // More traversal upwards.
        $pid = $relationship['object']['value'];
        $result = islandora_serial_object_find_root_levels($pid, $counter + 1);
        if ($result['success']) {
          return $result;
        }
      }
      return array(
        'success' => FALSE,
        'message' => t('The highest most member in the intermediate levels traversal was not a root object.'),
      );
    }
    else {
      if (isset($object['SERIAL-LEVELS'])) {
        $intermediate_levels = json_decode($object['SERIAL-LEVELS']->content, TRUE);
        return array(
          'success' => TRUE,
          'type' => $intermediate_levels[$counter]['type'],
        );
      }
      else {
        return array(
          'success' => FALSE,
          'message' => t('The root serial object did not containing the SERIAL-LEVELS datastream.'),
        );
      }
    }
  }
  return array(
    'success' => FALSE,
    'message' => t('The object @pid was specified in the intermediate levels traversal and does not exist, or the user does not have access to it.', array('@pid' => $pid)),
  );
}
