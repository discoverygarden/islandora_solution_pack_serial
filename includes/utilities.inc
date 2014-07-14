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
 * @param AbstractObject $object
 *   An optional AbstractObject representing an object within Fedora.
 */
function islandora_serial_object_build_intermediates(&$form, &$form_state, $type, AbstractObject $object = NULL) {
  if ($type == 'ingest') {
    $root = 0;
    $ingest_object = islandora_ingest_form_get_object($form_state);
    $intermediate_levels = json_decode($ingest_object['SERIAL-LEVELS']->content, TRUE);
  }
  else {
    $root = $object->id;
    $intermediate_levels = json_decode($object['SERIAL-LEVELS']->content, TRUE);
  }
  // If the form state has already been built up, just use it.
  if (isset($form_state['islandora_serial_object_intermediates'])) {
    $intermediates = $form_state['islandora_serial_object_intermediates'];
  }
  else {
    if ($type == 'ingest') {
      $intermediates = FALSE;
    }
    else {
      module_load_include('inc', 'islandora_serial_object', 'includes/utilities');
      $intermediates = array(
        'last_id' => 0,
        'intermediates' => array(),
      );
      $intermediates['intermediates'] = islandora_serial_object_get_children($object->id);
      // Need to get the correct depth and ordering on our existing elements.
      $intermediates = islandora_serial_object_sort_intermediates($intermediates, $root);
      // Store these in the form state for use.
      $form_state['islandora_serial_object_intermediates'] = $intermediates;
    }
  }
  $form['islandora_serial_object_intermediates']['#theme'] = 'islandora_serial_object_intermediates_table';
  $form['islandora_serial_object_intermediates']['#levels'] = count($intermediate_levels);
  if ($intermediates) {
    islandora_serial_object_construct_markup($form['islandora_serial_object_intermediates'], $intermediates['intermediates'], $root, $intermediate_levels);
  }
  // Add the JS for tableDrag event firing fun!
  $module_path = drupal_get_path('module', 'islandora_serial_object');

  // Need to use drupal_add_js as opposed to attaching to the form as
  // tabledrag's JS is added in the theme layer with a weight of -1. As our
  // custom tabledrag is defining onDrop functionality the core tabledrag JS
  // needs to be loaded first.
  drupal_add_js("$module_path/js/islandora_serial_object_tabledrag.js");
  $form['#attached']['js'][] = array(
    'data' => array('islandoraSerialObjectIntermediateLevels' => $intermediate_levels),
    'type' => 'setting',
  );
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
 *     -caption: A string describing the caption.
 *     -number: A string identifying the number.
 *     -pid: A string that stores reference to which parent the intermediate is
 *     part of.
 *     -id: A unique string used for identification.
 *     -weight: A string referencing how far down in the order the intermedaite
 *     is.
 *     -depth: A string describing how far "idented" the object is.
 * @param int|string $root
 *   The key of the root element to start at.
 *
 * @return array
 *   A sorted array in the form described above.
 */
function islandora_serial_object_sort_intermediates($intermediates, $root = 0) {
  islandora_serial_object_get_tree_for_form($intermediates['intermediates'], $root);
  return $intermediates;
}

/**
 * Sorts the intermediates into the correct form for rendering.
 *
 * @param array $children
 *   An array of associative arrays where the key is the pid
 *   (parent id) of the intermediate. They key of Each associative array is the
 *   id of the intermediate and contains:
 *     -caption: A string describing the caption.
 *     -number: A string identifying the number.
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
function islandora_serial_object_get_tree_for_form(&$children, $parent_id, $depth = 0) {
  if (isset($children[$parent_id])) {
    uasort($children[$parent_id], 'islandora_serial_object_stable_drupal_sort_weight');
    foreach ($children[$parent_id] as &$child) {
      $child['depth'] = $depth;
      islandora_serial_object_get_tree_for_form($children, $child['id'], $depth + 1);
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
 *   (parent id) of the intermediate. They key of each associative array is the
 *   id of the intermediate and contains:
 *     -caption: A string describing the caption.
 *     -number: A string identifying the number.
 *     -pid: A string that stores reference to which parent the intermediate is
 *     part of.
 *     -id: A unique string used for identification.
 *     -weight: A string referencing how far down in the order the intermedaite
 *     is.
 *     -depth: A string describing how far "idented" the object is.
 *     -ingested: A bool denoting whether this object has been added to Fedora
 *     yet, determines rendering.
 * @param string $parent_id
 *   The pid (parent id) of where to start traversing.
 * @param array $intermediate_levels
 *   An array of the defined mapping of the intermediate levels where the key is
 *   the level. The array contains:
 *   -level: An integer denoting the level this mapping relates to.
 *   -type: A string denoting the type for this level.
 *   -caption: A string denoting the caption for this level.
 */
function islandora_serial_object_construct_markup(&$form, $children, $parent_id, $intermediate_levels) {
  if (isset($children[$parent_id])) {
    foreach ($children[$parent_id] as $child) {
      $depth = isset($child['depth']) ? $child['depth'] : 0;
      $form[$child['id']] = array(
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
          '#delta' => 9999,
          '#title_display' => 'invisible',
          '#attributes' => array(
            'class' => array('islandora-serial-object-intermediate-weight'),
          ),
        ),
        'depth' => array(
          '#type' => 'hidden',
          '#default_value' => $depth,
        ),
        'ingested' => array(
          '#type' => 'value',
          '#value' => isset($child['ingested']) ? $child['ingested'] : FALSE,
        ),
        'leaf' => array(
          '#type' => 'value',
          '#value' => isset($child['leaf']) ? $child['leaf'] : FALSE,
        ),
      );
      $form[$child['id']] += array(
        'caption' => array(
          '#type' => 'textfield',
          '#default_value' => isset($child['caption']) ? $child['caption'] : $intermediate_levels[$depth + 1]['caption'],
          '#attributes' => array(
            'class' => array('islandora-serial-object-caption'),
          ),
        ),
        'number' => array(
          '#type' => 'textfield',
          '#default_value' => isset($child['number']) ? $child['number'] : FALSE,
        ),
      );
      if ((isset($child['leaf']) && $child['leaf']) || count($intermediate_levels) == 1) {
        $form[$child['id']]['#attributes']['class'][] = 'tabledrag-leaf';
      }
      // If the value of the caption was modified, the default value should not
      // be applied when the row is re-ordered.
      if (isset($child['default_edit']) && !$child['default_edit']) {
        $form[$child['id']]['caption']['#attributes']['class'][] = 'islandora-serial-object-modified-intermediate';
      }
      islandora_serial_object_construct_markup($form, $children, $child['id'], $intermediate_levels);
    }
  }
}

