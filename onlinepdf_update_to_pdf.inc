<?php

foreach ($file as $item)
                {

                    //convert image to PDF
                    if (strpos($item->filemime, 'image/') === 0)
                    {
                        $abpath = drupal_realpath($item->uri);
                        $imagesize = getimagesize($abpath);
                        $filename = basename($item->uri);
                        $dest[] = drupal_realpath('public://')."/tmp/".$filename.".pdf";
                        $temppdf = end($dest);
                        
                        $page = "";
                        $quality = "";
                        $density = $entity->field_dpi[LANGUAGE_NONE][0]['value'];
                        $imgEdits = "";
                        $flip = "";
                        $resize = "";
                        $xoffset = 0;
                        $yoffset = 0;
                        $border = 0;
                        $shave = "";
                        $background = "white";
                        
                        if ($item->field_rotation[LANGUAGE_NONE][0]['value'] == "90" | $item->field_rotation[LANGUAGE_NONE][0]['value'] == "-90")
                        {
                            list($imagesize[0], $imagesize[1]) = array($imagesize[1], $imagesize[0]);
                        }
                        if ($item->field_rotation[LANGUAGE_NONE][0]['value'] != "0") {
                            $imgEdits = "-rotate ".$item->field_rotation[LANGUAGE_NONE][0]['value'];
                        }
                        
                        if ($item->field_flip_vertically[LANGUAGE_NONE][0]['value'] == true) {
                            $imgEdits .= " -flip ";
                        }
                        if ($item->field_flip[LANGUAGE_NONE][0]['value'] == true) {
                            $imgEdits .= " -flop";
                        }
                        if ($item->field_equalize[LANGUAGE_NONE][0]['value'] == true) {
                            $imgEdits .= " -equalize";
                        }
                        if ($item->field_negate[LANGUAGE_NONE][0]['value'] == true) {
                            $imgEdits .= " -negate";
                        }
                        if (isset($item->field_sepia_color_effect[LANGUAGE_NONE][0]['value']) || isset($item->field_contrast[LANGUAGE_NONE][0]['value']) || isset($item->field_hue[LANGUAGE_NONE][0]['value']))
                        {
                            if ($item->field_sepia_color_effect[LANGUAGE_NONE][0]['value'] == 100 && $item->field_contrast[LANGUAGE_NONE][0]['value'] == 100 && $item->field_hue[LANGUAGE_NONE][0]['value'] == 100)
                            {
                                //do nothing, untouched. everything zero
                            }
                            elseif ($item->field_sepia_color_effect[LANGUAGE_NONE][0]['value'] == 0 && $item->field_contrast[LANGUAGE_NONE][0]['value'] == 0 && $item->field_hue[LANGUAGE_NONE][0]['value'] == 0)
                            {
                                //do nothing, untouched. everything zero
                            }
                            elseif ($item->field_sepia_color_effect[LANGUAGE_NONE][0]['value'] == -100 && $item->field_contrast[LANGUAGE_NONE][0]['value'] == -100 && $item->field_hue[LANGUAGE_NONE][0]['value'] == -100)
                            {
                                //do nothing, untouched. everything zero
                            }
                            else {
                                $imgEdits .= " -modulate ".$item->field_sepia_color_effect[LANGUAGE_NONE][0]['value'].",".$item->field_contrast[LANGUAGE_NONE][0]['value'].",".$item->field_hue[LANGUAGE_NONE][0]['value'];
                            }
                        }
                        
                        
                        $dimentions = explode("x", $entity->field_page_size[LANGUAGE_NONE][0]['value']);
                        If ($entity->field_page_orientation[LANGUAGE_NONE][0]['value'] == "Landscape")
                        {
                            list($dimentions[0], $dimentions[1]) = array($dimentions[1], $dimentions[0]);
                        }
                        //pagesize is adapted to content; Calculate
                        if ($entity->field_page_size[LANGUAGE_NONE][0]['value'] == "adapt")
                        {
                            $dimentions[0] = $imagesize[0];
                            $dimentions[1] = $imagesize[1];
                        }

                        
                        $pageWidth = $dimentions[0];
                        $pageHeight = $dimentions[1];
                        $PaperInch[0] = ($dimentions[0] / 72);
                        $PaperInch[1] = ($dimentions[1] / 72);
                        $PaperRatio = $dimentions[0] / $dimentions[1];
                        $ImageRatio = $imagesize[0] / $imagesize[1];
                        
                        
                        
                        If ($ImageRatio > $PaperRatio)
                        {
                            $ImgDens = round($imagesize[0]/$PaperInch[0]);
                        }
                        else
                        {
                            $ImgDens = round($imagesize[1]/$PaperInch[1]);
                        }
                        // If the input image has a higher resolution (dpi) than the wanted resolution of output.pdf
                        If ($ImgDens > $density)
                        {
                            //the image is resampled to the lower resolution
                            if ($entity->field_scaling[LANGUAGE_NONE][0]['value'] == 1 || $entity->field_scaling[LANGUAGE_NONE][0]['value'] == 3)
                            {
                                $ImgDens = $density;
                                
                            }
                            else //the image is cropped
                            {
                                
                                $shavex = max(0, round(($imagesize[0] - $PaperInch[0] * $density)/2));
                                $shavey = max(0, round(($imagesize[1] - $PaperInch[1] * $density)/2));
                                $shave = "-shave {$shavex}x{$shavey}";
                                
                                $imagesize[0] = $imagesize[0] - ($shavex * 2) ;
                                $imagesize[1] = $imagesize[1] - ($shavey * 2);
                                $ImageRatio = $imagesize[0] / $imagesize[1];
                                
                                
                                If ($ImageRatio > $PaperRatio)
                                {
                                    $ImgDens = round($imagesize[0]/$PaperInch[0]);
                                }
                                else
                                {
                                    $ImgDens = round($imagesize[1]/$PaperInch[1]);
                                }
                                
                            }
                        }
                        $ImgWidth = $PaperInch[0] *$ImgDens; 
                        $ImgHeight = $PaperInch[1] *$ImgDens;
                        $pageWidth = $PaperInch[0] *$ImgDens;
                        $pageHeight = $PaperInch[1] *$ImgDens;
                        
                        if ($ImgDens < $density)
                        {
                            //Don't scale the page resolution down when the input is to small
                            if ($entity->field_scaling[LANGUAGE_NONE][0]['value'] == 0 || $entity->field_scaling[LANGUAGE_NONE][0]['value'] == 1)
                            $ImgDens = $density;
                            $pageWidth = $PaperInch[0] *$ImgDens;
                            $pageHeight = $PaperInch[1] *$ImgDens;
                        }
                        
                        if ($entity->field_pagemargin[LANGUAGE_NONE][0]['value'] > 0)
                        {
                            $border = ($entity->field_pagemargin[LANGUAGE_NONE][0]['value']) * $ImgDens / 25.4;
                            $ImgWidth = $ImgWidth - ((($border * 2) ) );
                            $ImgHeight = $ImgHeight - ((($border * 2)) );
                            
                        }
                        
                        //Adapt page to image size
                        if ($entity->field_page_size[LANGUAGE_NONE][0]['value'] == "adapt")
                        {
                            $pageWidth = $ImgWidth;
                            $pageHeight = $ImgHeight;
                        }
                        $ImgWidth = round($ImgWidth);
                        $ImgHeight = round($ImgHeight);
                        $pageHeight = round($pageHeight);
                        $pageWidth = round($pageWidth);
                                                
                        //add frame
                        $frame = "";
                        if($entity->field_frame[LANGUAGE_NONE][0]['value'] == true)
                        { $frame = "-bordercolor Black -border 1"; }
                        
                        if ($entity->field_quality[LANGUAGE_NONE][0]['value'] != "0") $quality = "-quality ".$entity->field_quality[LANGUAGE_NONE][0]['value'];
                        
                            $cmd = "convert \"$abpath\" $imgEdits $shave -resize {$ImgWidth}x{$ImgHeight} -background $background -gravity center $frame -extent {$pageWidth}x{$pageHeight} -units PixelsPerInch -set density $ImgDens -repage {$pageWidth}x{$pageHeight}+0+0 $quality $temppdf";
                            $operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('Converting image %filename to PDF', array('%filename' => $filename)), $params, $nul));
                        

                        
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
                        
                        //Add frame
                        $frame = "";
                        if(isset($entity->field_frame[LANGUAGE_NONE][0]['value']) && $entity->field_frame[LANGUAGE_NONE][0]['value'] == true)
                        { $frame = "--frame true"; }
                        
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
                        $cmd = 'python /home/jj/scripts/DocumentConverter.py "'.
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

$outfile = drupal_realpath('public://')."/tmp2/online-pdf.org-".$record->nid."-"._unique_id().".pdf";
foreach ($dest as $path)
{
    $allpdf .= "'".$path."' ";
}

//merge all pdf files together
$cmd = "pdftk $allpdf cat output $outfile";
