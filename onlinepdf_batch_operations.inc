<?php

//hook_batch_finished
//a little after process and filefield creation or overwriting
//delete all temporary files
function onlinepdf_batch_finished($success, $results, $operations) {
     if ($success) {
        // Here we do something meaningful with the results.
        if (!(isset($results['to_attach']) && count($results['to_attach']))) {
            drupal_set_message(t('Something went wrong! No files generated. Site administrator is noticed.', array('%filename' => $filename)), 'error');
            watchdog('onlinepdf', 'No files produced from processing document', array(), WATCHDOG_NOTICE);
            return;
        }
        $field_name = "field_download";
        $entity_id = $results['params']['entity_id'];
        $entity_type = $results['params']['entity_type'];
          if (isset($entity_id) && is_numeric($entity_id)) {
        // Don't use the entity as given, load it again as things may have happened
        // to it since the batch job began.
        $entity = entity_load_unchanged($entity_type, $entity_id);
        if (is_object($entity)) {
          $field_lang = field_language($entity_type, $entity, $field_name);
          
          // This removes the existing images by emptying the list.
          // The (re?) attaches the generated ones.
          $node->field_download[$field_lang] = array();
          ksort($results['to_attach'], SORT_NUMERIC);
          $node = node_load($entity_id);
          foreach ($results['to_attach'] as $file) {
            $file2 = str_replace("*", "", $file);
            exec("convert $file*.ppm $file2");
            if (strpos($file, "*"))
            {
                
                $files = glob($file);
                natsort($files);
                foreach ($files as $filename) {
                    
                    $file = create_fileobject($filename);
                    if ($file)
                    {
                        $node->field_download[$field_lang][] = (array)$file;
                    }
                    else {  //no files produced
                        drupal_set_message(t('Something went wrong! No files generated. Site administrator is noticed.', array('%filename' => $filename)), 'error');
                        watchdog("onlinepdf", 'No output at nid %nid', array('%nid' => $entity_id), WATCHDOG_ERROR);
                    }
                }
            }
            else {
                
                $file = create_fileobject($file);
                if ($file)
                {
                    $node->field_download[$field_lang][] = (array)$file;
                }
                else {  //no files produced
                    drupal_set_message(t('Something went wrong! No files generated. Site administrator is noticed.', array('%filename' => $filename)), 'error');
                    watchdog("onlinepdf", 'No output at nid %nid', array('%nid' => $entity_id), WATCHDOG_ERROR);
                }
                
            }
            
            
          }
          field_attach_update('node', $node);
          //watchdog('onlinepdf', 'Attached converted files to content.', array(), WATCHDOG_INFO);
          
          
          //delete everything
          if (isset($results['to_delete']))
          {
            foreach ($results['to_delete'] as $garbage)
            {
              if (strpos($garbage, "*"))
              {
                  foreach (glob($garbage) as $file)
                  {
                      unlink ($file);
                  }
              }
              else {
                  unlink($garbage);
              }
            }
          }
          foreach ($results['to_attach'] as $garbage)
          {
            if (strpos($garbage, "*"))
            {
                foreach (glob($garbage) as $file)
                {
                    unlink ($file);
                }
            }
            else {
                unlink($garbage);
            }
          }
        }
        else {
          watchdog('onlinepdf', 'Invalid content object. Cannot attach generated images to it.', array(), WATCHDOG_ERROR);
        }
      }
      else {
        watchdog('onlinepdf', 'Invalid content id given to attach generated images to.', array(), WATCHDOG_ERROR);
      }
     //add batch runtime to user's session
     if (module_exists('aup'))
     {
          aup_session_addtime(time() - $results['params']['starttime']);
     } 
     $duration = (time() - $results['params']['starttime']);
     //drupal_set_message(t('Succes! Your PDF is converted in %seconds.', array('%seconds' => $duration)), 'notice');
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}

function onlinepdf_batch_exec_process($options1, $message, $params, $to_attach, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = 1;
  }
  //exec command
  if ($options1) exec($options1);
  
    
    // Update our progress information.
    $context['sandbox']['progress']++;
    
    $context['message'] = $message;
  
  if ($params && !isset($context['results']['params']))
  {
    $context['results']['params'] = $params;
  }
  
  if ($to_attach)
  {
    $context['results']['to_attach'][] = $to_attach;
  }
  
  // Inform the batch engine that we are finished,
  // and provide an estimation of the completion level we reached.
    $context['finished'] = 1;
}