/**
 * Performs a stable sort to preserve order when encountering the same weight.
 *
 * @param array $a
 *   An array describing an intermediate object containing:
 *   -caption: A string describing the caption.
 *   -number: A string identifying the number.
 *   -pid: A string that stores reference to which parent the intermediate is
 *   part of.
 *   -id: A unique string used for identification.
 *   -weight: A string referencing how far down in the order the intermedaite
 *   is.
 *   - depth: A string describing how far "idented" the object is.
 * @param array $b
 *   An array describing an intermediate object containing:
 *   -caption: A string describing the caption.
 *   -number: A string identifying the number.
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

/**
 * Munges the intermediates into a more treelike structure.
 *
 * @param array $intermediates
 *   An array of associative arrays where the key is the pid
 *   (parent id) of the intermediate. They key of each associative array is the
 *   id of the intermediate and contains:
 *     -caption: A string describing the caption.
 *     -number: A string identifying the number.
 *     -pid: A string that stores reference to which parent the intermediate is
 *     part of.
 *     -id: A unique string used for identification.
 *     -weight: A string referencing how far down in the order the intermediate
 *     is.
 *     -depth: A string describing how far "indented" the object is.
 *
 * @param string $root_pid
 *   The PID of the root serial object.
 *
 * @param bool $ingest
 *   Whether the action is generating from the islandora_ingest_form or through
 *   the intermediate manage form.
 *
 * @return array
 *   FALSE if the tree has no intermediate objects or the tree structure of the
 *   serial intermediate objects to set. The tree structure is an array of
 *   arrays where the key is the root serial object pid. Each array is an
 *   an array containing:
 *     - children: An array containing:
 *       - children: An array like this one (recursion).
 *       - caption: A string describing the caption.
 *       - number: A string identifying the number.
 */
