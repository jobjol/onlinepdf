<?php

/* returns imagesize, crop and boxsize array according to PDF settings
 * @arguments: $entity, $image (file entity)
 * @return: array(x resize pixels, y resize pixels,
 *                x crop pixels, y crop pixels,
 *                x-page pixels, y-page pixels,
 *                x-box pixels, y-box pixels) *
*/
function _onlinepdf_imgsizepdf($entity, $image) {
  $imagesize = getimagesize(drupal_realpath($image->uri));

  //image will get rotated after resize and crop, take this into account
  if ($image->field_rotation[LANGUAGE_NONE][0]['value'] == "90"
    | $image->field_rotation[LANGUAGE_NONE][0]['value'] == "-90") {
    array_flip($imagesize);
  }

  if ($entity->field_page_size[LANGUAGE_NONE][0]['value'] === 'adapt') {
    //calculate pagesize from img resolution
    if ($entity->field_resolution_of_the_images[LANGUAGE_NONE][0]['value'] > 0) {
      $pagesize[0] = round($imagesize[0] / $entity->field_resolution_of_the_images[LANGUAGE_NONE][0]['value'] * 25.4);
      $pagesize[1] = round($imagesize[1] / $entity->field_resolution_of_the_images[LANGUAGE_NONE][0]['value'] * 25.4);
    }
    else {
      $pagesize[0] = false;
      $pagesize[1] = false;
    }
  }
  elseif ($entity->field_page_size[LANGUAGE_NONE][0]['value'] == 'Set') {
    $pagesize[0] = $entity->field_width[LANGUAGE_NONE][0]['value'];
    $pagesize[1] = $entity->field_width[LANGUAGE_NONE][0]['value'];
  }
  else {
    $pagesize = explode('x',$entity->field_page_size[LANGUAGE_NONE][0]['value']);
  }

  // image horizontal? flip pagesize to horizontal
  switch ($entity->field_page_orientation[LANGUAGE_NONE][0]['value']) {
    case 'none':
      if ($imagesize[0] > $imagesize[1]) {
        array_flip($pagesize);
      }
      break;
    case 'Landscape':
      array_flip($pagesize);
      break;
    default:
      break;
  }
  $boxsize[0] = $pagesize[0];
  $boxsize[1] = $pagesize[1];
  $cropx = false;
  $cropy = false;
  // reduce pagesize with page margins
  // no pagesize = no boxsize
  if ($pagesize) {
    if ($entity->field_pagemargin[LANGUAGE_NONE][0]['value'] > 0) {
      $boxsize[0] = $pagesize[0] - (2 * ($entity->field_pagemargin[LANGUAGE_NONE][0]['value']));
      $boxsize[1] = $pagesize[1] - (2 * ($entity->field_pagemargin[LANGUAGE_NONE][0]['value']));
    }

    if ($entity->field_images_per_page_horizontal[LANGUAGE_NONE][0]['value'] > 1) {
      $boxsize[0] = floor($pagesize[0] / $entity->field_images_per_page_horizontal[LANGUAGE_NONE]['item']['value']);
      $boxsize[0] = $pagesize[0] - round($entity->field_distance[LANGUAGE_NONE]['item']['value'] / 2);
    }

    if ($entity->field_images_per_page_vertical_[LANGUAGE_NONE][0]['value'] > 1) {
      $boxsize[1] = floor($pagesize[1] / $entity->field_images_per_page_vertical_[LANGUAGE_NONE]['item']['value']);
      $boxsize[1] = $pagesize[1] - round($entity->field_distance[LANGUAGE_NONE]['item']['value'] / 2);
    }

    //transform boxsize to pixels
    if ($entity->field_resolution_of_the_images[LANGUAGE_NONE][0]['value'] > 0) {
      $boxsize[0] = round($boxsize[0] / $entity->field_resolution_of_the_images[LANGUAGE_NONE][0]['value'] * 25.4);
      $boxsize[0] = round($boxsize[0] / $entity->field_resolution_of_the_images[LANGUAGE_NONE][0]['value'] * 25.4);
    }
    else {
      //no DPI given. make boxsize large enough to fit image resolution
      //
      //TODO test this
      $hratio = round($imagesize[1] / $boxsize[1]);
      $wratio = round($imagesize[0] / $boxsize[0]);
      $ratio = min($hratio, $wratio);

      //if the source is smaller than the destination size,
      //don't resize -- add a margin instead
      //(that is, dont shrink boxes)
      if($ratio < 1.0) {
        $ratio = 1.0;
      }
      //compute sizes
      $sy = floor($boxsize[1] * $ratio);
      $sx = floor($boxsize[0] * $ratio);
      $boxsize[1] = $sy > $boxsize[1] ? $sy : false;
      $boxsize[0] = $sx > $boxsize[0] ? $sx : false;
    }

    //if image resolution is set, calculate pixels and crop. Else only crop and keep image res.
    switch ($entity->field_scaling[LANGUAGE_NONE][0]['value']) {
      case 1:
        //keep original size, crop if needed
        $cropx = $imagesize[0] > $boxsize[0] ? $boxsize[0] : false;
        $cropy = $imagesize[1] > $boxsize[1] ? $boxsize[1] : false;
      case 2:
        //scale to fit boxsize
        $v_fact = $boxsize[1] / $imagesize[1];
        $h_fact = $boxsize[0] / $imagesize[0];
        $im_fact = min($v_fact, $h_fact);
        $imagesize[1] = round($imagesize[1] * $im_fact);
        $imagesize[0] = round($imagesize[0] * $im_fact);
        break;
      case 3:
        //scale and crop to page width
        $scaling_factor = $boxsize[0] / $imagesize[0];
        $imagesize[0] = $boxsize[0];
        $imagesize[1] = round($imagesize[1] * $scaling_factor);
        $cropy = $imagesize[1] > $boxsize[1] ? $boxsize[1] : false;
      case 4:
        //scale and crop to page height
        $scaling_factor = $boxsize[1] / $imagesize[1];
        $imagesize[1] = $boxsize[1];
        $imagesize[0] = round($imagesize[0] * $scaling_factor);
        $cropy = $imagesize[0] > $boxsize[0] ? $boxsize[0] : false;
      case 5:
        //scale and crop to fill page
        //compute resize ratio
        $hratio = round($boxsize[1] / $imagesize[1]);
        $wratio = round($boxsize[0] / $imagesize[0]);
        $ratio = min($hratio, $wratio);

        //if the source is smaller than the thumbnail size,
        //don't resize -- add a margin instead
        //(that is, dont magnify images)
        if($ratio > 1.0) {
          $ratio = 1.0;
        }
        //compute sizes
        $sy = floor($imagesize[1] * $ratio);
        $sx = floor($imagesize[0] * $ratio);
        $cropy = $sy > $imagesize[1] ? $sy : false;
        $cropx = $sx > $imagesize[0] ? $sx : false;
    }

  }
  else { //no pagesize, boxsize, no img resolution, no resize or crop
    return false;
  }



  return array($imagesize[0], $imagesize[1],
    $cropx, $cropy,
    $pagesize[0], $pagesize[1],
    $boxsize[0], $boxsize[1]);
}