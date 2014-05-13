<?php

function mci_importer_page() {
  $item = menu_get_item();
  $content = system_admin_menu_block($item);
  if (count($content) == 1) {
    $item = array_shift($content);
    drupal_goto($item['href']);
  }
  return theme('mci_importer_list', array('content' => $content));
}

function mci_importer_form($form,&$form_state, $importer_id) {
  $importer = mci_importer_load($importer_id);

  if (empty($importer)) {
    return drupal_not_found();
  }
  // drupal_set_message("<pre>".print_r($importer,true)."</pre>");
  // 
  $form_state['importer'] = $importer;

  $template_rows = array();
  $row = array();
  foreach ($importer->template as $column_name) {
    if ($column_name == $importer->config['primary_column']) {
      $row[] = "<strong>".$column_name."</strong> (Primary)";
    } else {
      $row[] = $column_name;
    }
  }

  $template_rows[] = $row;


  $form['template_wrapper'] = array(
    '#type' => 'fieldset',
    '#title' => 'CSV Import Column Template',
  );

  $form['template_wrapper']['template'] = array(
    '#header' => array(),
    '#rows' => $template_rows,
    '#empty' => t('No Template'),
    '#type' => 'container',
    '#theme' => 'table',
  );

  $max_file_size = min(array(file_upload_max_size(),4 * 1024 * 1024));

  $form['import_file'] = array(
    '#type' => 'managed_file',
    '#title' => t('Import File'),
    '#description' => t('The file that contains the import data.<br /><strong>File Format:</strong> csv<br /><strong>File Size:</strong> '.  format_size($max_file_size)),
    '#upload_location' => 'public://mci-importer/csv/template/',
    '#upload_validators' => array(
      'file_validate_extensions' => array('csv'),
      'file_validate_size' => array($max_file_size),
    ),
  );

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Import Content'),
    ),
  );

  return $form;
}

function mci_importer_form_submit($form, &$form_state) {
  $importer = $form_state['importer'];
  $columns = array();
  if (!empty($form_state['values']['import_file'])) {
    $file = file_load($form_state['values']['import_file']);
    $columns = mci_process_template_csv($file);
    if ($importer->template == $columns) {
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);

      $importer_batch = new stdClass();
      $importer_batch->importer_id = $importer->id;
      $importer_batch->status = 0;
      $importer_batch->start_time = REQUEST_TIME;
      $importer_batch->end_time = '';
      $importer_batch->current = 0;
      $importer_batch->total = 0;
      $importer_batch->fid = $form_state['values']['import_file'];

      mci_importer_batch_save($importer_batch);

      //drupal_set_message("<pre>".print_r($importer_batch,true)."</pre>");

      drupal_goto('admin/mci/batch/'.$importer_batch->id);
    } else {
      drupal_set_message("The csv import file you uploaded does not contain the correct template for the columns. Please correct the csv to have the column template like the one shown on this page.","error");
    }
  } else {
    drupal_set_message("The csv import file you uploaded is invalid, please upload a valid csv file.","error");
  }
}

function mci_importer_log_form($form,&$form_state,$importer_id) {
  $importer = mci_importer_load($importer_id);
  if (empty($importer)) {
    return drupal_not_found();
  }

  drupal_set_title(t('!name Import Logs',array('!name' => $importer->name)));

  $importer_batches = mci_importer_batch_load_multiple(array(),array('importer_id' => $importer_id));

  $header = array(
    t('ID'),
    t('Current Row'),
    t('Total Rows'),
    t('Status'),
    t('Start Time'),
    t('End Time'),
  );

  $rows = array();

  foreach ($importer_batches as $batch) {
    $row = array();
    $row[] = $batch->id;
    $row[] = $batch->current;
    $row[] = $batch->total;
    switch ($batch->status) {
      case 0:
        $row[] = t('Queued');
        break;
      case 1:
        $row[] = t('Processing');
        break;
      case 2:
        $row[] = t('Completed');
        break;
    }

    $row[] = (!empty($batch->start_time)) ? date('m-d-Y h:i a',$batch->start_time) : '';
    $row[] = (!empty($batch->end_time)) ? date('m-d-Y h:i a',$batch->end_time) : '';

    $rows[] = $row;
  }

  $form['logs'] = array(
    '#type' => 'container',
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No import logs found for this importer.'),
  );

  return $form;
}

function mci_importer_batch_page($importer_batch) {
  $batch = array(
    'title' => t('Importing Content'),
    'operations' => array(
      array('mci_importer_batch_process', array($importer_batch)),
    ),
    'init_message' => t('Content import starting.'),
    'progress_message' => t('Importing content...'),
    'finished' => 'mci_importer_batch_finished',
    'file' => drupal_get_path('module', 'mci') . '/inc/mci.batch.inc',
  );

  batch_set($batch);

  batch_process('admin/mci/batch/'.$importer_batch->id.'/complete');
}

function mci_importer_batch_complete_page($importer_batch) {
  $file = file_load($importer_batch->fid);
  $file->status = 0;
  file_save($file);

  $importer_batch->status = 2;
  $importer_batch->end_time = REQUEST_TIME;
  mci_importer_batch_save($importer_batch);

  drupal_goto('admin/mci/'.$importer_batch->importer_id);
}