function islandora_serial_object_get_intermediates_as_tree($intermediates, $root_pid, $ingest) {
  if (empty($intermediates)) {
    $tree = FALSE;
  }
  else {
    // Set the root pid value first.
    $parent_map = array();
    $parents = array();
    $start = $ingest ? 0 : $root_pid;
    $parents =& $intermediates[$start];
    foreach ($intermediates as &$root) {
      foreach ($root as &$child) {
        $parent_map[$child['id']] =& $child;
        $parent_map[$child['id']]['children'] = array();
        if ($child['pid'] != $root_pid) {
          $parent_map[$child['pid']]['children'][$child['id']] = &$child;
        }
      }
      unset($child);
    }
    unset($root);
    $tree = array(
      $root_pid => array(
        'children' => $parents,
      ),
    );
  }
  return $tree;
}

/**
 * Retrieve all children recursively.
 *
 * @param string $parent_pid
 *   The PID of the object to find the children of.
 * @param bool $tree_structure
 *   Whether the structure is being returned flat (the default), or as a tree.
 *
 * @return string|array
 *   A string of the root if there are no children.
 *   An array containing a tree of PIDs, where leaves are strings.
 */
function islandora_serial_object_get_children($parent_pid, $tree_structure = FALSE) {
  $tuque = islandora_get_tuque_connection();
  $islandora_namespace = ISLANDORA_RELS_EXT_URI;
  $sequence_predicate = ISLANDORA_SERIAL_OBJECT_SEQUENCE_PREDICATE;
  $query = '
    select $object $parent $label $sequence_num from <#ri>
    where (
      $object <fedora-model:label> $label and
      $object <!namespace!sequence> $sequence_num and
      walk(
        $object <fedora-rels-ext:isMemberOf> <info:fedora/' . $parent_pid . '>
        and $object <fedora-rels-ext:isMemberOf> $parent)
    )
  ';
  $query = format_string($query, array(
    '!namespace' => $islandora_namespace,
    '!sequence' => $sequence_predicate,
  ));
  $raw_results = $tuque->repository->ri->itqlQuery($query);
  $leafs = islandora_serial_object_get_pdf_child_objects();
  $flat_structure = array();
  foreach ($raw_results as $result) {
    $flat_structure[$result['object']['value']] = array(
      'object' => $result['object']['value'],
      'parent' => $result['parent']['value'],
      'label' => $result['label']['value'],
      'weight' => $result['sequence_num']['value'],
      'leaf' => isset($leafs[$result['object']['value']]) ? TRUE : FALSE,
    );
  }
  if ($tree_structure) {
    $processed_results = islandora_serial_object_build_tree($flat_structure, $parent_pid);
  }
  else {
    $processed_results = array();
    $tree = islandora_serial_object_build_tree($flat_structure, $parent_pid);
    if ($tree != $parent_pid) {
      islandora_serial_object_build_flat_structure_from_tree(
        $tree[$parent_pid],
        array_map('islandora_serial_object_format_values_for_flat_structure', $flat_structure),
        $processed_results,
        $parent_pid
      );
    }
  }
  return $processed_results;
}

