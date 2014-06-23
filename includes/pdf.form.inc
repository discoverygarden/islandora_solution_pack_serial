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
  SELECT ?pdf ?position ?label ?model
  FROM <#ri>
  WHERE {
    ?pdf islandora:$pdf_parent_predicate <info:fedora/{$object->id}> ;
      islandora:$sequence_predicate ?position ;
      <fedora-model:label> ?label ;
      <fedora-model:hasModel> ?model
      FILTER(sameTerm(?model, <info:fedora/islandora:sp_pdf>) || sameTerm(?model, <info:fedora/islandora:intermediateSerialCModelStub>))
  }
  ORDER BY ASC(?position)
EOQ;
  $results = $tuque->repository->ri->sparqlQuery($query);
  foreach ($results as $result) {
    $model = $result['model']['value'];
    $form['pdfs'][$result['pdf']['value']] = array(
      'weight' => array(
        '#default_value' => $result['position']['value'],
        '#type' => 'weight',
        '#delta' => 99999,
        '#title' => t('Weight'),
        '#title_display' => 'invisible',
      ),
      'label' => array('#markup' => $result['label']['value']),
      'type' => array(
        '#markup' => ($model == 'islandora:sp_pdf') ? t('PDF') : t('Section'),
      ),
    );
    if ($model == 'islandora:intermediateSerialCModelStub') {
      $pid = $result['pdf']['value'];
      $stub_object = islandora_object_load($pid);
      if (islandora_serial_object_pdf_management_edit_stub_access($object, $stub_object)) {
        $form['pdfs'][$pid]['edit'] = array(
          '#markup' => l(t('edit'), "islandora/object/{$object->id}/manage/serial-pdf-management/edit/$pid"),
        );
      }
    }
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
  $tuque = islandora_get_tuque_connection();
  $config = array();
  // The ISLANDORA_BASIC_COLLECTION_FAKE_PID is needed such that
  // constructObject() won't call getNextIdentifier(), needlessly.
  $new_object = $tuque->repository->constructObject(ISLANDORA_BASIC_COLLECTION_FAKE_PID);
  $config['objects'] = array(
    $new_object,
  );
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
/**
 * Ingest step callback to name an object.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 *
 * @return array
 *   An array containing the renderable Drupal form.
 */
function islandora_serial_object_ingest_section_form($form, &$form_state) {
  $form = array(
    'title' => array(
      '#type' => 'textfield',
      '#title' => t('Section Name'),
      '#required' => TRUE,
    ),
  );
  return $form;
}

/**
 * Ingest step callback to name an object .
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_serial_object_ingest_section_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_basic_collection', 'includes/ingest.form');
  $shared_storage = &islandora_ingest_form_get_shared_storage($form_state);
  $obj = islandora_ingest_form_get_object($form_state);
  $obj->label = $form_state['values']['title'];
  // Need to set an actual PID since we are ingesting this object.
  $namespace = islandora_get_namespace('islandora:rootSerialCModel');
  if ($obj->id == ISLANDORA_BASIC_COLLECTION_FAKE_PID || islandora_get_namespace($obj->id) !== $namespace) {
    $obj->id = $obj->repository->getNextIdentifier($namespace, variable_get('islandora_basic_collection_generate_uuid', FALSE));
  }
  $obj->models = $shared_storage['models'];
}

/**
 * The stub edit form.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 * @param AbstractObject $root_object
 *   An AbstractObject representing the root object.
 * @param AbstractObject $stub_object
 *   An AbstractObject representing the stub object.
 *
 * @return array
 *   An array representing the renderable Drupal form.
 */
function islandora_serial_object_pdf_management_edit_stub_form($form, &$form_state, AbstractObject $root_object, AbstractObject $stub_object) {
  if (!isset($form_state['confirm_form'])) {
    $edit_access = islandora_object_access(ISLANDORA_MANAGE_PROPERTIES, $stub_object);
    if ($edit_access) {
      $form['label'] = array(
        '#type' => 'textfield',
        '#title' => t('Section Name'),
        '#required' => TRUE,
        '#default_value' => $stub_object->label,
      );
    }
    else {
      $form['label'] = array(
        '#markup' => $stub_object->label,
      );
    }
    $form['update'] = array(
      '#type' => 'submit',
      '#value' => t('Update'),
      '#access' => $edit_access,
    );
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete Section'),
      '#access' => islandora_object_access(ISLANDORA_PURGE, $stub_object),
    );
    // Toss the PIDs in the form state because of potential caching issues.
    $form_state['stub_object_pid'] = $stub_object->id;
    $form_state['root_object_pid'] = $root_object->id;
  }
  else {
    $stub_object = islandora_object_load($form_state['stub_object_pid']);
    $form['message'] = array(
      '#type' => 'item',
      '#markup' => t('Are you sure you want to purge @label?', array(
        '@label' => $stub_object->label,
      )),
    );
    $form['purge'] = array(
      '#type' => 'submit',
      '#value' => t('Confirm'),
    );
    $form['cancel'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
    );
  }
  return $form;
}

/**
 * Submit handler for the the stub edit form.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_serial_object_pdf_management_edit_stub_form_submit($form, &$form_state) {
  $last_parent = end($form_state['triggering_element']['#parents']);
  if ($last_parent == 'update') {
    $stub_object = islandora_object_load($form_state['stub_object_pid']);
    if ($stub_object->label != $form_state['values']['label']) {
      $stub_object->label = $form_state['values']['label'];
    }
    drupal_set_message(t('The label has been updated for the section.'));
    $form_state['redirect'] = "islandora/object/{$form_state['root_object_pid']}/manage/serial-pdf-management";
  }
  elseif ($last_parent == 'delete') {
    $form_state['confirm_form'] = TRUE;
    $form_state['rebuild'] = TRUE;
  }
  elseif ($last_parent == 'cancel') {
    $form_state['redirect'] = "islandora/object/{$form_state['root_object_pid']}/manage/serial-pdf-management/edit/{$form_state['stub_object_pid']}";
  }
  else {
    $stub_object = islandora_object_load($form_state['stub_object_pid']);
    $label = $stub_object->label;
    $stub_object->repository->purgeObject($form_state['stub_object_pid']);
    $form_state['redirect'] = "islandora/object/{$form_state['root_object_pid']}/manage/serial-pdf-management";
    drupal_set_message(t('The section @label has been purged.', array('@label' => $label)));
  }
}
