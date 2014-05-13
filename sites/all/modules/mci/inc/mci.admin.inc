<?php

/**
 * Lists all available importers
 */
function mci_admin_form($form,&$form_state) {
  $importers = mci_importer_load_multiple();

  $header = array(
    t('Name'),
    t('Primary Column'),
    t('Status'),
    t('Actions'),
  );

  $rows = array();

  foreach ($importers as $importer) {
    $row = array();
    $row[] = "<strong>".$importer->name."</strong><br /><span class='description'>".$importer->description."</span>";

    $row[] = (!empty($importer->config['primary_column'])) ? $importer->config['primary_column'] : '';

    $destination = $_GET['q'];

    $actions = array(
      'edit' => l('edit',"admin/config/content/mci/{$importer->id}", array('query' => array('destination' => $destination))),
      'mapping' => l('field mapping',"admin/config/content/mci/{$importer->id}/mapping", array('query' => array('destination' => $destination))),
      'toggle' => l(($importer->status) ? 'disable' : 'enable', "admin/config/content/mci/{$importer->id}/toggle", array('query' => array('destination' => $destination))),
      'delete' => l('delete',"admin/config/content/mci/{$importer->id}/delete", array('query' => array('destination' => $destination))),
    );

    if (empty($importer->template)) {
      unset($actions['mapping']);
    }

    $row[] = mci_importer_get_status($importer,TRUE);

    $row[] = implode(' | ',$actions);

    $rows[] = $row;
  }

  $form['importers'] = array(
    '#theme' => 'table',
    '#type' => 'container',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No importers have been added. !link', array('!link' => l('Add an Importer','admin/config/content/mci/add'))),
  );

  return $form;
}

/**
 * Importer config form. Used in both mci_admin_add_form and mci_admin_edit_form.
 */
function mci_admin_importer_form(&$form,&$form_state, $importer = NULL) {
  $form_state['importer'] = $importer;

  $form['name'] = array(
    '#type' => 'textfield',
    '#default_value' => (!empty($importer)) ? $importer->name : '',
    '#title' => t('Importer Name'),
    '#description' => t('The name of the importer.'),
    '#required' => TRUE,
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#default_value' => (!empty($importer)) ? $importer->description : '',
    '#title' => t('Importer description'),
    '#description' => t('The description of the importer.'),
  );

  $max_file_size = min(array(file_upload_max_size(),4 * 1024 * 1024));

  $form['template_csv'] = array(
    '#type' => 'managed_file',
    '#title' => t('Template CSV'),
    '#description' => t('Upload a csv template with a single row with the column names for linking fields to their respective column.<br /><strong>File Format:</strong> csv<br /><strong>File Size:</strong> '.  format_size($max_file_size).'<br /><strong>Note:</strong> Uploading a new csv template will remove all mapping and the mapping will need to be re-built.'),
    '#upload_location' => 'public://mci-importer/csv/template/',
    '#upload_validators' => array(
      'file_validate_extensions' => array('csv'),
      'file_validate_size' => array($max_file_size),
    ),
  );

  $type_options = array();
  $content_types = node_type_get_types();

  foreach ($content_types as $type) {
    $type_options[$type->type] = $type->name;
  }

  $form['config'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    'content_type' => array(
      '#type' => 'select',
      '#default_value' => (!empty($importer)) ? $importer->config['content_type'] : '',
      '#options' => $type_options,
      '#title' => t('Content Type'),
      '#description' => t('The content type to create on import<br /><strong>Note:</strong> Changing the content type will remove all mapping and the mapping will need to be re-built.'),
      '#required' => TRUE,
    ),
  );

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => "Save Importer",
      '#submit' => array('mci_admin_importer_form_submit'),
    ),
  );
}

/**
 * Submit callback when saving new or existing content importers.
 */