/**
 * Builds the flat structure for the form from a tree.
 *
 * @param array|string $tree
 *   A string of the root if there are no children. An array containing a tree
 *   of PIDs, where leaves are strings.
 * @param array $flat_structure
 *   An array indexed by unique ids containing arrays that have:
 *     -pid: A string that stores reference to which parent the intermediate is
 *     part of.
 *     -id: A unique string used for identification.
 *     -weight: A string referencing how far down in the order the intermediate
 *     is.
 *     -ingested: A bool denoting whether or not the object has already
 *     been ingested.
 *     -leaf: A bool denoting whether or not the intermediate is a leaf.
 * @param array $results
 *   A passed by reference array that is used to retain values as the flat
 *   structure is built up for the form.
 * @param string $parent_pid
 *   The pid of the root serial object.
 * @param int $depth
 *   The current depth of the tree.
 */
function islandora_serial_object_build_flat_structure_from_tree($tree, $flat_structure, &$results, $parent_pid, $depth = 1) {
  $levels =& drupal_static(__FUNCTION__, NULL);
  if ($levels === NULL) {
    $parent_object = islandora_object_load($parent_pid);
    $levels = json_decode($parent_object['SERIAL-LEVELS']->content, TRUE);
  }
  foreach ($tree as $branch => $vals) {
    if (is_array($vals)) {
      $results[$flat_structure[$branch]['pid']][$branch] = $flat_structure[$branch];
      $mods_data = islandora_serial_object_retrieve_mods_data($branch, $levels[$depth]['type']);
      $default_edit = ($mods_data['caption'] == $levels[$depth]['caption']) ? TRUE : FALSE;
      $results[$flat_structure[$branch]['pid']][$branch]['caption'] = $mods_data['caption'];
      $results[$flat_structure[$branch]['pid']][$branch]['number'] = $mods_data['number'];
      $results[$flat_structure[$branch]['pid']][$branch]['default_edit'] = $default_edit;
      $results[$flat_structure[$branch]['pid']][$branch]['default_type'] = htmlspecialchars($levels[$depth]['type']);
      islandora_serial_object_build_flat_structure_from_tree($vals, $flat_structure, $results, $parent_pid, $depth + 1);
    }
    else {
      $mods_data = islandora_serial_object_retrieve_mods_data($vals, $levels[$depth]['type']);
      $default_edit = ($mods_data['caption'] == $levels[$depth]['caption']) ? TRUE : FALSE;
      $results[$flat_structure[$vals]['pid']][$vals] = $flat_structure[$vals];
      $results[$flat_structure[$vals]['pid']][$vals]['caption'] = $mods_data['caption'];
      $results[$flat_structure[$vals]['pid']][$vals]['number'] = $mods_data['number'];
      $results[$flat_structure[$vals]['pid']][$vals]['default_edit'] = $default_edit;
      $results[$flat_structure[$vals]['pid']][$vals]['default_type'] = htmlspecialchars($levels[$depth]['type']);
    }
  }
}

/**
 * Mapping function to transform values from the RI to the expected form.
 *
 * @param array $values
 *   An array of values returned from the RI query containing:
 *   -object: A string denoting the object's PID in Fedora.
 *   -parent: A string denoting the object's parent PID in Fedora.
 *   -title; A string denoting the object's label.
 *   -weight: An integer denoting the weight of the object.
 *   -leaf: A bool denoting whether or not the intermediate is a leaf.
 *
 * @return array
 *   The formatted array for use in constructing the form.
 */
function islandora_serial_object_format_values_for_flat_structure($values) {
  return array(
    'id' => $values['object'],
    'pid' => $values['parent'],
    'title' => $values['label'],
    'weight' => $values['weight'],
    'ingested' => TRUE,
    'leaf' => $values['leaf'],
  );
}

/**
 * Turn a flat array structure of objects into a tree.
 *
 * @param array $flat_structure
 *   An array of arrays containing:
 *    -object: A string of the current object's PID.
 *    -parent: a string of the parent's PID.
 * @param string $root
 *   The root PID to build the tree for.
 *
 * @return string|array
 *   A string of the root if there are no children.
 *   An array containing a tree of PIDs, where leaves are strings.
 */
