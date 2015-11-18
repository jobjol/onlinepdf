<?php
           //convert image to Image
            foreach ($file as $item)
                {
                    if (strpos($item->filemime, 'image/') === 0)
                    {
                        $abpath = drupal_realpath($item->uri);
                        $imagesize = getimagesize($abpath);
                        $filename = basename($item->uri);
                        
                        $temppdf = drupal_realpath('public://')."/tmp/".$filename;
                        
                        $quality = "";
                        $resize = $entity->field_resize_in_percent[LANGUAGE_NONE][0]['value'];
                        
                        $imgEdits = "";
                        $flip = "";
                        $resize = "";
                        $border = 0;
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
                        if (isset($item->field_equalize[LANGUAGE_NONE][0]['value']) && $item->field_equalize[LANGUAGE_NONE][0]['value'] == true) {
                            $imgEdits .= " -equalize";
                        }
                        if (isset($item->field_negate[LANGUAGE_NONE][0]['value']) && $item->field_negate[LANGUAGE_NONE][0]['value'] == true) {
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
                        //set resize
                        if ($entity->field_resize_in_percent[LANGUAGE_NONE][0]['value']) $resize = $entity->field_resize_in_percent[LANGUAGE_NONE][0]['value'];
                        dsm($resize);
                        if ($entity->field_quality[LANGUAGE_NONE][0]['value'] != "0") $quality = "-quality ".$entity->field_quality[LANGUAGE_NONE][0]['value'];
                        $cmd = "convert \"$abpath\" $imgEdits -resize {$resize}% $quality $temppdf";
                        $operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('Converting image %filename', array('%filename' => $filename)), $params, $temppdf));
                       
                    }
                    if (strpos($item->filemime, 'application/pdf') === 0)
                    {
                        //copy file to one we can manipulate
                        $abpath = drupal_realpath(file_unmanaged_copy($item->uri, "public://tmp/".basename($item->uri)));
                        
                        $filename = basename($abpath);
                        $dest[] = $abpath;
                        $tempjpg = drupal_realpath('public://')."/tmp/".basename($filename,".pdf")."-page%p.jpg";
                        //Decrypt PDF
                        $input_pw = "";
                        if (isset($item->field_unlock_password[LANGUAGE_NONE][0]['value']))
                        {
                            
                            $input_pw = escapeshellarg($item->field_unlock_password[LANGUAGE_NONE][0]['value']);
                            $cmd = "pdftk $abpath input_pw $input_pw output {$abpath}decrypted; mv -f {$abpath}decrypted {$abpath};";
                            $operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('unlocking PDF %filename', array('%filename' => $filename)), $params, $nul));
                        }
                        
                        //page selection made
                        $page_selection = "";
                        if (isset($item->field_select_following_pages[LANGUAGE_NONE][0]["safe_value"]) && preg_match("^[0-9](,[0-9])*$^", $item->field_select_following_pages[LANGUAGE_NONE][0]["safe_value"]))
                        {
                            $page_selection = escapeshellarg($item->field_select_following_pages[LANGUAGE_NONE][0]["safe_value"]);
                            $cmd = "pdfjam $abpath $page_selection --outfile $abpath";
                            $operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('Extracting selected pages from PDF %filename', array('%filename' => $filename)), $params, $nul));
                        }
                        
                        //count pdf pages and store in batch var
                        $operations[] = array('onlinepdf_batch_pdfpagecount_process', array($abpath, t('Counting pages for ').$filename));
                        
                        //auto crop PDF
                        if (isset($item->field_auto_crop_pdf_pages[LANGUAGE_NONE][0]["value"]) && $item->field_auto_crop_pdf_pages[LANGUAGE_NONE][0]["value"] == true)
                        {
                            $cmd = "pdfcrop $abpath $abpath";
                            $operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('Remove %filename white margins', array('%filename' => $filename)), $params, $nul));
                        }
                        
                        //manual crop PDF
                        $trim = "";
                        if (isset($item->field_trim_page_edges[LANGUAGE_NONE][0]["value"]) && $item->field_trim_page_edges[LANGUAGE_NONE][0]["value"] && $item->field_trim_page_edges[LANGUAGE_NONE][0]["value"] != 0)
                        {
                            $trim = escapeshellarg("-".$item->field_trim_page_edges[LANGUAGE_NONE][0]["value"]);
                            $cmd = "pdfcrop --margins $trim $abpath $abpath";                            
                            $operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('Crop %filename page margins', array('%filename' => $filename)), $params, $nul));
                        }
                        
                        //Prepare conversion
                        $quality = "";
                        if (isset($entity->field_quality[LANGUAGE_NONE][0]['value']) && $entity->field_quality[LANGUAGE_NONE][0]['value'] != 0)
                        {
                            $quality = "-quality ".escapeshellarg($entity->field_quality[LANGUAGE_NONE][0]['value']);
                        }
                        $dpi = dpi($entity->field_dpi_setting[LANGUAGE_NONE][0]['value'], $entity->field_dpi[LANGUAGE_NONE][0]['value']);
                        $resize = resize($entity).'%';
                        //convert pdf to jpg command line
                        //$cmd = "gs -q -dNOPAUSE -dBATCH -dSAFER -dFirstPage=%p -dLastPage=%p -sDEVICE=jpeg $quality -r{$dpi}x{$dpi} -sOutputFile=\"$tempjpg\" $abpath";
                        if (resize($entity) == 100)
                        {
                            $cmd = "convert -density $dpi {$abpath}[%n] $quality $tempjpg";
                        }
                        else {
                            $cmd = "convert -density $dpi {$abpath}[%n] -background \"#FFFFFF\" -alpha Deactivate -resize $resize  +profile '*' -profile USWebCoatedSWOP.icc -profile sRGB.icm $quality $tempjpg";

                        }
                        
                        //OVERRIDE extract instead of convert
                        if (isset($entity->field_action[LANGUAGE_NONE][0]['value']) && $entity->field_action[LANGUAGE_NONE][0]['value'] == 'extract')
                        {
                            $tempjpg2 = str_replace("%p", "", $tempjpg);
                            $cmd = "pdfimages -j $abpath $tempjpg2";
                        }
                        
                        $operations[] = array('onlinepdf_batch_pdfconv_process', array($cmd, $filename, $abpath, $tempjpg, $params));
                        
                        $tempjpg = str_replace("%p", "*", $tempjpg);
                        //rotate pages
                        if ($item->field_pdf_rotation[LANGUAGE_NONE][0]["value"] != 0)
                        {
                                $rot = $item->field_pdf_rotation[LANGUAGE_NONE][0]["value"];
                                $cmd = 'shopt -s nullglob; for f in '.$tempjpg.'; do convert $f -rotate '.$rot.' $f; done';
                                $operations[] = array('onlinepdf_batch_exec_process', array($cmd, t('Rotating pages of %filename', array('%filename' => $filename)), $params, $nul));
                        }
                        
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
                        
                        $temppdf = drupal_realpath('public://')."/tmp/".$filename.".pdf";
                        $tempjpg = drupal_realpath('public://')."/tmp/".$filename."%p.jpg";
                        
                        if (isset($item->field_grid) && $item->field_grid == 1 ) $grid = 1;
                        else $grid = 0;
                        
                            $orientation = 0;
                        
                        
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
                        
                        $pageWidth = 1;
                        $pageHeight = 1;
                        $margin = 0;
                        
                        dsm($pageWidth);
                        $output = 'python /home/jj/scripts/DocumentConverter.py "'.
                        $abpath.'" "'.
                        $temppdf.'" '.
                        $grid.' '.
                        $vertical_scale.' '.
                        $orientation.' '.
                        $pageWidth.' '.
                        $pageHeight.' '.
                        $margin.' '.
                        $horizontal_scale;
                                                
                        //c = exec( $output );
                       
                        $operations[] = array('onlinepdf_batch_exec_process', array($output, t('Preparing %filename for further processing', array('%filename' => $filename)), $params, $nul));
                        
                        //get page number
                        //exec('/usr/bin/pdfinfo '.$temppdf.' | awk \'/Pages/ {print $2}\'', $pagecount);
                        //dsm($pagecount);
                        //$pagecount = $pagecount[0];
                        
                        $operations[] = array('onlinepdf_batch_pdfpagecount_process', array($temppdf, t('Counting pages for ').$filename));
                        
                        //Do conversion
                            $quality = "";
                            if (isset($entity->field_quality[LANGUAGE_NONE][0]['value']) && $entity->field_quality[LANGUAGE_NONE][0]['value'] != 0)
                            {
                                $quality = "-dJPEGQ=".$entity->field_quality[LANGUAGE_NONE][0]['value'];
                            }
                            $dpi = dpi($entity->field_dpi_setting[LANGUAGE_NONE][0]['value'], $entity->field_dpi[LANGUAGE_NONE][0]['value']);
                            $cmd = ("gs -q -dNOPAUSE -dBATCH -dSAFER -dFirstPage=%p -dLastPage=%p -sDEVICE=jpeg $quality -r{$dpi}x{$dpi} -sOutputFile=\"$tempjpg\" $temppdf");
                        
                        //convert pdf to jpg
                        $tempjpg = str_replace("%p", "*", $tempjpg);
                        $operations[] = array('onlinepdf_batch_pdfconv_process', array($cmd, $filename, $temppdf, $tempjpg, $params));

                        

                    }
                } //end file scan