//merge OCR output and save as word
function onlinepdf_batch_ocr_merge($filename, $message, $params, $to_attach, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = 1;
  }
  //exec command
  if ($filename) {
    foreach (glob(drupal_realpath('public://tmp/'.$filename."*.txt")) as $filename) {
        $text_files[] = file_get_contents($filename);
    }
    file_put_contents(drupal_realpath('public://tmp/'.$filename."*.txt", implode("\n", $text_files)));
        $filepath2 = $filename.".txt";
  }
    
    // Update our progress information.
    $context['sandbox']['progress']++;
    
    $context['message'] = $message;
  
  if ($params && !isset($context['results']['params']))
  {
    $context['results']['params'] = $params;
  }
  
  if ($to_attach)
  {
    $context['results']['to_attach'][] = $to_attach;
  }
  
  // Inform the batch engine that we are finished,
  // and provide an estimation of the completion level we reached.
    $context['finished'] = 1;
}

//execute ssh commands at remote server
function onlinepdf_batch_ssh_exec_process($remote_cmd, $conn, $message, $params, $to_attach, &$context) {
  
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = 1;
  }
 
 $connection = ssh2_connect(variable_get('onlinepdf_winserver_ip'));
                        
    if (!$connection) watchdog('onlinepdf', 'SSH Connection failed',array(),WATCHDOG_ERROR);

    if (ssh2_auth_password($connection, variable_get('onlinepdf_winserver_user'), variable_get('onlinepdf_winserver_password'))) {

    } else {
      watchdog('onlinepdf', 'SSH Win server Authentication Failed...',array(),WATCHDOG_ERROR);
    }
   
  //exec command
    $stream = ssh2_exec($connection, $remote_cmd);
    stream_set_blocking($stream, true);
    $stream_out = ssh2_fetch_stream($stream, SSH2_STREAM_STDIO);
    stream_get_contents($stream_out);
    ssh2_exec($connection, 'exit');
    unset($connection);
    // Update our progress information.
    $context['sandbox']['progress']++;
    
    $context['message'] = $message;
  
  if ($params && !isset($context['results']['params']))
  {
    $context['results']['params'] = $params;
  }
  
  if ($to_attach)
  {
    $context['results']['to_attach'][] = $to_attach;
  }
  
  // Inform the batch engine that we are finished,
  // and provide an estimation of the completion level we reached.
    $context['finished'] = 1;
}

function onlinepdf_batch_pdfconv_process($options1, $filename, $temppdf, $tempjpg, $params, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = $context['results']['pdf_pagecount'];
    
  }
  
  //exec command
  $context['sandbox']['progress']++;
  if ($options1)
  {
    // Update our progress information.
    
    $options1 = str_replace("%p", $context['sandbox']['progress'], $options1); //starting at 1
    $options1 = str_replace("%n", $context['sandbox']['progress'] - 1, $options1); //starting at zero
    
    $context['message'] = t('Converting page %page / %max of %file', array('%page' => $context['sandbox']['progress'], '%max' => $context['results']['pdf_pagecount'], '%file' => $filename));
    
    exec($options1);
    
  }
  if ($context['sandbox']['progress'] == 1)
  {
    if ($tempjpg && strpos($tempjpg, '%p')) $context['results']['to_delete'][] = str_replace("%p", "*", $tempjpg);
    if ($tempjpg && strpos($tempjpg, '%n')) $context['results']['to_delete'][] = str_replace("%n", "*", $tempjpg);
    if ($temppdf) $context['results']['to_delete'][] = $temppdf;
    
    if (strpos($tempjpg, '%p')) $context['results']['to_attach'][] = str_replace("%p", "*", $tempjpg);
    if (strpos($tempjpg, '%n')) $context['results']['to_attach'][] = str_replace("%n", "*", $tempjpg);
  }
  if ($params && !isset($context['results']['params']))
  {
    $context['results']['params'] = $params;
  }
  // Inform the batch engine that we are finished,
  // and provide an estimation of the completion level we reached.
  if ($context['sandbox']['progress'] < $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
  if ($context['results']['pdf_pagecount'] < 1) $context['finished'] = true;
}