function islandora_serial_object_build_tree($flat_structure, $root) {
  $tree = array();
  $branches = array();
  foreach ($flat_structure as $result) {
    if ($result['parent'] == $root) {
      $child = islandora_serial_object_build_tree(
        $flat_structure,
        $result['object']
      );
      if (is_array($child)) {
        $branches = $branches + $child;
      }
      else {
        $branches[] = $child;
      }
    }
  }
  if ($branches) {
    $tree[$root] = $branches;
  }
  else {
    $tree = $root;
  }
  return $tree;
}

/**
 * Determines whether or not the intermediate object is a leaf.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing a Fedora object.
 *
 * @return bool
 *   TRUE if the intermediate is a leaf, FALSE otherwise.
 */
function islandora_serial_object_is_leaf(AbstractObject $object) {
  $leaf_query = <<<EOQ
SELECT ?child from <#RI> WHERE {
  ?child <fedora-rels-ext:isMemberOf> <info:fedora/{$object->id}> ;
         <fedora-model:hasModel> <info:fedora/islandora:intermediateCModel>
}
EOQ;
  return !($object->repository->ri->countQuery($leaf_query, 'sparql'));
}

/**
 * Retrieves the highest sequence number of a PDF child.
 *
 * @param string $parent_id
 *   The pid of the Fedora object that is the parent.
 *
 * @return int
 *   0 if the object has no child, the maximum otherwise.
 */
function islandora_serial_object_get_max_sequence_number($parent_id) {
  $tuque = islandora_get_tuque_connection();
  // Need to append the sequence of this PDF to the end by default.
  $sequence_predicate = ISLANDORA_SERIAL_OBJECT_SEQUENCE_PREDICATE;
  $pdf_parent_predicate = ISLANDORA_SERIAL_OBJECT_PDF_PARENT_PREDICATE;
  $islandora_namespace = ISLANDORA_RELS_EXT_URI;
  $sequence_query = <<<'EOQ'
  select $sequence_number from <#ri> where
    $child <!parent_predicate> <info:fedora/!pid> and
    $child <!sequence_predicate> $sequence_number
  order by $sequence_number desc
  limit 1
EOQ;
  $formatted_query = format_string($sequence_query, array(
    '!parent_predicate' => "$islandora_namespace$pdf_parent_predicate",
    '!pid' => $parent_id,
    '!sequence_predicate' => "$islandora_namespace$sequence_predicate",
  ));
  $sequence_number_results = $tuque->repository->ri->itqlQuery($formatted_query);
  $sequence_number = reset($sequence_number_results);
  return $sequence_number ? $sequence_number['sequence_number']['value'] : 0;
}

/**
 * Creates a listing of all objects that have PDFs associated to them.
 *
 * @return array
 *   An array containing all intermediates that have PDF children on them.
 */
function islandora_serial_object_get_pdf_child_objects() {
  $connection = islandora_get_tuque_connection();
  $sequence_predicate = ISLANDORA_SERIAL_OBJECT_SEQUENCE_PREDICATE;
  $pdf_parent_predicate = ISLANDORA_SERIAL_OBJECT_PDF_PARENT_PREDICATE;
  $islandora_namespace = ISLANDORA_RELS_EXT_URI;
  $pdf_child_query = <<<EOQ
SELECT ?sequence_number ?obj ?pdf ?pdf_label FROM <#ri> WHERE {
  ?obj <fedora-model:hasModel> <info:fedora/islandora:intermediateCModel> .
  ?pdf <$islandora_namespace{$pdf_parent_predicate}> ?obj ;
       <fedora-model:label> ?pdf_label ;
  <$islandora_namespace{$sequence_predicate}> ?sequence_number
  }
  ORDER BY ?sequence_number
