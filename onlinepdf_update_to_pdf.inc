<?php

foreach ($file as $item) {

  //convert image to PDF
  if (strpos($item->filemime, 'image/') === 0)
  {
    $tempimage = file_unmanaged_copy($item->uri, '/tmp');
    $imagesize = getimagesize($tempimage);
    $filename = basename($tempimage, '.jpg');
    $dest[] = "/tmp/".$filename.".pdf";
    $temppdf = end($dest);

    $imgEdits = null;
    $crop = null;
    $resize = null;
    $page = null;

    if ($item->field_rotation[LANGUAGE_NONE][0]['value'] == "90" | $item->field_rotation[LANGUAGE_NONE][0]['value'] == "-90")
    {
      list($imagesize[0], $imagesize[1]) = array($imagesize[1], $imagesize[0]);
    }
    if ($item->field_rotation[LANGUAGE_NONE][0]['value'] != "0") {
      $imgEdits = "-rotate ".$item->field_rotation[LANGUAGE_NONE][0]['value'];
    }

    if ($item->field_flip_vertically[LANGUAGE_NONE][0]['value'] === true) {
      $imgEdits .= " -flip ";
    }
    if ($item->field_flip[LANGUAGE_NONE][0]['value'] === true) {
      $imgEdits .= " -flop";
    }
    if ($item->field_equalize[LANGUAGE_NONE][0]['value'] === true) {
      $imgEdits .= " -equalize";
    }
    if ($item->field_negate[LANGUAGE_NONE][0]['value'] === true) {
      $imgEdits .= " -negate";
    }

    $dimentions = _onlinepdf_imgsizepdf($entity, $item);

    if ($dimentions) {
      list ($imagesize[0], $imagesize[1],
        $cropx, $cropy,
        $pagesize[0], $pagesize[1],
        $boxsize[0], $boxsize[1]) = $dimentions;
      $offsetx = 0;
      $offsety = 0;
      $offsetx = round(($boxsize[0] - $imagesize[0]) /2);
      $offsety = round(($boxsize[1] - $imagesize[1]) /2);
      $size = "-size {$imagesize[0]}x{$imagesize[1]}";
      //image should be cropped
      if ($entity->field_scaling[LANGUAGE_NONE][0]['value'] == 5) {
        $resize = "-resize {$boxsize[0]}x{$boxsize[1]}^ -gravity center -crop {$boxsize[0]}x{$boxsize[1]}+0+0 +repage -page {$pagesize[0]}x{$pagesize[1]} -resize {$boxsize[0]}x{$boxsize[1]} -gravity center -extent {$pagesize[0]}x{$pagesize[1]}";
      }
      elseif ($entity->field_scaling[LANGUAGE_NONE][0]['value'] == 4) {
        $resize = "-resize x{$boxsize[1]} -gravity center -crop {$boxsize[0]}x{$boxsize[1]}+0+0 +repage -page {$pagesize[0]}x{$pagesize[1]} -resize {$boxsize[0]}x{$boxsize[1]} -gravity center -extent {$pagesize[0]}x{$pagesize[1]}";
      }
      elseif ($entity->field_scaling[LANGUAGE_NONE][0]['value'] == 3) {
        $resize = "-resize {$boxsize[0]} -gravity center -crop {$boxsize[0]}x{$boxsize[1]}+0+0 +repage -page {$pagesize[0]}x{$pagesize[1]} -resize {$boxsize[0]}x{$boxsize[1]} -gravity center -extent {$pagesize[0]}x{$pagesize[1]}";
      }
      elseif ($entity->field_scaling[LANGUAGE_NONE][0]['value'] == 1) {
        $resize = "-gravity center -crop {$boxsize[0]}x{$boxsize[1]}+0+0 +repage -page {$pagesize[0]}x{$pagesize[1]} -resize {$boxsize[0]}x{$boxsize[1]} -gravity center -extent {$pagesize[0]}x{$pagesize[1]}";
      }
      else {
        $resize = "-resize {$imagesize[0]}x{$imagesize[1]} -gravity center -extent {$pagesize[0]}x{$pagesize[1]}";
      }

      //if image should be resized
      $original_imagesize = getimagesize(drupal_realpath($item->uri));


      //if page size in mm is set
      if ($pagesize[0] && $pagesize[1]) {
        $offsetx = 0;
        $offsety = 0;
        if($pagesize[0] > $boxsize[0]) {
          $offsetx = floor(($pagesize[0] - $boxsize[0]) / 2);

        }
        //if ($boxsize[0] > $imagesize[0]) $offsetx += floor(($boxsize[0] - $imagesize[0]) / 2);
        if($pagesize[1] > $boxsize[1]) {
          $offsety = floor(($pagesize[1] - $boxsize[1]) / 2);

        }
        //if ($boxsize[1] > $imagesize[1]) $offsety += floor(($boxsize[1] - $imagesize[1]) / 2);

        $page = "-page {$pagesize[0]}x{$pagesize[1]}";
      }
      //if image DPI setting is given
      $dpi = $entity->field_resolution_of_the_images[LANGUAGE_NONE][0]['value'];
      if ($dpi > 0) {
        $dpi = "-density $dpi -units PixelsPerInch";
      }
    }

    dsm($entity);
    $attach = null;
    if ($entity->field_convert_to[LANGUAGE_NONE][0]['value'] == 1) {
      $attach = $temppdf;
    }

    if ($entity->field_quality[LANGUAGE_NONE][0]['value'] != "0") $quality = "-quality ".$entity->field_quality[LANGUAGE_NONE][0]['value'];
    //$cmd = "mogrify $imgEdits $resize $crop $tempimage";
    //dsm($cmd);
    //$operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('Modifying image %filename', array('%filename' => $filename)), $params, $nul));
    $cmd = "convert  \"$tempimage\" $units $resize $dpi $quality $temppdf";
    dsm($cmd);
    $operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('Converting image %filename to PDF', array('%filename' => $filename)), $params, $attach));



  }
  if (strpos($item->filemime, 'application/pdf') === 0)
  {
      $abpath = drupal_realpath($item->uri);
      $filename = basename($item->uri);
      $dest[] = drupal_realpath('public://')."/tmp/".$filename.".pdf";
      $temppdf = end($dest);

      //Decrypt PDF
      $input_pw = "";
      if (isset($item->field_unlock_password[LANGUAGE_NONE][0]['value']))
      {

          $input_pw = escapeshellarg($item->field_unlock_password[LANGUAGE_NONE][0]['value']);
          $cmd = "pdftk $abpath input_pw $input_pw output {$abpath}decrypted; mv -f {$abpath}decrypted {$abpath};";
          dsm($cmd);
          $operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('unlocking PDF %filename', array('%filename' => $filename)), $params, $nul));
      }


      $operations[] = array('onlinepdf_batch_pdfpagecount_process', array($abpath, t('Counting pages for ').$filename));

      //rotate pages
      $angle = "";
      if ($item->field_pdf_rotation[LANGUAGE_NONE][0]["value"] != 0)
      {
          $angle = "--angle '".$item->field_pdf_rotation[LANGUAGE_NONE][0]["value"]."'";
      }

      //page selection made
      $page_selection = "";
      if (isset($item->field_select_following_pages[LANGUAGE_NONE][0]["safe_value"]))
      {
          $page_selection = "'".$item->field_select_following_pages[LANGUAGE_NONE][0]["safe_value"]."'";
      }

      //convert paper size
      $papersize = "";
      if ($entity->field_page_size[LANGUAGE_NONE][0]['value'])
      {
          $dimentions = explode("x", $entity->field_page_size[LANGUAGE_NONE][0]['value']);
          $pageWidth = $dimentions[0] / 72;
          $pageHeight = $dimentions[1] / 72;
          $papersize = "--papersize '{".$pageWidth."in,".$pageHeight."in}'";
      }

      //set landscape option
      $landscape = "";
      if ($entity->field_page_orientation[LANGUAGE_NONE][0]['value'] == "Landscape") {
          $landscape = "--landscape";
      }
      if ($entity->field_page_orientation[LANGUAGE_NONE][0]['value'] == "Portrait") {
          $landscape = "--no-landscape";
      }

      //trimming
      $trim = "";
      if (isset($item->field_trim_page_edges[LANGUAGE_NONE][0]["value"]) && $item->field_trim_page_edges[LANGUAGE_NONE][0]["value"] && $item->field_trim_page_edges[LANGUAGE_NONE][0]["value"] != 0)
      {
          $trim = $item->field_trim_page_edges[LANGUAGE_NONE][0]["value"];
          $trim = "--trim '".$trim."mm ".$trim."mm ".$trim."mm ".$trim."mm' --clip true";
      }

      //get page margin
      $pagemargin = "";
      if ($entity->field_pagemargin[LANGUAGE_NONE][0]['value'] > 0)
      {
          $pagemargin = $entity->field_pagemargin[LANGUAGE_NONE][0]['value'];
          //assuming A4 format. This needs a FIX!
          $pagemargin = round(210 - ($pagemargin * 2))/210;
          $pagemargin = "--scale '{$pagemargin}'";
      }

      //rescale paper without rescaling content
      //exec("pdfjoin $abpath --paper letterpaper --fitpaper false --outfile $abpath");

      $cmd = "pdfjam $abpath $page_selection $pagemargin $papersize $frame --noautoscale false $trim $landscape $angle --outfile '$temppdf'";
      $operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('Rebuilding PDF %filename with new page settings', array('%filename' => $filename)), $params, $nul));

  }

  //file is office document
  if (strpos($item->filemime, 'text/plain') === 0 ||
      strpos($item->filemime, 'application/msword') === 0 ||
      strpos($item->filemime, 'application/vnd.ms-excel') === 0 ||
      strpos($item->filemime, 'application/vnd.ms-powerpoint') === 0 ||
      strpos($item->filemime, 'application/vnd.oasis.opendocument.text') === 0 ||
      strpos($item->filemime, 'application/vnd.oasis.opendocument.spreadsheet') === 0 ||
      strpos($item->filemime, 'application/vnd.oasis.opendocument.presentation') === 0 ||
      strpos($item->filemime, 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet') === 0 ||
      strpos($item->filemime, 'application/vnd.openxmlformats-officedocument.presentationml.presentation') === 0 ||
      strpos($item->filemime, 'application/vnd.openxmlformats-officedocument.wordprocessingml.document') === 0)
  {
      $abpath = drupal_realpath($item->uri);
      $filename = basename($item->uri);
      $dest[] = drupal_realpath('public://')."/tmp/".$filename.".pdf";
      $temppdf = end($dest);

      if (isset($item->field_grid) && $item->field_grid == 1 ) $grid = 1;
      else $grid = 0;
      if ($entity->field_page_orientation[LANGUAGE_NONE][0]['value'] == "none" || $entity->field_page_orientation[LANGUAGE_NONE][0]['value'] == "Portrait")
      {
          $orientation = 0;
      }
      else {
          $orientation = 1;
      }

      if (isset($item->field_scale_sheets_vertical[LANGUAGE_NONE][0]['value'])) {
          $vertical_scale =  $item->field_scale_sheets_vertical[LANGUAGE_NONE][0]['value'];
      }
      else
      {
          $vertical_scale = "0";
      }
      if (isset($item->field_scale_sheets_horizontal[LANGUAGE_NONE][0]['value'])) {
          $horizontal_scale = $item->field_scale_sheets_horizontal[LANGUAGE_NONE][0]['value'];
      }
      else
      {
          $horizontal_scale = 0;
      }
      if ($entity->field_page_size[LANGUAGE_NONE][0]['value'])
      {
          $dimentions = explode("x", $entity->field_page_size[LANGUAGE_NONE][0]['value']);
          $pageWidth = round(($dimentions[0] / 72) * 25.4 * 100);
          $pageHeight = round(($dimentions[1] / 72) * 25.4 * 100);
          $papersize = "--papersize '{".$pageWidth."in,".$pageHeight."in}'";
      }

      if ($entity->field_page_orientation[LANGUAGE_NONE][0]['value'] == "Landscape")
      {
          list($pageWidth,$pageHeight) = array($pageHeight,$pageWidth);
      }
      $margin = $entity->field_pagemargin[LANGUAGE_NONE][0]['value'] * 100;
      $cmd = 'python3 /home/jj/scripts/DocumentConverter.py "'.
      $abpath.'" "'.
      $temppdf.'" '.
      $grid.' '.
      $vertical_scale.' '.
      $orientation.' '.
      $pageWidth.' '.
      $pageHeight.' '.
      $margin.' '.
      $horizontal_scale;

      $operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('Converting document %filename to PDF', array('%filename' => $filename)), $params, $nul));

  }
}
//merge all to final PDF
               
