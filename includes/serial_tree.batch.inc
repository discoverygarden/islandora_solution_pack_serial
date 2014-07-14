<?php
/**
 * @file
 * Batch helpers for ingesting/updating serials.
 */

/**
 * Runs a batch to set the serial objects in a tree structure.
 *
 * @param array $object_tree
 *   The tree structure of the serial intermediate objects to set. An array of
 *   arrays where the key is the root serial object pid. Each array is an
 *   an array containing:
 *     - children: An array containing:
 *       - children: An array like this one (recursion).
 *       - caption: A string describing the caption.
 *       - number: A string identifying the number.
 */
function islandora_serial_object_serial_batch($object_tree) {
  $operations = islandora_serial_object_serial_batch_generate_ops($object_tree);
  if (!empty($operations)) {
    batch_set(
      array(
        'init_message' => t('Preparing to set serial objects.'),
        'error_message' => t('An error occured setting the serial objects.'),
        'progress_message' => t(
          'Setting serial objects <br/>Time elapsed: @elapsed <br/>Estimated time
            remaining @estimate.'
        ),
        'file' => drupal_get_path('module', 'islandora_serial_object') . '/includes/serial_tree.batch.inc',
        'operations' => $operations,
      )
    );
  }
}

/**
 * Will ingest/update an interim object.
 *
 * @param string $parent_id
 *   The current object's parent's PID.
 * @param string $object_id
 *   The current object's PID.
 * @param array $object_info
 *   An array of information about the current object.
 * @param array $context
 *   The context of the current batch operation.
 */
function islandora_serial_object_serial_batch_op($parent_id, $object_id, $object_info, &$context) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora_serial_object', 'includes/utilities');
  // Handle results setup on first op.
  if (!isset($context['results']['tree_ids_to_pids'])) {
    $context['results']['tree_ids_to_pids'] = array();
    $context['results']['tree_ids_to_pids'][$parent_id] = $parent_id;
  }
  // Workaround for http://drupal.org/node/1300928.
  // If the batch op runs first, then by the time the ingest form submit handler
  // gets called, its code is out of scope.
  module_load_include('inc', 'islandora', 'includes/ingest.form');

  $parent_pid = $context['results']['tree_ids_to_pids'][$parent_id];
  // If current object isn't created create it.
  $object_is_new = FALSE;
  if (islandora_is_valid_pid($object_id)) {
    $object = islandora_object_load($object_id);
    $object_pid = $object_id;
  }
  else {
    $object = FALSE;
    $object_pid = islandora_get_namespace($parent_pid);
  }
  if (!$object) {
    $tuque = islandora_get_tuque_connection();
    $object = $tuque->repository->constructObject($object_pid);
    $object_is_new = TRUE;
  }
  $context['results']['tree_ids_to_pids'][$object_id] = $object->id;

  // If the MODS datastream isn't created create it.
  $mods_datastream_is_new = FALSE;
  $mods_doc = new DOMDocument();
  $update_type = FALSE;
  if (!isset($object['MODS'])) {
    $mods_datastream = $object->constructDatastream('MODS', 'M');
    $mods_datastream_is_new = TRUE;
    $special_type = htmlspecialchars($object_info['type']);
    $mods_string = <<<EOXML
<?xml version="1.0" encoding="UTF-8"?>
<mods
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns="http://www.loc.gov/mods/v3"
  xsi:schemaLocation="http://www.loc.gov/mods/v3 http://www.loc.gov/standards/mods/v3/mods-3-4.xsd"
    version="3.4">
  <relatedItem type="host">
    <part>
      <detail type="{$special_type}">
         <caption/>
         <number/>
      </detail>
     </part>
  </relatedItem>
</mods>
EOXML;
  }
  else {
    $mods_datastream = $object['MODS'];
    $mods_string = $object['MODS']->content;
    // The level for the object may have changed and the type may have to be
    // updated to reflect this.
    if ($object_info['type'] != $object_info['default_type']) {
      $update_type = TRUE;
    }
  }
  $mods_doc->loadXML($mods_string);
  $mods_xpath = new DOMXPath($mods_doc);
  $mods_xpath->registerNamespace('mods', 'http://www.loc.gov/mods/v3');

  if ($update_type) {
    $type_xpath = islandora_serial_object_replace_xpath(ISLANDORA_SERIAL_OBJECT_MODS_DETAIL_XPATH, htmlspecialchars($object_info['default_type']));
    $mods_detail_xpath = $mods_xpath->query($type_xpath);
    if ($mods_detail_xpath->length) {
      $mods_type = $mods_detail_xpath->item(0);
      $mods_type->setAttribute('type', htmlspecialchars($object_info['type']));
    }
  }
  $caption_xpath = islandora_serial_object_replace_xpath(ISLANDORA_SERIAL_OBJECT_MODS_CAPTION_XPATH, htmlspecialchars($object_info['type']));
  $mods_caption_xpath = $mods_xpath->query($caption_xpath);
  if ($mods_caption_xpath->length) {
    $mods_caption = $mods_caption_xpath->item(0);
    $mods_caption->nodeValue = htmlspecialchars($object_info['caption']);
  }
  $number_xpath = islandora_serial_object_replace_xpath(ISLANDORA_SERIAL_OBJECT_MODS_NUMBER_XPATH, htmlspecialchars($object_info['type']));
  $mods_number_xpath = $mods_xpath->query($number_xpath);
  if ($mods_number_xpath->length) {
    $mods_number = $mods_number_xpath->item(0);
    $mods_number->nodeValue = htmlspecialchars($object_info['number']);
  }
  $mods_datastream->content = $mods_doc->saveXML();

  // Only want to update the label if the object does not have one.
  if (empty($object->label)) {
    module_load_include('inc', 'islandora_serial_object', 'includes/utilities');
    $label = array(
      'caption' => $object_info['caption'],
      'number' => $object_info['number'],
    );
    $label = islandora_serial_object_filter_mods_data($label);
    $object->label = implode(' ', $label);
  }
  $object->relationships->autoCommit = FALSE;
  $object->relationships->remove(
    FEDORA_RELS_EXT_URI,
    'isMemberOf'
  );
  $object->relationships->add(
    FEDORA_RELS_EXT_URI,
    'isMemberOf',
    $parent_pid,
    RELS_TYPE_URI
  );
  $object->relationships->remove(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_SERIAL_OBJECT_SEQUENCE_PREDICATE
  );
  $object->relationships->add(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_SERIAL_OBJECT_SEQUENCE_PREDICATE,
    $object_info['sequence_number'],
    RELS_TYPE_INT
  );
  $object->relationships->commitRelationships();

  // Ingest if necessary.
  if ($mods_datastream_is_new) {
    $mods_datastream->label = 'MODS Metadata';
    $mods_datastream->mimetype = 'text/xml';
    $object->ingestDatastream($mods_datastream);
  }
  if ($object_is_new) {
    $object->models = 'islandora:intermediateCModel';
    islandora_add_object($object);
  }
}