EOQ;
  $results = $connection->repository->ri->sparqlQuery($pdf_child_query);
  $leaf_objects = array();
  foreach ($results as $result) {
    if (!isset($leaf_objects[$result['obj']['value']])) {
      $leaf_objects[$result['obj']['value']] = array();
    }
    $leaf_objects[$result['obj']['value']][$result['pdf']['value']] = array(
      'pid' => $result['pdf']['value'],
      'label' => $result['pdf_label']['value'],
    );
  }
  return $leaf_objects;
}
/**
 * Creates a listing of all objects that have the stub content model.
 *
 * @return array
 *   An array containing all intermediates that have PDF children on them.
 */
function islandora_serial_object_get_stub_child_objects() {
  $connection = islandora_get_tuque_connection();
  $pdf_child_query = <<<EOQ
SELECT ?obj ?label FROM <#ri> WHERE {
  ?obj <fedora-model:hasModel> <info:fedora/islandora:intermediateSerialCModelStub> ;
       <fedora-model:label> ?label
}
EOQ;
  $results = $connection->repository->ri->sparqlQuery($pdf_child_query);
  $leaf_objects = array();
  foreach ($results as $result) {
    $leaf_objects[$result['obj']['value']] = array(
      'pid' => $result['obj']['value'],
      'label' => $result['label']['value'],
    );
  }
  return $leaf_objects;
}
/**
 * Go grab a structure to be used for JSONing for the tree view.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 *
 * @return array
 *   An array structure to be encoded into JSON containing:
 *   -data: An array containing:
 *     -id: A string of the PID of the object.
 *     -parent: A string of the parent PID of the object.
 *     -text: A string of the label of the object.
 *     -type: A string denoting how the node is to be rendered, either leaf,
 *     branch or pdf.
 *   -types: An array defining all the types used in the tree.
 *   -plugins: An array of plugins to include for the tree.
 */
function islandora_serial_object_get_json_for_tree(AbstractObject $object) {
  // Need to get the correct depth and ordering on our existing elements.
  $intermediates['intermediates'] = islandora_serial_object_get_children($object->id);
  // Need to get the correct depth and ordering on our existing elements.
  $intermediates = islandora_serial_object_sort_intermediates($intermediates, $object->id);
  $json_data = array();
  if (variable_get('islandora_serial_object_display_stubs_pdfs_in_tree', FALSE)) {
    $leafs = islandora_serial_object_get_pdf_child_objects();
    $stubs = islandora_serial_object_get_stub_child_objects();
  }
  else {
    $leafs = array();
    $stubs = array();
  }
  foreach ($intermediates['intermediates'] as $children) {
    foreach ($children as $values) {
      if (isset($stubs[$values['id']])) {
        $type = 'stub';
      }
      else {
        $type = $values['leaf'] ? 'leaf' : 'branch';
      }
      $json_data[] = array(
        'id' => $values['id'],
        'parent' => $values['pid'] == $object->id ? '#' : $values['pid'],
        'text' => $values['title'],
        'type' => $type,
      );
      if ($values['leaf'] && variable_get('islandora_serial_object_display_stubs_pdfs_in_tree', FALSE)) {
        if (isset($leafs[$values['id']])) {
          foreach ($leafs[$values['id']] as $pdf_child) {
            if (isset($stubs[$pdf_child['pid']])) {
              $type = 'stub';
            }
            else {
              $type = 'pdf';
            }
            $json_data[] = array(
              'id' => $pdf_child['pid'],
              'parent' => $values['id'],
              'text' => $pdf_child['label'],
              'type' => $type,
            );
          }
        }
      }
    }
  }
  return array(
    'data' => $json_data,
    'types' => array(
      'branch' => array(),
      'leaf' => array(),
      'pdf' => array(),
      'stub' => array(),
    ),
    'plugins' => array(
      'types',
      'ui',
      'state',
    ),
  );
}

/**
 * Constructs the tree view for display purposes.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 *
 * @return array
 *   An array representing the jsTree view to be rendered.
 */
