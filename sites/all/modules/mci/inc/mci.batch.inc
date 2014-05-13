<?php

function mci_importer_batch_process($importer_batch, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $file = file_load($importer_batch->fid);
    $file_rows = array();
    $file_header = array();
    $row = 0;
    if (($handle = fopen(drupal_realpath($file->uri), "r")) !== FALSE) {
      while (($data = fgetcsv($handle, 1000, ",")) !== FALSE) {
        if ($row == 0) {
          $file_header = $data;
        } else {
          if (count($data) == count($file_header)) {
            $file_rows[] = $data;
          }
        }
        $row++;
      }
      fclose($handle);
    }
    
    $linecount = count($file_rows);

    $context['sandbox']['file_header'] = $file_header;
    $context['sandbox']['file_rows'] = $file_rows;

    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = max($linecount,0);
  }

  $limit = 5;

  $file_rows = $context['sandbox']['file_rows'];

  $rows = array_splice($file_rows, $context['sandbox']['progress'], $limit);

  foreach ($rows as $row) {
    $context['sandbox']['progress']++;

    $result = mci_import_content($row,$importer_batch->importer);

    if ($result == 0) {
      $context['results']['invalid'][] = "Processed Row: ".implode(', ',$row);
    } else {
      $context['results']['valid'][] = "Processed Row: ".implode(', ',$row);
    }

    $context['message'] = "Processing Row: ".implode(', ',$row);
  }

  $importer_batch->current = $context['sandbox']['progress'];
  $importer_batch->total = $context['sandbox']['max'];
  $importer_batch->status = 1;

  mci_importer_batch_save($importer_batch);

  // Inform the batch engine that we are not finished,
  // and provide an estimation of the completion level we reached.
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Batch 'finished' callback
 */
function mci_importer_batch_finished($success, $results, $operations) {
  // The 'success' parameter means no fatal PHP errors were detected. All
  // other error management should be handled using 'results'.
  if ($success) {
    $message = format_plural(count((!empty($results['valid'])) ? $results['valid'] : array()), '1 item imported.', '@count items imported.');
    drupal_set_message($message);
    if (!empty($results['invalid'])) {
      $invalid_message = format_plural(count((!empty($results['invalid'])) ? $results['invalid'] : array()), '1 invalid item.', '@count invalid items.');
      drupal_set_message($invalid_message,"error");
    }
  }
  else {
    $message = t('Finished with an error.');
    drupal_set_message($message, 'error');
  }
}