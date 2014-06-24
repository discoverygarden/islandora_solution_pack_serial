<?php
/**
 * @file
 * Holds a default view for displaying PDF children on leaves.
 */

/**
 * Implements hook_views_default_views().
 */
function islandora_serial_object_views_default_views() {
  $export = array();
  $view = new view();
  $view->name = 'islandora_serial_object_intermediate_pdf_view';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'islandora_solr';
  $view->human_name = 'Islandora Serial Object Intermediate PDF View';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'PDF Objects';
  $handler->display->display_options['use_ajax'] = TRUE;
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'none';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'mini';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['pager']['options']['id'] = '0';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'mods_titleInfo_title_ms' => 'mods_titleInfo_title_ms',
    'mods_name_corporate_namePart_ms' => 'mods_name_corporate_namePart_ms',
    'mods_name_personal_namePart_ms' => 'mods_name_corporate_namePart_ms',
  );
  $handler->display->display_options['style_options']['default'] = '-1';
  $handler->display->display_options['style_options']['info'] = array(
    'mods_titleInfo_title_ms' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'mods_name_corporate_namePart_ms' => array(
      'align' => '',
      'separator' => ', ',
      'empty_column' => 0,
    ),
    'mods_name_personal_namePart_ms' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  /* No results behavior: Global: Text area */
  $handler->display->display_options['empty']['area']['id'] = 'area';
  $handler->display->display_options['empty']['area']['table'] = 'views';
  $handler->display->display_options['empty']['area']['field'] = 'area';
  $handler->display->display_options['empty']['area']['empty'] = TRUE;
  $handler->display->display_options['empty']['area']['content'] = 'No PDFs are currently associated.';
  $handler->display->display_options['empty']['area']['format'] = 'filtered_html';
  /* Field: Islandora Solr: PID */
  $handler->display->display_options['fields']['PID']['id'] = 'PID';
  $handler->display->display_options['fields']['PID']['table'] = 'islandora_solr';
  $handler->display->display_options['fields']['PID']['field'] = 'PID';
  $handler->display->display_options['fields']['PID']['label'] = '';
  $handler->display->display_options['fields']['PID']['exclude'] = TRUE;
  $handler->display->display_options['fields']['PID']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['PID']['link_to_object'] = 0;
  /* Field: Islandora Solr: fgs_label_s */
  $handler->display->display_options['fields']['fgs_label_s']['id'] = 'fgs_label_s';
  $handler->display->display_options['fields']['fgs_label_s']['table'] = 'islandora_solr';
  $handler->display->display_options['fields']['fgs_label_s']['field'] = 'fgs_label_s';
  $handler->display->display_options['fields']['fgs_label_s']['label'] = '';
  $handler->display->display_options['fields']['fgs_label_s']['exclude'] = TRUE;
  $handler->display->display_options['fields']['fgs_label_s']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['fgs_label_s']['link_to_object'] = 0;
  /* Field: Islandora Solr: mods_titleInfo_title_ms */
  $handler->display->display_options['fields']['mods_titleInfo_title_ms']['id'] = 'mods_titleInfo_title_ms';
  $handler->display->display_options['fields']['mods_titleInfo_title_ms']['table'] = 'islandora_solr';
  $handler->display->display_options['fields']['mods_titleInfo_title_ms']['field'] = 'mods_titleInfo_title_ms';
  $handler->display->display_options['fields']['mods_titleInfo_title_ms']['label'] = 'Title';
  $handler->display->display_options['fields']['mods_titleInfo_title_ms']['alter']['make_link'] = TRUE;
  $handler->display->display_options['fields']['mods_titleInfo_title_ms']['alter']['path'] = 'islandora/object/[PID]/datastream/OBJ/view';
  $handler->display->display_options['fields']['mods_titleInfo_title_ms']['alter']['alt'] = 'PDF View';
  $handler->display->display_options['fields']['mods_titleInfo_title_ms']['empty'] = '[fgs_label_s]';
  $handler->display->display_options['fields']['mods_titleInfo_title_ms']['link_to_object'] = 0;
  /* Field: Islandora Solr: mods_name_corporate_namePart_ms */
  $handler->display->display_options['fields']['mods_name_corporate_namePart_ms']['id'] = 'mods_name_corporate_namePart_ms';
  $handler->display->display_options['fields']['mods_name_corporate_namePart_ms']['table'] = 'islandora_solr';
  $handler->display->display_options['fields']['mods_name_corporate_namePart_ms']['field'] = 'mods_name_corporate_namePart_ms';
  $handler->display->display_options['fields']['mods_name_corporate_namePart_ms']['label'] = 'Authors';
  $handler->display->display_options['fields']['mods_name_corporate_namePart_ms']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['mods_name_corporate_namePart_ms']['link_to_object'] = 0;
  /* Field: Islandora Solr: mods_name_personal_namePart_ms */
  $handler->display->display_options['fields']['mods_name_personal_namePart_ms']['id'] = 'mods_name_personal_namePart_ms';
  $handler->display->display_options['fields']['mods_name_personal_namePart_ms']['table'] = 'islandora_solr';
  $handler->display->display_options['fields']['mods_name_personal_namePart_ms']['field'] = 'mods_name_personal_namePart_ms';
  $handler->display->display_options['fields']['mods_name_personal_namePart_ms']['label'] = '';
  $handler->display->display_options['fields']['mods_name_personal_namePart_ms']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['mods_name_personal_namePart_ms']['link_to_object'] = 0;
  /* Sort criterion: Islandora Solr: RELS_EXT_sequence_position_literal_l */
  $handler->display->display_options['sorts']['RELS_EXT_sequence_position_literal_l']['id'] = 'RELS_EXT_sequence_position_literal_l';
  $handler->display->display_options['sorts']['RELS_EXT_sequence_position_literal_l']['table'] = 'islandora_solr';
  $handler->display->display_options['sorts']['RELS_EXT_sequence_position_literal_l']['field'] = 'RELS_EXT_sequence_position_literal_l';
  $handler->display->display_options['sorts']['RELS_EXT_sequence_position_literal_l']['expose']['label'] = 'RELS_EXT_sequence_position_literal_l';
  /* Contextual filter: Islandora Solr: RELS_EXT_isComponentOf_uri_mt */
  $handler->display->display_options['arguments']['RELS_EXT_isComponentOf_uri_mt']['id'] = 'RELS_EXT_isComponentOf_uri_mt';
  $handler->display->display_options['arguments']['RELS_EXT_isComponentOf_uri_mt']['table'] = 'islandora_solr';
  $handler->display->display_options['arguments']['RELS_EXT_isComponentOf_uri_mt']['field'] = 'RELS_EXT_isComponentOf_uri_mt';
  $handler->display->display_options['arguments']['RELS_EXT_isComponentOf_uri_mt']['default_action'] = 'default';
  $handler->display->display_options['arguments']['RELS_EXT_isComponentOf_uri_mt']['default_argument_type'] = 'raw';
  $handler->display->display_options['arguments']['RELS_EXT_isComponentOf_uri_mt']['default_argument_options']['index'] = '2';
  $handler->display->display_options['arguments']['RELS_EXT_isComponentOf_uri_mt']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['RELS_EXT_isComponentOf_uri_mt']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['RELS_EXT_isComponentOf_uri_mt']['summary_options']['items_per_page'] = '25';
  $handler->display->display_options['arguments']['RELS_EXT_isComponentOf_uri_mt']['specify_validation'] = TRUE;
  $handler->display->display_options['arguments']['RELS_EXT_isComponentOf_uri_mt']['validate']['type'] = 'php';
  $handler->display->display_options['arguments']['RELS_EXT_isComponentOf_uri_mt']['validate_options']['code'] = 'module_load_include(\'inc\', \'islandora_serial_object\', \'includes/utilities\');
$object = islandora_object_load($argument);
return $object && islandora_serial_object_is_leaf($object) && in_array(\'islandora:intermediateCModel\', $object->models);
  ';
  $handler->display->display_options['filter_groups']['operator'] = 'OR';
  $handler->display->display_options['filter_groups']['groups'] = array(
    1 => 'OR',
    2 => 'OR',
  );
  /* Filter criterion: Islandora Solr: RELS_EXT_hasModel_uri_ms */
  $handler->display->display_options['filters']['RELS_EXT_hasModel_uri_ms']['id'] = 'RELS_EXT_hasModel_uri_ms';
  $handler->display->display_options['filters']['RELS_EXT_hasModel_uri_ms']['table'] = 'islandora_solr';
  $handler->display->display_options['filters']['RELS_EXT_hasModel_uri_ms']['field'] = 'RELS_EXT_hasModel_uri_ms';
  $handler->display->display_options['filters']['RELS_EXT_hasModel_uri_ms']['value'] = 'info:fedora/islandora:sp_pdf';
  $handler->display->display_options['filters']['RELS_EXT_hasModel_uri_ms']['group'] = 1;
  /* Filter criterion: Islandora Solr: RELS_EXT_hasModel_uri_ms */
  $handler->display->display_options['filters']['RELS_EXT_hasModel_uri_ms_1']['id'] = 'RELS_EXT_hasModel_uri_ms_1';
  $handler->display->display_options['filters']['RELS_EXT_hasModel_uri_ms_1']['table'] = 'islandora_solr';
  $handler->display->display_options['filters']['RELS_EXT_hasModel_uri_ms_1']['field'] = 'RELS_EXT_hasModel_uri_ms';
  $handler->display->display_options['filters']['RELS_EXT_hasModel_uri_ms_1']['value'] = 'info:fedora/islandora:intermediateSerialCModelStub';
  $handler->display->display_options['filters']['RELS_EXT_hasModel_uri_ms_1']['group'] = 2;

  /* Display: Block */
  $handler = $view->new_display('block', 'Block', 'islandora_serial_object_intermediate_pdf_view');
  $translatables['islandora_serial_object_intermediate_pdf_view'] = array(
    t('Master'),
    t('PDF Objects'),
    t('more'),
    t('Apply'),
    t('Reset'),
    t('Sort by'),
    t('Asc'),
    t('Desc'),
    t('Items per page'),
    t('- All -'),
    t('Offset'),
    t('« first'),
    t('‹ previous'),
    t('next ›'),
    t('last »'),
    t('No PDFs are currently associated.'),
    t('Title'),
    t('PDF View'),
    t('[fgs_label_s]'),
    t('Authors'),
    t('RELS_EXT_sequence_position_literal_l'),
    t('All'),
    t('Block'),
  );
  $export['islandora_serial_object_intermediate_pdf_view'] = $view;
  return $export;
}