$q = db_query("SELECT MAX(nid) as nid FROM node");
$record = $q->fetchObject();  

$allpdf = "";

$outfile = "/tmp/jpg-to-pdf.net-".$record->nid."-"._unique_id().".pdf";

foreach ($dest as $path)
{
    $allpdf .= "'".$path."' ";
}

//merge all pdf files together if saved as single PDF

if ($entity->field_convert_to[LANGUAGE_NONE][0]['value'] != 1) {
  $cmd = "pdftk $allpdf cat output $outfile";
  $operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('Merging pages to final PDF'), $params));
  $nup_horizontal = $entity->field_images_per_page_horizontal[LANGUAGE_NONE][0]['value'];
  $nup_vertical = $entity->field_images_per_page_vertical_[LANGUAGE_NONE][0]['value'];
  if ($nup_horizontal > 1 || $nup_vertical > 1) {
    $suffix = $nup_horizontal.'x'.$nup_vertical;
    if ($pagesize[1] > $pagesize[0]) {
      $landscape = '--no-landscape';
    }
    $cmd = "pdfnup --nup {$nup_horizontal}x{$nup_vertical} $landscape $outfile --outfile /tmp";

    $nup = preg_replace('/\\.[^.\\s]{3,4}$/', '', $outfile).'-nup.pdf';
    $operations[] = array('onlinepdf_batch_exec_process',
                    array($cmd, t('Merging multiple pages to one'), $params, $nup));
  }
}



//unlink all temp files
foreach ($dest as $path)
{
  $cmd = "unlink $path";
  $filename = basename($path);
  exec ($cmd);
  //$operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('deleting temporary file %filename', array('%filename' => $filename)), $params, $nul));
}