<?php

$file = file_load($entity->field_pdf[LANGUAGE_NONE][0]['fid']);
$pdfpath = drupal_realpath($file->uri);
$filename = basename($file->uri, ".pdf");
$dest = "sites/default/files/html/$filename/".$filename.".html";
$destdir = drupal_realpath('public://')."/html/$filename/";
$to_attach = drupal_realpath('public://')."/html/$filename/".$filename.".html";
$pdf2html_dir =  drupal_realpath('public://')."/pdf2htmlEX";
$cmd = 'pdf2htmlEX --fit-width 980 --embed-image 0 --dest-dir "'.$destdir.'" --data-dir "'.$pdf2html_dir.'" "'.$pdfpath.'" '.$filename.'.html';
watchdog("onlinepdf", $cmd);
drupal_set_message("To share, copy paste the address in the addressbar or use the following buttons:");
$operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('Creating view of PDF %filename', array('%filename' => $filename)), $params, null));