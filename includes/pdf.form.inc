<?php
/**
 * @file
 * Houses the PDF management form for intermediate serial objects.
 */

/**
 * Form to re-order PDF children.
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
  // Get all PDFs.
  $tuque = islandora_get_tuque_connection();
  $sequence_predicate = ISLANDORA_SERIAL_OBJECT_SEQUENCE_PREDICATE;
  $pdf_parent_predicate = ISLANDORA_SERIAL_OBJECT_PDF_PARENT_PREDICATE;
  $islandora_namespace = ISLANDORA_RELS_EXT_URI;
  $query = <<<EOQ
  PREFIX islandora: <$islandora_namespace>
  SELECT ?pdf ?position ?label
  FROM <#ri>
  WHERE {
    ?pdf islandora:$pdf_parent_predicate <info:fedora/{$object->id}> ;
      islandora:$sequence_predicate ?position ;
      <fedora-model:label> ?label
  }
  ORDER BY ASC(?position)
EOQ;
  $results = $tuque->repository->ri->sparqlQuery($query);
  foreach ($results as $result) {
    $form['pdfs'][$result['pdf']['value']] = array(
      'weight' => array(
        '#default_value' => $result['position']['value'],
        '#type' => 'weight',
        '#delta' => 99999,
        '#title' => t('Weight'),
        '#title_display' => 'invisible',
      ),
      'label' => array('#markup' => $result['label']['value']),
    );
  }
  $form['pdfs']['#tree'] = TRUE;
  $form['pdfs']['#theme'] = 'islandora_serial_object_pdfs_table';
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update Order'),
  );
  return $form;
}
/**
 * Form to re-order PDF children validation handler.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_serial_object_pdf_management_form_validate($form, &$form_state) {
  if (!isset($form_state['values']['pdfs'])) {
    form_error($form, t('There are no PDFs to reorder.'));
  }
}

/**
 * Form to re-order PDF children submit handler.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_serial_object_pdf_management_form_submit($form, &$form_state) {
  $pdfs = $form_state['values']['pdfs'];
  uasort($pdfs, 'drupal_sort_weight');

  $position = 1;
  $operations = array();
  foreach ($pdfs as $pid => $pdf) {
    $operations[] = array(
      'islandora_serial_object_set_pdf_position',
      array($pid, $position++),
    );
  }

  batch_set(
    array(
      'operations' => $operations,
      'title' => t('Reordering PDFs'),
      'init_message' => t('Preparing to reorder PDFs.'),
      'progress_message' => t('Reordered @current out of @total.'),
      'error_message' => t('Encountered an error while reordering.'),
      'file' => drupal_get_path('module', 'islandora_serial_object') . '/includes/pdf.form.inc',
    )
  );

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
 * Page callback for ingesting a section hub on an intermediate serial object.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing a Fedora object.
 *
 * @return array
 *   The islandora ingest form passing through the ingest steps configuration.
 */
function islandora_serial_object_ingest_section_page_callback(AbstractObject $object) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  module_load_include('inc', 'islandora_basic_collection', 'includes/ingest.form');
  $parent = islandora_object_load('islandora:serial_collection');
  $config = islandora_basic_collection_get_ingest_configuration($parent);
  $config['models'] = array('islandora:intermediateSerialCModelStub');
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
  module_load_include('inc', 'islandora_serial_object', 'includes/utilities');
  $object = islandora_ingest_form_get_object($form_state);
  $sequence_number = islandora_serial_object_get_max_sequence_number($parent_id) + 1;
  $object->relationships->add(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_SERIAL_OBJECT_PDF_PARENT_PREDICATE,
    $parent_id
  );
  // Need to remove the existing collection relationship.
  $object->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', 'islandora:sp_pdf_collection');
  $object->relationships->add(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_SERIAL_OBJECT_SEQUENCE_PREDICATE,
    $sequence_number,
    RELS_TYPE_INT
  );
  $form_state['redirect'] = "islandora/object/$parent_id/manage/serial-pdf-management";
}

/**
 * Ingest step callback to unassociate a PDF.
 *
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_serial_object_unassociate_pdf(&$form_state) {
  $object = islandora_ingest_form_get_object($form_state);
  $object->relationships->remove(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_SERIAL_OBJECT_PDF_PARENT_PREDICATE
  );
  $object->relationships->remove(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_SERIAL_OBJECT_SEQUENCE_PREDICATE
  );
}

/**
 * Set/reset the position of a PDF if necessary.
 *
 * @param string $pid
 *   The PID of the object to set the parent of.
 * @param int $position
 *   The position of the PDF object.
 */
function islandora_serial_object_set_pdf_position($pid, $position) {
  $object = islandora_object_load($pid);
  $relations = $object->relationships;
  $relations->autoCommit = FALSE;

  $relations->remove(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_SERIAL_OBJECT_SEQUENCE_PREDICATE
  );

  $relations->add(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_SERIAL_OBJECT_SEQUENCE_PREDICATE,
    $position,
    RELS_TYPE_INT
  );

  $relations->commitRelationships();
}

function islandora_serial_object_ingest_section_form($form, &$form_state){
  $form = array(
    'title' => array(
      '#type' => 'textfield',
      '#title' => t('Section Name'),
      '#required' => true, 
    )
  );
  return $form;
}

function islandora_serial_object_ingest_section_form_submit($form, &$form_state){
  $obj = islandora_ingest_form_get_object($form_state);  
  $obj->label = $form_state['values']['title'];
  return $form;
}

