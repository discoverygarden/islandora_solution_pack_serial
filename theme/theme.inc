<?php
/**
 * @file
 * Theme functions for the Islandora Serial solution pack.
 */

/**
 * Implemenets theme_element().
 */
function theme_islandora_serial_object_intermediates_table(array $variables) {
  $form = $variables['form'];

  // Initialize the variable which will store our table rows.
  $rows = array();

  // Iterate over each element in our $form['example_items'] array.
  foreach (element_children($form) as $id) {

    // To support the tabledrag behaviour, we need to assign each row of the
    // table a class attribute of 'draggable'. This will add the 'draggable'
    // class to the <tr> element for that row when the final table is
    // rendered.
    $class = array('draggable');

    // We can add the 'tabledrag-root' class to a row in order to indicate
    // that the row may not be nested under a parent row.  In our sample data
    // for this example, the description for the item with id '8' flags it as
    // a 'root' item which should not be nested.
    /*if ($id == '8') {
      $class[] = 'tabledrag-root';
    }

    // We can add the 'tabledrag-leaf' class to a row in order to indicate
    // that the row may not contain child rows.  In our sample data for this
    // example, the description for the item with id '9' flags it as a 'leaf'
    // item which can not contain child items.
    if ($id == '9') {
      $class[] = 'tabledrag-leaf';
    }
    */
    // If this is a child element, we need to add some indentation to the row,
    // so that it appears nested under its parent.  Our $depth parameter was
    // calculated while building the tree in tabledrag_example_parent_get_data
    $indent = theme('indentation', array('size' => $form[$id]['depth']['#value']));
    unset($form[$id]['depth']);

    // We are now ready to add each element of our $form data to the $rows
    // array, so that they end up as individual table cells when rendered
    // in the final table.  We run each element through the drupal_render()
    // function to generate the final html markup for that element.
    $rows[] = array(
      'data' => array(
        // Include our indentation.
        $indent . drupal_render($form[$id]['title']),
        drupal_render($form[$id]['volume']),
        drupal_render($form[$id]['issue']),
        drupal_render($form[$id]['year']),
        drupal_render($form[$id]['id']),
        drupal_render($form[$id]['pid']),
        drupal_render($form[$id]['weight']),
      ),
      // To support the tabledrag behaviour, we need to assign each row of the
      // table a class attribute of 'draggable'. This will add the 'draggable'
      // class to the <tr> element for that row when the final table is
      // rendered.
      'class' => $class,
    );
  }

  // We now define the table header values.  Ensure that the 'header' count
  // matches the final column count for your table.
  //
  // Normally, we would hide the headers on our hidden columns, but we are
  // leaving them visible in this example.
  // $header = array(t('Name'), t('Description'), '', '', '');
  $header = array(
    t('Title'),
    t('Volume'),
    t('Issue'),
    t('Year'),
    t('ID'),
    t('PID'),
    t('Weight'),
  );

  // We also need to pass the drupal_add_tabledrag() function an id which will
  // be used to identify the <table> element containing our tabledrag form.
  // Because an element's 'id' should be unique on a page, make sure the value
  // you select is NOT the same as the form ID used in your form declaration.
  $table_id = 'islandora-serial-object-intermediate-table';

  // We can render our tabledrag table for output.
  $output = theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => $table_id),
    'empty' => t('No intermediate children on the serial!'),
  ));
  // And then render any remaining form elements (such as our submit button).
  $output .= drupal_render_children($form);
  // We now call the drupal_add_tabledrag() function in order to add the
  // tabledrag.js goodness onto our page.
  //
  // For our parent/child tree table, we need to pass it:
  // - the $table_id of our <table> element (example-items-table),
  // - the $action to be performed on our form items ('match'),
  // - a string describing where $action should be applied ('parent'),
  // - the $group value (pid column) class name ('example-item-pid'),
  // - the $subgroup value (pid column) class name ('example-item-pid'),
  // - the $source value (id column) class name ('example-item-id'),
  // - an optional $hidden flag identifying if the columns should be hidden,
  // - an optional $limit parameter to control the max parenting depth.
  drupal_add_tabledrag($table_id, 'match', 'parent', 'islandora-serial-object-intermediate-pid', 'islandora-serial-object-intermediate-pid', 'islandora-serial-object-intermediate-id', FALSE);

  // Because we also want to sort in addition to providing parenting, we call
  // the drupal_add_tabledrag function again, instructing it to update the
  // weight field as items at the same level are re-ordered.
  drupal_add_tabledrag($table_id, 'order', 'sibling', 'islandora-serial-object-intermediate-weight');

  return $output;
}