//count pdf pages in batch api context
function onlinepdf_batch_pdfpagecount_process($temppdf, $message, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = 1;
  }
  $filename = basename($temppdf);
  
  //exec command
  if ($temppdf) {
    exec('/usr/bin/pdfinfo '.escapeshellarg($temppdf).' | awk \'/Pages/ {print $2}\'', $pagecount);
    // Store some result for post-processing in the finished callback.
    isset($pagecount[0]) ? $context['results']['pdf_pagecount'] = $pagecount[0] : $context['results']['pdf_pagecount'] = 0;
  }
    //throw error if something went wrong
    if (is_nan($context['results']['pdf_pagecount']) || !isset($pagecount[0]))
    {
        $context['results']['pdf_pagecount'] = 0;
        drupal_set_message(t('Could not determine numer of pages for %name', array('%name' => $filename)), 'error', FALSE);
        watchdog("onlinepdf", 'Could not determine numer of pages for %name', array('%name' => $filename), WATCHDOG_ERROR);
    }
    
    // Update our progress information.
    $context['sandbox']['progress']++;
    
    $context['message'] = $message;
  
  // Inform the batch engine that we are finished,
  // and provide an estimation of the completion level we reached.
  
    $context['finished'] = 1;
  
}

//receive file from remote worker.
function onlinepdf_batch_sshget_process($docname, $outputpath,  $message, $params, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = 1;
  }
  //Todo: change sleep to some more intelligent check of conversion completion.
  sleep(10);
  $connection = ssh2_connect(variable_get('onlinepdf_winserver_ip'));
                        
                        if (!$connection) watchdog('onlinepdf', 'SSH Connection failed',array(),WATCHDOG_ERROR);
                        
                        if (!ssh2_auth_password($connection, variable_get('onlinepdf_winserver_user'), variable_get('onlinepdf_winserver_password'))) {
                          watchdog('onlinepdf', 'SSH Win server Authentication Failed...',array(),WATCHDOG_ERROR);
                        }
                        if (!$resSftp = ssh2_sftp($connection)) {
                            watchdog('onlinepdf', 'Unable to create SFTP connection.', array(), WATCHDOG_ERROR);
                        }
                        if (!$stream = ssh2_sftp($connection)) watchdog('onlinepdf','Unable to create a sftp stream.',array(),WATCHDOG_ERROR);
	
                        //Copy remote
                        if (!$remote = fopen("ssh2.sftp://$resSftp/$docname", 'r'))
                        {
                            watchdog('onlinepdf', "Failed to open remote file: $docname\n",array(),WATCHDOG_ERROR);
                            continue;
                        }
                        //write to local file
                        if (!$local = fopen($outputpath, 'w'))
                        {
                            watchdog('onlinepdf', "Failed to create local file: $docname\n",array(),WATCHDOG_ERROR);
                            continue;
                        }
                        $read = 0;
                        $filesize = filesize("ssh2.sftp://$resSftp/$docname");
                        while ( ($read < $filesize) && ($buffer = fread($remote, $filesize - $read)) )
                        {
                            $read += strlen($buffer);
                            if (fwrite($local, $buffer) === FALSE)
                            {
                                watchdog('onlinepdf', "Failed to write to local file: $docname\n",array(),WATCHDOG_ERROR);
                                break;
                            }
                        }
                        fclose($local);
                        fclose($remote);
                        
                        //delete remote files
                        //$stream = ssh2_exec($connection, 'del c:\\pdftoexcel\\'.$bestandsnaam.'*');
                        //stream_set_blocking($stream, true);
                        //$stream_out = ssh2_fetch_stream($stream, SSH2_STREAM_STDIO);
                        //stream_get_contents($stream_out);
                        
                        ssh2_exec($connection, 'exit');
                        unset($connection);
  
    
    // Update our progress information.
    $context['sandbox']['progress']++;
    
    $context['message'] = $message;
  
  // Inform the batch engine that we are finished,
  // and provide an estimation of the completion level we reached.
  
    $context['finished'] = 1;
  
}