function mci_admin_importer_form_submit($form,&$form_state) {
  $importer = $form_state['importer'];
  if (empty($importer)) {
    $importer = new stdClass();
    $importer->template = array();
    $importer->config = array('content_type' => '', 'mapping' => array());
  }

  $importer->name = $form_state['values']['name'];
  $importer->description = $form_state['values']['description'];
  $old_content_type = $importer->config['content_type'];
  $importer->config['content_type'] = $form_state['values']['config']['content_type'];

  if (!empty($form_state['values']['template_csv'])) {
    $importer->template = mci_process_template_csv(file_load($form_state['values']['template_csv']));
    $importer->config['mapping'] = array();
    unset($importer->config['primary_column']);

    drupal_set_message(t('Updated template for "!name". The mapping has been cleared and needs to be re-done.', array('!name' => $importer->name)));
  }

  if (!empty($old_content_type) && $old_content_type != $importer->config['content_type']) {
    $importer->config['mapping'] = array();
    unset($importer->config['primary_column']);

    drupal_set_message(t('Content type has changed for "!name". The mapping has been cleared and needs to be re-done.', array('!name' => $importer->name)));
  }

  mci_importer_save($importer);

  $destination = (!empty($_GET['destination'])) ? $_GET['destination'] : 'admin/config/content/mci';

  drupal_set_message(t('Successfully saved importer "!name"', array('!name' => $importer->name)));

  $form_state['redirect'] = $destination;
}

/**
 * Importer add form
 */
function mci_admin_importer_add_form($form,&$form_state) {
  drupal_set_title('Add Importer');
  mci_admin_importer_form($form,$form_state);

  return $form;
}

/**
 * Importer Edit Form
 */
function mci_admin_importer_edit_form($form,&$form_state, $importer) {

  if (empty($importer)) {
    return drupal_not_found();
  }

  mci_admin_importer_form($form,$form_state,$importer);

  return $form;
}

/**
 * Menu callback -- ask for confirmation of importer deletion
 */
function mci_admin_importer_delete_form($form, &$form_state, $importer) {
  $form_state['importer'] = $importer;
  $form['id'] = array('#type' => 'value', '#value' => $importer->id);
  return confirm_form($form,
    t('Are you sure you want to delete content importer "%title"?', array('%title' => $importer->name)),
    'admin/config/content/mci',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * The submit callback when deleting content importers
 */
function mci_admin_importer_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $importer = $form_state['importer'];
    mci_importer_delete($importer);
    watchdog('mci', 'Content Importer "%title" deleted.', array('%title' => $importer->name));
    drupal_set_message(t('Content Importer "%title" has been deleted.', array('%title' => $importer->name)));
  }

  $destination = (!empty($_GET['destination'])) ? $_GET['destination'] : 'admin/config/content/mci';

  $form_state['redirect'] = $destination;
}

/**
 * Importer Field Mapping form
 */
