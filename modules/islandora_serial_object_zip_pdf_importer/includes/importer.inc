<?php
/**
 * @file
 * Defines the IslandoraSerialObjectZipPDFBatchImporter.
 */

/**
 * Defines the ZIP PDF importer for Islandora Serial Object.
 */
class IslandoraSerialObjectZipPDFBatchImporter extends ZipBatchImporter {
  protected $itemClass = 'IslandoraSerialObjectZipPDFBatchImportObject';

  /**
   * Get the form for this importer.
   *
   * @see IslandoraBatchImporter::getForm()
   */
  public static function getForm(array &$form_state) {
    $form = parent::getForm($form_state);
    $form['fs']['#title'] = t('PDF Zip Batch Importer');
    $form['fs']['content_model'] = array(
      '#type' => 'value',
      '#value' => array('islandora:sp_pdf'),
    );
    unset($form['fs']['fs']);
    $form['fs']['namespace'] = array(
      '#type' => 'value',
      '#value' => islandora_get_namespace($form_state['storage']['parent_pid']),
    );
    return $form;
  }

  /**
   * Get the required info to pass into the file parameter of the constructor.
   *
   * @see IslandoraBatchImporter::getBatchInfo()
   */
  public static function getBatchInfo(array &$form_state) {
    module_load_include('inc', 'islandora_serial_object', 'includes/utilities');
    $info = parent::getBatchInfo($form_state);
    $info['parent_pid'] = $form_state['storage']['parent_pid'];
    $info['sequence_number'] = islandora_serial_object_get_max_sequence_number($form_state['storage']['parent_pid']);
    return $info;
  }

  /**
   * Get ingest info.
   *
   * @see IslandoraBatchImporter::getIngestInfo()
   */
  protected static function getIngestInfo($file) {
    $zip = new ZipArchive();
    if (($error = $zip->open(drupal_realpath($file->uri))) !== TRUE) {
      drupal_set_message(t('Error opening the provided Zip file. Code: %code', array(
        '%code' => $error,
      )));
      return;
    }

    $count = $zip->numFiles;

    $info = array();
    for ($i = 0; $i < $count; $i++) {
      $name = $zip->getNameIndex($i);
      $path_info = pathinfo($name);

      $object_key = $path_info['dirname'] . '-_-' . $path_info['filename'];
      if (strpos($path_info['dirname'], '__MACOSX') === FALSE
        && isset($path_info['extension'])
        && $path_info['extension'] != 'DS_Store') {
        $datastream_key = strtolower($path_info['extension']);
        if ($datastream_key == 'pdf' || $datastream_key == 'xml') {
          $info[$object_key][$datastream_key] = $name;
        }
      }
    }
    $zip->close();
    return $info;
  }
}

/**
 * ZIP import object.
 *
 * Actually does the heavy-lifting during the import.
 */
class IslandoraSerialObjectZipPDFBatchImportObject extends ZipBatchImportObject {

  /**
   * Get an item from the source.
   *
   * @see IslandoraImportObject::getOne()
   */
  public static function getOne(&$info) {
    $record = array(
      'pid_namespace' => $info['pid_namespace'],
      'file' => $info['file'],
      'content_model' => $info['content_model'],
      'sequence_number' => ++$info['sequence_number'],
      'parent_pid' => $info['parent_pid'],
    );
    $key = key($info['object_info']);
    $record['object_info'] = $info['processed_objects'][$key] = $info['object_info'][$key];
    unset($info['object_info'][$key]);
    return (empty($record) ? FALSE : new static($record));
  }

  /**
   * Modify the relationships for the object.
   *
   * @see IslandoraImportObject::modifyRelationships()
   */
  public function modifyRelationships(AbstractObject $object) {
    parent::modifyRelationships($object);
    $object->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');
    $object->relationships->add(ISLANDORA_RELS_EXT_URI, ISLANDORA_SERIAL_OBJECT_PDF_PARENT_PREDICATE, $this->source['parent_pid']);
    $object->relationships->add(ISLANDORA_RELS_EXT_URI, ISLANDORA_SERIAL_OBJECT_SEQUENCE_PREDICATE, $this->source['sequence_number'], RELS_TYPE_INT);
  }
}
