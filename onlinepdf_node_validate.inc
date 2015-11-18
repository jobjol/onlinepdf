<?php

function _onlinepdf_node_validate($form, &$form_state)
{
    //check field_file for file objects
    foreach ($form_state['values']['field_file'][LANGUAGE_NONE] as $file)
    {
      if ($file['fid'] && $file['fid'] > 0)
      {
        $file = file_load($file['fid']);
        //if mime type is PDF, get PDF info
        if ($file && $file->filemime == 'application/pdf')
        {
          $fid = $file->fid;
          if (dpi($form_state['values']['field_dpi_setting'][LANGUAGE_NONE][0]['value'], $form_state['values']['field_dpi'][LANGUAGE_NONE]['item']['value']) < 10) {
            form_set_error("field_dpi_setting", t('The dpi setting is invalid. Please choose a valid DPI value'));
          }
          $pdfinfo = _onlinepdf_pdfinfo($file,$form_state['values']['files'][$fid]['field_unlock_password'][LANGUAGE_NONE][0]['value']);
          if ($pdfinfo && $form_state['values']['field_action'][LANGUAGE_NONE][0]['value'] == 'convert') {
            $jpgsize = _onlinepdf_jpgsize($pdfinfo, dpi($form_state['values']['field_dpi_setting'][LANGUAGE_NONE][0]['value'], $form_state['values']['field_dpi'][LANGUAGE_NONE]['item']['value']));
            if ($jpgsize) {
              $megapixel = round(($jpgsize[0] * $jpgsize[1]) / 1000000);
              if ($megapixel > variable_get('onlinepdf_max_jpg_resolution', 10)) {
                  if ($form_state['values']['field_dpi_setting'][LANGUAGE_NONE][0]['value'] != '-1') {
                    form_set_error("field_dpi_setting", t('Please lower the value of DPI preset. The chosen value will generate JPG image(s) of %megapixelMP, greater than the limit of %limit megapixels for %filename', array('%megapixel' => $megapixel,'%limit' => variable_get('onlinepdf_max_jpg_resolution', 10), '%filename' => $file->filename)));
                  }
                  else {
                    form_set_error("field_dpi", t('Please lower the DPI value. The current setting will generate JPG image(s) of %megapixelMP, greater than our limit of %limit megapixels for %filename', array('%megapixel' => $megapixel,'%limit' => variable_get('onlinepdf_max_jpg_resolution', 10), '%filename' => $file->filename)));
                  }
              }
            }
            else { //could not determine jpg output dimentions
                  form_set_error("field_file", t('Could not determine expected output resolution for %filename', array('%filename' => $file->filename)));
                  watchdog('onlinepdf', 'Could not determine expected jpg output dimentions for %filename', array('%filename' => $file->filename), WATCHDOG_ERROR);
            }
          }
          $page_selection = $form_state['values']['files'][$file->fid]['field_select_following_pages']['und'][0]['value'];
          if (strlen($page_selection) > 0) {
            if (!preg_match("^[0-9](,[0-9])*$^", $page_selection)) {
              form_set_error("field_file", t('The page selection for %filename contains invalid characters', array('%filename' => $file->filename)));
            }
            else {
              $pages = array();
              preg_match_all('/([\d]+)/', $page_selection, $pages);
              $pdfpagecount = _onlinepdf_pdfpages($pdfinfo);
              //throw error, no page count found in pdfinfo
              if (!$pdfpagecount) {
                form_set_error("field_file", t('The amount of pages for %filename could not be determined.', array('%filename' => $file->filename)));
                watchdog('onlinepdf', 'Could not determine number of pages %filename with pdfinfo', array('%filename' => $file->filename), WATCHDOG_ERROR);
              }
              else
              {
                //check if the page selection contains existing page numbers.
                foreach ($pages[0] as $page) {
                  if ($page > $pdfpagecount) {
                    form_set_error("field_file", t('The page selection for %filename selects page %page, but it has only %pagecount pages.', array('%pagecount' => $pdfpagecount,'%page' => $page, '%filename' => $file->filename)));
                    watchdog('onlinepdf', 'The page selection for %filename selects page %page, but it has only %pagecount pages.', array('%pagecount' => $pdfpagecount,'%page' => $page, '%filename' => $file->filename), WATCHDOG_WARNING);
                  }
                }
              }
            }
          }
        }
        else {
          //no pdf mimetype or valid file object
          form_set_error("field_file", t('%filename is not recognized as a valid PDF file.', array('%filename' => $file->filename)));
        }
      }
    }
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