function mci_admin_importer_mapping_form($form,&$form_state,$importer) {

  if (!empty($importer) && !empty($form_state['values']) && !empty($form_state['values']['config']['mapping'])) {
    $importer->config['mapping'] = $form_state['values']['config']['mapping'];
  }

  $form_state['importer'] = $importer;

  drupal_set_title(t('Field Mapping for \'!name\'',array('!name' => $importer->name)));

  $content_type = $importer->config['content_type'];

  $fields = field_info_instances('node', $content_type);

  foreach (array_keys($fields) as $field_name) {
    $fields[$field_name] = $field_name;
  }

  $extra_fields = array();

  $extra_fields_display = field_info_extra_fields('node', $content_type,'display');

  foreach (array_keys($extra_fields_display) as $key) {
    if (!in_array($key,array('add_to_cart'))) {
      $extra_fields[$key] = $key;
    }
  }

  $extra_fields_form = field_info_extra_fields('node', $content_type,'form');

  foreach (array_keys($extra_fields_form) as $key) {
    $extra_fields[$key] = $key;
  }

  $field_list = $extra_fields + $fields;

  $form['config'] = array(
    '#tree' => TRUE,
    '#type' => 'container',
  );

  $primary_column_options = array();
  foreach ($importer->template as $column_name) {
    $primary_column_options[$column_name] = $column_name;
  }

  $form['config']['primary_column'] = array(
    '#title' => t('Primary Column'),
    '#description' => t('The primary column to import the content based on. For example you would want to use the title column or another column that will not have any duplicate entries'),
    '#type' => 'select',
    '#options' => $primary_column_options,
    '#default_value' => (!empty($importer->config['primary_column'])) ? $importer->config['primary_column'] : '',
    '#required' => TRUE,
  );

  $form['config']['mapping'] = array(
    '#type' => 'container',
  );

  foreach ($importer->template as $column_name) {
    $form['config']['mapping'][$column_name] = array(
      '#type' => 'fieldset',
      '#title' => $column_name,
      '#collapsed' => TRUE,
      '#collapsible' => TRUE,
    );

    $column_id = 'field-config-wrapper-'.  strtolower(preg_replace('/ /', '-', preg_replace('/_/', '-', $column_name)));

    $field_name = (!empty($importer->config['mapping'][$column_name]['field_name'])) ? $importer->config['mapping'][$column_name]['field_name'] : '';

    if (!in_array($field_name, $field_list)) {
      $field_name = '';
    }

    $form['config']['mapping'][$column_name]['field_name'] = array(
      '#type' => 'select',
      '#options' => array('' => 'Do Not Import', 'Fields' => $field_list),
      '#title' => t('Field Name'),
      '#default_value' => $field_name,
      '#ajax' => array(
        'callback' => 'mci_admin_importer_field_name_js',
        'wrapper' => $column_id,
      ),
      '#submit' => array('mci_admin_importer_field_name_submit'),
    );

    $form['config']['mapping'][$column_name]['field_config'] = array(
      '#type' => 'container',
      '#prefix' => '<div id="'.$column_id.'">',
      '#suffix' => '</div>',
    );

    if (!empty($field_name)) {
      $form['config']['mapping'][$column_name]['field_config']['#type'] = 'fieldset';
      $form['config']['mapping'][$column_name]['field_config']['#title'] = t('Field Settings');
      $form['config']['mapping'][$column_name]['field_config']['#collapsed'] = TRUE;
      $form['config']['mapping'][$column_name]['field_config']['#collapsible'] = TRUE;

      if (in_array($field_name, array('title','model','sku'))) {
        $form['config']['mapping'][$column_name]['field_config']['text_case'] = array(
          '#title' => t('Case Conversion'),
          '#description' => t('Select the type of case conversion you want to apply before importing.'),
          '#type' => 'select',
          '#options' => array('' => 'No Conversion', 'upercase' => 'UPPERCASE', 'lowecase' => 'lowercase', 'ucwords' => 'Capitalize Each Word'),
          '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['text_case'])) ? $importer->config['mapping'][$column_name]['field_config']['text_case'] : '',
        );
      } elseif (in_array($field_name, array('display_price','list_price','sell_price','cost'))) {
        $form['config']['mapping'][$column_name]['field_config']['currency_conversion'] = array(
          '#title' => t('Currency Conversion'),
          '#description' => t('Select the conversion for the currency.'),
          '#type' => 'select',
          '#options' => array('' => 'No Conversion', 'dollars_to_pennies' => 'Dollars to Pennies (amount * 100)', 'pennies_to_dollars' => 'Pennies to Dollars (amount / 100)'),
          '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['currency_conversion'])) ? $importer->config['mapping'][$column_name]['field_config']['currency_conversion'] : '',
        );
      } elseif (in_array($field_name, array('dimensions'))) {
        $form['config']['mapping'][$column_name]['field_config']['notes'] = array(
          '#markup' => "<p><strong>Notes:</strong><br />Dimensions need to be in the format of [Length]x[Width]x[Height].<br />Do not include any units of measurement.</p>",
        );
        $form['config']['mapping'][$column_name]['field_config']['units'] = array(
          '#title' => t('Units'),
          '#description' => t('Select the unit type for the dimensions.'),
          '#type' => 'select', 
          '#options' => array('in' => 'Inches', 'ft' => 'Feet', 'cm' => 'Centimeters', 'mm' => 'Millimeters'),
          '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['units'])) ? $importer->config['mapping'][$column_name]['field_config']['units'] : 'in',
        );
      } elseif (in_array($field_name, array('weight'))) {
        $form['config']['mapping'][$column_name]['field_config']['notes'] = array(
          '#markup' => "<p><strong>Notes:</strong><br />Do not include any units of measurement.</p>",
        );
        $form['config']['mapping'][$column_name]['field_config']['units'] = array(
          '#title' => t('Units'),
          '#description' => t('Select the unit type for the weight.'),
          '#type' => 'select', 
          '#options' => array('lb' => 'Pounds', 'kg' => 'Kilograms', 'oz' => 'Ounces', 'g' => 'Grams'),
          '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['units'])) ? $importer->config['mapping'][$column_name]['field_config']['units'] : 'in',
        );
      } elseif (!in_array($field_name, $extra_fields)) {
        mci_admin_importer_build_field_settings($form['config']['mapping'][$column_name]['field_config'],$field_name,$importer->config['content_type'], $importer, $column_name);
      }
    }
  }

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => "Save Field Mapping",
    ),
  );

  return $form;

}

