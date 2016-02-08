<?php

function onlinepdf_node_validate($node, $form, &$form_state)
{

  //system load higher then 10, set form error
    $load = sys_getloadavg();
    if ($load[0] > variable_get('onlinepdf_max_load', 16) || $load[1] > variable_get('onlinepdf_max_5min_load', 8)) {
      form_set_error("submit", t('The pdfjpg service is currently overloaded! Please wait a few minutes before submitting again.', array()));
      watchdog('load', 'The systemload is %load! Conversion process on hold.', array('%load' => $load[0]), WATCHDOG_WARNING);
    };
    $form_state['submit_handlers'][] = '_custom_request_node_disable_msg';
}

//TODO finish the created message removal
function _custom_request_node_disable_msg($form, &$form_state) {
  //...
  // clear status messages
  //drupal_get_messages('status');
}

//Return pdf info as array. Returns false and sets form error with reading errors
//Input vars: $file = file object, $password = PDF password
function _onlinepdf_pdfinfo($file, $password)
{
  if (strlen($password) > 0) {
      $password = '-upw '.escapeshellarg($password);
  }
  $fullpath = escapeshellarg(drupal_realpath($file->uri));
  exec ("pdfinfo $password $fullpath", $out, $return_var);
  switch ($return_var) {
    case 1:
      form_set_error("field_file", t('Error reading PDF information for %filename', array('%filename' => $file->filename)));
      watchdog('onlinepdf', 'Form error reading PDF information for %filename', array('%filename' => $file->filename), WATCHDOG_NOTICE);
      break;
    case 3:
      form_set_error("field_file", t('PDF permission error for %filename', array('%filename' => $file->filename)));
      watchdog('onlinepdf', 'Form PDF permission error for %filename', array('%filename' => $file->filename), WATCHDOG_NOTICE);
      break;
  }
  if (count($out) == 0)
  {
      form_set_error("field_file", t('Error reading PDF page size for %filename', array('%filename' => $file->filename)));
      watchdog('onlinepdf', 'Form error reading PDF page size for %filename', array('%filename' => $file->filename), WATCHDOG_NOTICE);
      $out = FALSE;
  }
  return $out;
}


//returns expected jpg resolution output of conversion.
function _onlinepdf_jpgsize($pdfinfo, $dpi) {
  $pagesize = explode('x',str_replace(' ', '',strstr(substr(implode(preg_grep("/^Page size.*/", $pdfinfo)),10), 'pts', true)));
  if (is_numeric($pagesize[0]) && is_numeric($pagesize[1]) && $pagesize[0] > 0 && $pagesize[1] > 0)
  {
  $pagesize[0] = round(($pagesize[0]/72) * $dpi);
  $pagesize[1] = round(($pagesize[1]/72) * $dpi);
  return $pagesize;
  }
  else {
    return false;
  }
}

//returns physical PDF dimentions
function _onlinepdf_pdfpages($pdfinfo) {
  if (!is_array($pdfinfo))
  {
    return false;
  }
  $pagesize = str_replace(' ', '',substr(implode(preg_grep("/^Pages.*/", $pdfinfo)),6));
  if (is_numeric($pagesize) && $pagesize > 0)
  {
    return $pagesize;
  }
  else
  {
    return false;
  }
}
