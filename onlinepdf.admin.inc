<?php


/**
 * OnlinePDF admin configuration form.
 */
function onlinepdf_service_configuration_form() {
  $form = array();

  $form['onlinepdf_winserver'] = array (
    '#type' => 'fieldset',
    '#title' => t('Windows slave server SSH settings'),
  );
  $form['onlinepdf_winserver']['onlinepdf_winserver_ip'] = array(
    '#type' => 'textfield',
    '#title' => t('IP address remote worker'),
    '#field_suffix' => t('Ip_v4 address'),
    '#required' => true,
    '#default_value' => variable_get('onlinepdf_winserver_ip'),
    '#description' => t('SSH ip_v4 address'),
  );
  $form['onlinepdf_winserver']['onlinepdf_winserver_user'] = array(
    '#type' => 'textfield',
    '#title' => t('Username SSH access'),
    '#required' => true,
    '#default_value' => variable_get('onlinepdf_winserver_user'),
    '#description' => t('Credentials WinSSH remote worker'),
  );
  $form['onlinepdf_winserver']['onlinepdf_winserver_password'] = array(
    '#type' => 'password',
    '#title' => t('Password SSH access'),
    '#required' => true,
    '#default_value' => variable_get('onlinepdf_winserver_password'),
    '#description' => t('Credentials WinSSH remote worker'),
  );
  $form['onlinepdf_conversion_limits'] = array (
    '#type' => 'fieldset',
    '#title' => t('Onlinepdf conversion limits'),
  );
  $form['onlinepdf_conversion_limits']['onlinepdf_max_jpg_resolution'] = array(
    '#type' => 'textfield',
    '#title' => t('Max. JPG resolution'),
    '#field_suffix' => t('Megapixels'),
    '#required' => true,
    '#default_value' => variable_get('onlinepdf_max_jpg_resolution'),
    '#description' => t('Max JPG resolution created in conversion.'),
  );
  $form['onlinepdf_conversion_limits']['onlinepdf_max_age'] = array(
    '#type' => 'textfield',
    '#title' => t('Max. age'),
    '#field_suffix' => t('seconds'),
    '#required' => true,
    '#default_value' => variable_get('onlinepdf_max_age', 3200),
    '#description' => t('The age in seconds before anonymous data and temp files are deleted.'),
  );
  $form['onlinepdf_conversion_limits']['onlinepdf_max_load'] = array(
    '#type' => 'textfield',
    '#title' => t('Max. system load'),
    '#field_suffix' => t('seconds'),
    '#required' => true,
    '#default_value' => variable_get('onlinepdf_max_load', 8),
    '#description' => t('Maximum system load to allow system operations.'),
  );
  $form['onlinepdf_conversion_limits']['onlinepdf_max_5min_load'] = array(
    '#type' => 'textfield',
    '#title' => t('Max. system load'),
    '#field_suffix' => t('seconds'),
    '#required' => true,
    '#default_value' => variable_get('onlinepdf_max_5min_load', 16),
    '#description' => t('Maximum 5 minute avg system load to allow system operations.'),
  );

  return system_settings_form($form);
}