function mci_admin_importer_mapping_form_submit($form,&$form_state) {
  $importer = $form_state['importer'];

  foreach ($form_state['values']['config']['mapping'] as $key => $map) {
    if (empty($map['field_name'])) {
      unset($form_state['values']['config']['mapping'][$key]);
    }
  }

  $importer->config['primary_column'] = $form_state['values']['config']['primary_column'];
  $importer->config['mapping'] = $form_state['values']['config']['mapping'];

  mci_importer_save($importer);

  drupal_set_message(t('Successfully saved field mapping for importer "!name"', array('!name' => $importer->name)));

  $destination = (!empty($_GET['destination'])) ? $_GET['destination'] : 'admin/config/content/mci';

  $form_state['redirect'] = $destination;
}

function mci_admin_importer_build_field_settings(&$container, $field_name, $content_type, $importer, $column_name) {
  $field_info = field_info_field($field_name);

  $config_form = array();

  if ($field_info['cardinality'] != 1) {
    $config_form['seperator'] = array(
      '#title' => t('Seperator/Delimiter'),
      '#description' => t('The seperator/delimiter used to seperate the field for multiple entries.'),
      '#type' => 'textfield',
      '#size' => 10,
      '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['seperator'])) ? $importer->config['mapping'][$column_name]['field_config']['seperator'] : ',',
    );
  }

  switch ($field_info['type']) {
    case 'taxonomy_term_reference':
      $config_form['hierarchical_seperator'] = array(
        '#title' => t('Hierarchical Seperator/Delimiter'),
        '#description' => t('The hierarchical seperator/delimiter used to seperate the entry into a hierarchical structure.'),
        '#type' => 'textfield',
        '#size' => 10,
        '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['hierarchical_seperator'])) ? $importer->config['mapping'][$column_name]['field_config']['hierarchical_seperator'] : ':',
      );
      $vocabulary_options = taxonomy_get_vocabularies();
      foreach (array_keys($vocabulary_options) as $key) {
        $vocabulary_options[$key] = $vocabulary_options[$key]->name;
      }

      $parent_column_options = array('' => 'None');
      foreach ($importer->template as $parent_column_name) {
        if ($parent_column_name == $column_name) {
          break;
        }
        $parent_column_options[$parent_column_name] = $parent_column_name;
      }

      $config_form['parent_column'] = array(
        '#title' => t('Parent Column'),
        '#description' => t('The column parent in the import csv.'),
        '#type' => 'select',
        '#options' => $parent_column_options,
        '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['parent_column'])) ? $importer->config['mapping'][$column_name]['field_config']['parent_column'] : '',
      );

      $config_form['vocabulary'] = array(
        '#title' => t('Vocabulary'),
        '#description' => t('The vocabulary in which to store the terms.'),
        '#type' => 'select',
        '#options' => $vocabulary_options,
        '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['vocabulary'])) ? $importer->config['mapping'][$column_name]['field_config']['vocabulary'] : reset(array_keys($vocabulary_options)),
      );
      if ($field_info['cardinality'] == -1) {
        $config_form['store_hierarchy'] = array(
          '#title' => t('Store Hierarchy'),
          '#description' => t('Store hierarchy terms as well. If checked, importing a term with a a hierarchy will result in each term to be stored into the field.'),
          '#type' => 'checkbox',
          '#return_value' => 1,
          '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['store_hierarchy'])) ? $importer->config['mapping'][$column_name]['field_config']['store_hierarchy'] : 0,
        );
      }
      break;
    case 'text':
    case 'text_long':
    case 'text_with_summary':
      $config_form['text_case'] = array(
        '#title' => t('Case Conversion'),
        '#description' => t('Select the type of case conversion you want to apply before importing.'),
        '#type' => 'select',
        '#options' => array('' => 'No Conversion', 'upercase' => 'UPPERCASE', 'lowecase' => 'lowercase', 'ucwords' => 'Capitalize Each Word'),
        '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['text_case'])) ? $importer->config['mapping'][$column_name]['field_config']['text_case'] : '',
      );
      break;
    case 'commerce_price':
      if (module_exists('commerce_price')) {
        $currency_codes = commerce_currency_code_options_list();
        $config_form['currency_code'] = array(
          '#title' => t('Currency Code'),
          '#description' => t('Select the currency Code.'),
          '#type' => 'select',
          '#options' => $currency_codes,
          '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['currency_code'])) ? $importer->config['mapping'][$column_name]['field_config']['currency_code'] : '',
        );
      }
      $config_form['currency_conversion'] = array(
        '#title' => t('Currency Conversion'),
        '#description' => t('Select the conversion for the currency.'),
        '#type' => 'select',
        '#options' => array('' => 'No Conversion', 'dollars_to_pennies' => 'Dollars to Pennies (amount * 100)', 'pennies_to_dollars' => 'Pennies to Dollars (amount / 100)'),
        '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['currency_conversion'])) ? $importer->config['mapping'][$column_name]['field_config']['currency_conversion'] : '',
      );
      break;
    case 'list_boolean':
      $config_form['true_value'] = array(
        '#title' => t('True Value'),
        '#description' => t('The value for when true'),
        '#type' => 'textfield',
        '#size' => 10,
        '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['true_value'])) ? $importer->config['mapping'][$column_name]['field_config']['true_value'] : 'yes',
      );
      $config_form['false_value'] = array(
        '#title' => t('False Value'),
        '#description' => t('The value for when false'),
        '#type' => 'textfield',
        '#size' => 10,
        '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['false_value'])) ? $importer->config['mapping'][$column_name]['field_config']['false_value'] : 'no',
      );
      $config_form['no_match'] = array(
        '#title' => t('No Match'),
        '#description' => t('If value does not match either the true or false values.'),
        '#type' => 'select',
        '#options' => array('true' => 'Set to True', 'false' => 'Set to False'),
        '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['no_match'])) ? $importer->config['mapping'][$column_name]['field_config']['no_match'] : 'false',
      );
      break;
    case 'image':
      $dir = drupal_realpath('public://');
      $dirs = mci_get_sub_folder_names($dir);

      $config_form['image_dir'] = array(
        '#title' => t('Image Directory'),
        '#description' => t('The image directory where the images are located. Relative to the public:// directory which is currenty !public', array('!public' => variable_get('file_public_path', 'sites/default/files'))),
        '#type' => 'select',
        '#options' => $dirs,
        '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['image_dir'])) ? $importer->config['mapping'][$column_name]['field_config']['image_dir'] : '',
      );

      $config_form['exist_method'] = array(
        '#title' => t('Existing Images Method'),
        '#description' => t('The method when updating content that already has images assigned in the field.'),
        '#type' => 'select',
        '#options' => array('append' => 'Append', 'replace' => 'Replace'),
        '#default_value' => (!empty($importer->config['mapping'][$column_name]['field_config']['exist_method'])) ? $importer->config['mapping'][$column_name]['field_config']['exist_method'] : 'append',
      );

      break;
  }

  if (empty($config_form)) {
    $config_form['no_config'] = array(
      '#markup' => '<p>No configuration available for this field</p>',
    );
  }

  $container += $config_form;
}

