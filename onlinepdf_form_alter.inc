<?php

function onlinepdf_form_alter(&$form, &$form_state, $form_id) {
    if (isset($form['actions']['submit']))
    {
        $form['actions']['submit']['#attributes']['class'][] = 'btn';
        $form['actions']['submit']['#attributes']['class'][] = 'btn-danger';
    }
    if (isset($form['actions']['delete']))
    {
        $form['actions']['delete']['#attributes']['class'][] = 'btn';
        $form['actions']['delete']['#attributes']['class'][] = '';
    }
    if ($form_id == "pdf-online-node-form" || $form_id == "pdf_online_node_form")
    {
        hide($form['field_download']);
        drupal_set_title(t("Convert PDF to JPG free online."));
        $form['actions']['submit']['#value'] = t("Start conversion");
        global $language ;
        $lang = $form['field_action']['#language'];
        $form['field_action'][LANGUAGE_NONE]['#title'].= ' <a href="#" class="info-tooltip" data-toggle="tooltip" title data-original-title="'.
        t($form['field_action'][LANGUAGE_NONE]['#description']).'"><div class="info-tooltip"></div></a>';
        unset($form['field_action'][LANGUAGE_NONE]['#description']);
        
        $form['field_dpi_setting'][LANGUAGE_NONE]['#title'].= ' <a href="#" class="info-tooltip" data-toggle="tooltip" title data-original-title="'.
        t($form['field_dpi_setting'][LANGUAGE_NONE]['#description']).'"><div class="info-tooltip"></div></a>';
        unset($form['field_dpi_setting'][LANGUAGE_NONE]['#description']);
        
        $form['field_resize_in_percent'][LANGUAGE_NONE]['value']['#title'].= ' <a href="#" class="info-tooltip" data-toggle="tooltip" title data-original-title="'.
        $form['field_resize_in_percent'][LANGUAGE_NONE]['value']['#description'].'"><div class="info-tooltip"></div></a>';
        unset($form['field_resize_in_percent'][LANGUAGE_NONE]['value']['#description']);
        
        $form['field_quality'][LANGUAGE_NONE]['#title'].= ' <a href="#" class="info-tooltip" data-toggle="tooltip" title data-original-title="'.
        t($form['field_quality'][LANGUAGE_NONE]['#description']).'"><div class="info-tooltip"></div></a>';
        unset($form['field_quality'][LANGUAGE_NONE]['#description']);
        
        $form['field_supersamping_conversion'][LANGUAGE_NONE]['#description'] = t($form['field_supersamping_conversion'][LANGUAGE_NONE]['#description']);
        
        $form['field_dpi'][LANGUAGE_NONE]['value']['#title'].= ' <a href="#" class="info-tooltip" data-toggle="tooltip" title data-original-title="'.
        $form['field_dpi'][LANGUAGE_NONE]['value']['#description'].'"><div class="info-tooltip"></div></a>';
        unset($form['field_dpi'][LANGUAGE_NONE]['value']['#description']);
        array_unshift($form['#validate'], '_onlinepdf_node_validate');
        $form['#after_build'][] = 'onlinepdf_after_build';
    }
    drupal_add_js('sites/all/themes/corolla/dist/js/vendor/video.js', array('scope'=>'footer'));
    drupal_add_js('sites/all/themes/corolla/dist/js/flat-ui.min.js', array('scope'=>'footer'));
    drupal_add_js('sites/all/themes/corolla/docs/assets/js/application.js', array('scope'=>'footer'));

}

function onlinepdf_after_build($form, $form_state)
{
    hide($form['field_download']);
    drupal_add_js('sites/all/themes/corolla/dist/js/vendor/video.js', array('scope'=>'footer'));
    drupal_add_js('sites/all/themes/corolla/dist/js/flat-ui.min.js', array('scope'=>'footer'));
    drupal_add_js('sites/all/themes/corolla/docs/assets/js/application.js', array('scope'=>'footer'));
        
  return $form;
}