/**
 * Recursively map the tree generating the batch operations.
 *
 * @param array $object_tree
 *   The tree structure of the serial intermediate objects to set. An array of
 *   arrays where the key is the root serial object pid. Each array is an
 *   an array containing:
 *     - children: An array containing:
 *       - children: An array like this one (recursion).
 *       - caption: A string describing the caption.
 *       - number: A string identifying the number.
 *  ID's must be unique across all children in the tree.
 * @param NULL|string $parent_pid
 *   The current parent PID of the tree or NULL if it's the root object.
 *
 * @return array
 *   An array describing the operations for the batch.
 */
function islandora_serial_object_serial_batch_generate_ops($object_tree, $parent_pid = NULL) {
  $operations = array();
  $sequence_number = 1;
  foreach ($object_tree as $object_pid => $object_info) {
    $children = isset($object_info['children']) ? $object_info['children'] : array();
    unset($object_info['children']);
    $object_info['sequence_number'] = $sequence_number;

    // Handle leaves.
    if (!is_array($children)) {
      $object_pid = $children;
    }
    // Skip operations for the first PID which should be the serial.
    if (!is_null($parent_pid)) {
      $operations[] = array(
        'islandora_serial_object_serial_batch_op',
        array($parent_pid, $object_pid, $object_info),
      );
    }
    if (!empty($children) && is_array($children)) {
      $operations = array_merge(
        $operations,
        islandora_serial_object_serial_batch_generate_ops($children, $object_pid)
      );
    }
    ++$sequence_number;
  }
  return $operations;
}