function mci_admin_importer_field_name_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

function mci_admin_importer_field_name_js($form, &$form_state) {
  $return = !empty($form['config']['mapping'][$form_state['triggering_element']['#array_parents'][2]]['field_config']) ? $form['config']['mapping'][$form_state['triggering_element']['#array_parents'][2]]['field_config'] : '';
  return $return;
}

/**
 * Menu callback -- ask for confirmation of importer enable/disable
 */
function mci_admin_importer_toggle_form($form, &$form_state, $importer) {
  $form_state['importer'] = $importer;
  $form['id'] = array('#type' => 'value', '#value' => $importer->id);
  return confirm_form($form,
    t('Are you sure you want to !status content importer "%title"?', array('!status' => ($importer->status) ? 'disable' : 'enable', '%title' => $importer->name)),
    'admin/config/content/mci',
    t('This will !status the importer.', array('!status' => ($importer->status) ? 'disable' : 'enable')),
    t('!status', array('!status' => ($importer->status) ? 'Disable' : 'Enable')),
    t('Cancel')
  );
}

/**
 * The submit callback when enable/disable content importers
 */
function mci_admin_importer_toggle_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $importer = $form_state['importer'];
    mci_importer_toggle($importer);
    watchdog('mci', 'Content Importer "%title" !status.', array('%title' => $importer->name, '!status' => (!$importer->status) ? 'disabled' : 'enabled'));
    drupal_set_message(t('Content Importer "%title" has been !status.', array('%title' => $importer->name, '!status' => (!$importer->status) ? 'disabled' : 'enabled')));
  }

  $destination = (!empty($_GET['destination'])) ? $_GET['destination'] : 'admin/config/content/mci';

  $form_state['redirect'] = $destination;
}