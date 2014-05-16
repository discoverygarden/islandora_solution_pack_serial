<?php
/**
 * @file
 * Houses the PDF management form for intermediate serial objects.
 */

/**
 * Form to add and re-order intermediate children.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 * @param AbstractObject $object
 *   An AbstractObject representing a Fedora object.
 *
 * @return array
 *   A renderable array containing the Drupal form.
 */
function islandora_serial_object_pdf_management_form($form, &$form_state, AbstractObject $object) {
  return $form;
}

/**
 * Page callback for ingesting a PDF on an intermediate serial object.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing a Fedora object.
 *
 * @return array
 *   The islandora ingest form passing through the ingest steps configuration.
 */
function islandora_serial_object_ingest_pdf_page_callback(AbstractObject $object) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  module_load_include('inc', 'islandora_basic_collection', 'includes/ingest.form');
  $parent = islandora_object_load('islandora:sp_pdf_collection');
  $config = islandora_basic_collection_get_ingest_configuration($parent);
  $config['models'] = array('islandora:sp_pdf');
  $config['islandora_serial_object_intermediate_pdf_parent'] = $object->id;
  return drupal_get_form('islandora_ingest_form', $config);
}

/**
 * Ingest step callback to associate a PDF.
 *
 * @param array $form_state
 *   An array containing the Drupal form state.
 *
 * @param string $parent_id
 *   The parent id of the intermediate serial object.
 */
function islandora_serial_object_associate_pdf(&$form_state, $parent_id) {
  $object = islandora_ingest_form_get_object($form_state);
  $object->relationships->add(ISLANDORA_RELS_EXT_URI, 'isConstituentOf', $parent_id);
  // Need to remove the existing collection relationship and make it part of
  // where our parent is.
  $object->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', 'islandora:sp_pdf_collection');
}

/**
 * Ingest step callback to unassociate a PDF.
 *
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_serial_object_unassociate_pdf(&$form_state) {
  $object = islandora_ingest_form_get_object($form_state);
  $object->relationships->remove(ISLANDORA_RELS_EXT_URI, 'isConstituentOf');
}
