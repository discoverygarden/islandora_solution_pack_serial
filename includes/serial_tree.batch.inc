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
 *       - title: A string of a MODS title.
 *       - volume: A string representing the volume for MODS.
 *       - issue: A string representing the issue for MODS.
 *       - year: A string representing the year for MODS.
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
  // Handle sandbox setup on first op.
  if (!isset($context['sandbox']['tree_ids_to_pids'])) {
    $context['sandbox']['tree_ids_to_pids'] = array();
    $context['sandbox']['tree_ids_to_pids'][$parent_id] = $parent_id;
  }
  // Workaround for http://drupal.org/node/1300928.
  // If the batch op runs first, then by the time the ingest form submit handler
  // gets called, its code is out of scope.
  module_load_include('inc', 'islandora', 'includes/ingest.form');

  $parent_pid = $context['sandbox']['tree_ids_to_pids'][$parent_id];
  // If current object isn't created create it.
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
  $context['sandbox']['tree_ids_to_pids'][$object_id] = $object->id;

  // If the MODS datastream isn't created create it.
  $mods_datastream_is_new = FALSE;
  $mods_doc = new DOMDocument();
  if (!isset($object['MODS'])) {
    $mods_datastream = $object->constructDatastream('MODS', 'M');
    $mods_datastream_is_new = TRUE;
    $mods_string = <<<EOXML
<?xml version="1.0" encoding="UTF-8"?>
<mods
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns="http://www.loc.gov/mods/v3"
  xsi:schemaLocation="http://www.loc.gov/mods/v3 http://www.loc.gov/standards/mods/v3/mods-3-4.xsd"
    version="3.4"
/>
EOXML;
    $mods_doc->loadXML($mods_string);
    $mods_xpath = new DOMXPath($mods_doc);
  }
  else {
    $mods_datastream = $object['MODS'];
    $mods_doc->loadXML($object['MODS']->content);
    $mods_xpath = new DOMXPath($mods_doc);
    $mods_xpath->registerNamespace('mods', 'http://www.loc.gov/mods/v3');
    // Remove MODS data if necessary.
    $xpaths_to_remove = array(
      ISLANDORA_SERIAL_OBJECT_MODS_TITLE_XPATH,
      ISLANDORA_SERIAL_OBJECT_MODS_VOLUME_XPATH,
      ISLANDORA_SERIAL_OBJECT_MODS_ISSUE_XPATH,
      ISLANDORA_SERIAL_OBJECT_MODS_YEAR_START_XPATH,
      ISLANDORA_SERIAL_OBJECT_MODS_YEAR_END_XPATH,
    );
    foreach ($xpaths_to_remove as $xpath) {
      $elements = $mods_xpath->query($xpath);
      if ($elements->length) {
        $item = $elements->item(0);
        $item->parentNode->removeChild($item);
      }
    }
    // Access elements to add to.
    $mods_origin_infos = $mods_xpath->query(ISLANDORA_SERIAL_OBJECT_MODS_ORIGIN_INFO_XPATH);
    $mods_title_infos = $mods_xpath->query(ISLANDORA_SERIAL_OBJECT_MODS_TITLE_INFO_XPATH);
  }
  // Get or create elements to work on.
  if (isset($mods_title_infos) && $mods_origin_infos->length) {
    $mods_origin_info = $mods_origin_infos->item(0);
  }
  else {
    $mods_origin_info = $mods_doc->createElement('originInfo');
    $mods_doc->documentElement->appendChild($mods_origin_info);
  }
  if (isset($mods_title_infos) && $mods_title_infos->length) {
    $mods_title_info = $mods_title_infos->item(0);
  }
  else {
    $mods_title_info = $mods_doc->createElement('titleInfo');
    $mods_doc->documentElement->appendChild($mods_title_info);
  }

  // Add MODS data.
  $title_element = $mods_doc->createElement('title', $object_info['title']);
  $volume_element = $mods_doc->createElement('partNumber', $object_info['volume']);
  $issue_element = $mods_doc->createElement('partName', $object_info['issue']);
  $year_start_element = $mods_doc->createElement('dateIssued', $object_info['year']);
  $year_end_element = $mods_doc->createElement('dateIssued', $object_info['year']);

  $year_start_element->setAttribute('point', 'start');
  $year_end_element->setAttribute('point', 'end');
  $year_start_element->setAttribute('encoding', 'w3cdtf');
  $year_end_element->setAttribute('encoding', 'w3cdtf');

  $mods_title_info->appendChild($title_element);
  $mods_title_info->appendChild($volume_element);
  $mods_title_info->appendChild($issue_element);
  $mods_origin_info->appendChild($year_start_element);
  $mods_origin_info->appendChild($year_end_element);

  $mods_datastream->content = $mods_doc->saveXML();
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
    RELS_TYPE_PLAIN_LITERAL
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
    $object->label = $object_info['title'];
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
 *       - title: A string of a MODS title.
 *       - volume: A string representing the volume for MODS.
 *       - issue: A string representing the issue for MODS.
 *       - year: A string representing the year for MODS.
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
    $parent_pid = $object_pid;
    if (!empty($children) && is_array($children)) {
      $operations = array_merge(
        $operations,
        islandora_serial_object_serial_batch_generate_ops($children, $parent_pid)
      );
    }
    ++$sequence_number;
  }
  return $operations;
}
