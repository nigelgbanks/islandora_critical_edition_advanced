<?php

/**
 * @file
 * Display / Manage TEI.
 */

/**
 * Display's the given Versionable Object's TEI-RDF's TEI datastream.
 *
 * @global type $base_url
 *
 * @param AbstractObject $object
 *   The Versionable Object.
 *
 * @return array
 *   The TEI page for the given Versionable Object.
 */
function islandora_critical_edition_advanced_versionable_object_tei_local_task(AbstractObject $object) {
  module_load_include('inc', 'islandora_critical_edition_advanced', 'includes/utilities');
  module_load_include('inc', 'islandora_critical_edition_advanced', 'includes/consolidate_tei.form');
  module_load_include('inc', 'islandora_critical_edition_advanced', 'includes/upload_tei.form');
  if (!in_array('islandora:versionableObjectCModel', $object->models)) {
    // Don't render anything for non Versionable Objects.
    return array();
  }
  // Get the TEI-RDF object if it exists.
  $tei_rdfs = array_keys(islandora_critical_edition_get_members($object->id, 'islandora:criticalEditionCModel'));
  $tei_rdf = islandora_object_load($tei_rdfs[0]);
  $has_tei = $tei_rdf && isset($tei_rdf['TEI']);
  return array(
    'actions' => array(
      '#type' => 'vertical_tabs',
      'consolidate_form' => array(
        '#type' => 'fieldset',
        '#title' => 'Consolidate TEI',
        'form' => $tei_rdf ? drupal_get_form('islandora_critical_edition_advanced_consolidate_tei_form', $tei_rdf->id) : NULL,
      ),
      'upload_form' => array(
        '#type' => 'fieldset',
        '#title' => 'Upload TEI',
        'form' => $tei_rdf ? drupal_get_form('islandora_critical_edition_advanced_upload_tei_form', $tei_rdf->id) : NULL,
      ),
    ),
    'download' => array(
      '#prefix' => '<span>',
      '#markup' => $has_tei ? l(t('Download TEI'), "islandora/object/{$tei_rdf->id}/datastream/TEI/download") : NULL,
      '#suffix' => '</span>',
    ),
    'tei' => array(
      '#prefix' => '<h1>TEI</h1>',
      '#markup' => $has_tei ? islandora_critical_edition_advanced_transform_tei_datastream_to_html($tei_rdf->id) : t('No TEI Present, please upload or conslidate the TEI to display.'),
    ),
  );

  if ($tei_rdf) {
    $output['conslidate_form'] = array(
      '#type' => 'fieldset',
      '#title' => 'Consolidate TEI',
      'form' => drupal_get_form('islandora_critical_edition_advanced_consolidate_tei_form', $tei_rdf->id),
    );
    $output['conslidate_form']['form']['submit']['#value'] = t('Conslidate TEI');
    $output['upload_form'] = drupal_get_form('islandora_critical_edition_advanced_upload_tei_form', $tei_rdf->id);
  }
  if (isset($tei_rdf['TEI'])) {
    $output['TEI'] = array(
      '#prefix' => '<h1>TEI</h1>',
      '#markup' => islandora_critical_edition_advanced_transform_tei_datastream_to_html($tei_rdf->id),
    );
  }
  else {
    $output['TEI'] = array(
      '#prefix' => '<h1>TEI</h1>',
      '#markup' => t('No TEI Present, please upload or conslidate the TEI to display.'),
    );
  }
  return $output;
}

/**
 * Converts the given Object's TEI datastream to HTML.
 *
 * @param string $pid
 *   The Object's PID.
 *
 * @return string
 *   HTML representing the given Objects TEI datastream.
 */
function islandora_critical_edition_advanced_transform_tei_datastream_to_html($pid) {
  module_load_include('inc', 'islandora', 'includes/authtokens');
  $tomcat_url = str_replace('/fedora', '', variable_get('islandora_base_url', 'http://localhost:8080/fedora'));
  $xml_path = url("islandora/object/{$pid}/datastream/TEI/view", array(
    'absolute' => TRUE,
    'query' => array(
      'token' => islandora_get_object_token($pid, 'TEI'),
    ),
  ));
  $module_path = drupal_get_path('module', 'islandora_critical_edition_advanced');
  drupal_add_css("$module_path/css/islandora_critical_edition_common_tei.css");
  drupal_add_css("$module_path/css/islandora_critical_edition_reading_tei.css");
  $xslt_path = url("{$module_path}/xml/EMiC_test_XSLT.xsl", array('absolute' => TRUE));
  $xml = file_get_contents("$tomcat_url/saxon/SaxonServlet?source=$xml_path&style=$xslt_path");
  $sxml = new SimpleXMLElement(file_get_contents("$tomcat_url/saxon/SaxonServlet?source=$xml_path&style=$xslt_path"));
  $body = "";
  if(!empty($sxml->body)) {
    foreach ($sxml->body->children() as $child) {
      $body .= $child->asXML();
    }
  }
  return $body ? $body : $xml;
}