function islandora_serial_object_get_tree_view(AbstractObject $object) {
  $form = array();
  $json = islandora_serial_object_get_json_for_tree($object);
  if (!empty($json['data'])) {
    $module_path = drupal_get_path('module', 'islandora_serial_object');
    $library_path = libraries_get_path('jstree');
    $form['serial_object_tree'] = array(
      '#markup' => '<div id="islandora_serial_object_tree_view"></div>',
    );
    $form['#attached']['css'] = array(
      "$library_path/dist/themes/default/style.min.css",
    );
    $form['#attached']['js'] = array(
      "$library_path/dist/libs/jquery.js" => array(
        'group' => JS_LIBRARY,
      ),
      "$library_path/dist/jstree.min.js" => array(
        'group' => JS_LIBRARY,
      ),
      "$module_path/js/jstree.js" => array(
        'group' => JS_LIBRARY,
      ),
    );
    $form['#attached']['js'][] = array(
      'data' => array('islandora_serial_object' => $json),
      'type' => 'setting',
    );
    $form['collapse_all'] = array(
      '#type' => 'button',
      '#value' => t('Collapse all'),
      '#attributes' => array(
        'class' => array('islandora_serial_object_tree_close_all'),
      ),
    );
    $form['description'] = array(
      '#type' => 'item',
      '#markup' => t('Left click to navigate to the object.'),
    );
  }
  return $form;
}

/**
 * Helper function to replace a portion of a dynamic yet constant XPath.
 *
 * @param string $xpath
 *   An XPath.
 * @param string $replacement
 *   The replacement value for the XPath.
 *
 * @return string
 *   The replaced XPath.
 */
function islandora_serial_object_replace_xpath($xpath, $replacement) {
  return format_string($xpath, array(
    '!replacement' => $replacement,
  ));
}

/**
 * Helper function to retrieve MODS data from an object.
 *
 * @param string $id
 *   The pid of the Fedora object to be parsed.
 * @param string $type
 *   The type value to be used for XPath replacing.
 *
 * @return array
 *   An array containing:
 *   -caption: A string with the populated caption value.
 *   -number: A string the populated number value.
 */
function islandora_serial_object_retrieve_mods_data($id, $type) {
  $data = array();
  $object = islandora_object_load($id);
  $mods_string = $object['MODS']->content;
  $mods_doc = new DOMDocument();
  $mods_doc->loadXML($mods_string);
  $mods_xpath = new DOMXPath($mods_doc);
  $mods_xpath->registerNamespace('mods', 'http://www.loc.gov/mods/v3');
  $caption_xpath = islandora_serial_object_replace_xpath(ISLANDORA_SERIAL_OBJECT_MODS_CAPTION_XPATH, htmlspecialchars($type));
  $mods_caption_xpath = $mods_xpath->query($caption_xpath);
  if ($mods_caption_xpath->length) {
    $mods_caption = $mods_caption_xpath->item(0);
    $data['caption'] = htmlspecialchars_decode($mods_caption->nodeValue);
  }

  $number_xpath = islandora_serial_object_replace_xpath(ISLANDORA_SERIAL_OBJECT_MODS_NUMBER_XPATH, htmlspecialchars($type));
  $mods_number_xpath = $mods_xpath->query($number_xpath);
  if ($mods_number_xpath->length) {
    $mods_number = $mods_number_xpath->item(0);
    $data['number'] = htmlspecialchars_decode($mods_number->nodeValue);
  }
  return $data;
}

/**
 * Helper function that filters out a source MODS array.
 *
 * @param array $mods_array
 *   The array to be filtered, containing:
 *   -caption: A string containing the caption value from MODS.
 *   -number: A string contaning the number value from MODS.
 *
 * @return array
 *   A filtered array.
 */
function islandora_serial_object_filter_mods_data($mods_array) {
  $empty_filter = function ($item) {
    if ($item === '') {
      return FALSE;
    }
    return TRUE;
  };
  return array_filter($mods_array, $empty_filter);
}
