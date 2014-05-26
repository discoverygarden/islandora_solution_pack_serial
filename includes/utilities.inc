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
  islandora_serial_object_get_tree_for_form($intermediates['intermediates'], 0);
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
          '#delta' => 9999,
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

/**
 * Munges the intermediates into a more treelike structure.
 *
 * @param array $intermediates
 *   An array of associative arrays where the key is the pid
 *   (parent id) of the intermediate. They key of each associative array is the
 *   id of the intermediate and contains:
 *     -title: A string describing the object.
 *     -volume: A string identifying the volume.
 *     -issue: A string identifying the issue.
 *     -year: A string identifying the year.
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
 * @return array
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
function islandora_serial_object_get_intermediates_as_tree($intermediates, $root_pid) {
  // Set the root pid value first.
  $parent_map = array();
  $parents = array();
  $parents =& $intermediates[0];

  foreach ($intermediates as &$root) {
    foreach ($root as &$child) {
      $parent_map[$child['id']] =& $child;
      $parent_map[$child['id']]['children'] = array();
      if ($child['pid'] > 0) {
        $parent_map[$child['pid']]['children'][] = &$child;
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
  return $tree;
}

/**
 * Retrieve all children recursively.
 *
 * @param string $parent_pid
 *   The PID of the object to find the children of.
 *
 * @return string|array
 *   A string of the root if there are no children.
 *   An array containing a tree of PIDs, where leaves are strings.
 */
function islandora_serial_object_get_children($parent_pid) {
  $tuque = islandora_get_tuque_connection();
  $query = '
    select $object $parent from <#ri>
    where (
      walk(
        $object <fedora-rels-ext:isMemberOf> <info:fedora/' . $parent_pid . '>
        and $object <fedora-rels-ext:isMemberOf> $parent)
    )
  ';
  $raw_results = $tuque->repository->ri->itqlQuery($query);
  $flat_structure = array();
  foreach ($raw_results as $result) {
    $flat_structure[] = array(
      'object' => $result['object']['value'],
      'parent' => $result['parent']['value'],
    );
  }
  $processed_results = islandora_serial_object_build_tree($flat_structure, $parent_pid);
  return $processed_results;
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
  $sequence_query = <<<EOQ
  SELECT ?sequence_number FROM <#ri> WHERE {
    ?child <$islandora_namespace$pdf_parent_predicate> <info:fedora/{$parent_id}> ;
           <$islandora_namespace{$sequence_predicate}> ?sequence_number
  }
  ORDER BY DESC(?sequence_number)
  LIMIT 1
EOQ;
  $sequence_number_results = $tuque->repository->ri->sparqlQuery($sequence_query);
  $sequence_number = reset($sequence_number_results);
  return $sequence_number ? $sequence_number['sequence_number']['value'] : 0;
}
