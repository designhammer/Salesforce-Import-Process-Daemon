<?php

/**
 * @file
 * Drushd integration for Salesforce Import module. Provides commands to import
 * Salesforce data using a drushd daemon.
 */

/**
 * Implementation of hook_drush_help().
 */
function sf_import_process_drush_help($section) {
  switch ($section) {
    case 'drush:sf-import-process-queue':
      return dt('Daemon to process the Salesforce import queue.');
  }
}

/**
 * Implementation of hook_drush_command().
 */
function sf_import_process_drush_command() {
  $items = array();

  $items['sf-import-process-queue'] = array(
    'daemon' => array(
      'class' => 'SfImportProcessDaemon',
      'file' => 'sf_import_process_daemon.inc',
      // file path is only necessary if your include is in a separate directory
      // than this file.
      //'file path' => dirname(__FILE__),
    ),
    // Since we have our own custom process method, you'll need to specify this
    // command-hook here, or Drush will try to find a callback function for this
    // command and whine at you about it.
    'command-hook' => 'drushd',
  // Everything below here is just typical drush command parameters.
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
    'description' => 'Daemon to process Salesforce import queue.',
    'drupal dependencies' => array('sf_import'),
    'aliases' => array('sip'),
    'options' => array(
      '--failure-threshold' => 'Float value representing the percentage of allowable failed items.',
      '--feedback' => 'Frequency of progress messages, in seconds or items processed.',
      '--filepath' => "The filepath of where log and status files should be saved. USE WITH CAUTION. If you fail to specify these same log files every time you run the command, you run the risk of multiple daemons running for a single site. It is recommended to use a drushrc.php file instead:

\$command_specific['sip'] = array(
  // Save all files to a log directory.
  'filepath' => '/var/www/html/example.com/log',
  // Optionally, specify the exact filenames:
  //'logfile' => '/var/log/example-com-sip.log',
  //'statusfile' => '/var/run/example-com-sip.status',
);",
    ),
    // Good to include these arguments, so people know how to use the daemon.
    'arguments' => array(
      'start'    => 'Start the daemon.',
      'status'   => 'Display status information about the daemon.',
      'stop'     => 'Stop the daemon.',
      'restart'  => 'Restart the daemon',
      'show-log' => 'Show the log file.',
    ),
    'examples' => array(
      'drush sip start'                            => 'Start the daemon.',
      'drush sip start --feedback="100 items"'     => 'Log a status message every 100 nodes.',
      'drush sip start --feedback="60 seconds"'    => 'Log a status message every 60 seconds.',
      'drush sip start --verbose'                  => 'Log verbosely.',
      'drush sip start --failure-threshold="0.05"' => 'Set the item failure threshold to 5%',
      ' '   => '',
      'drush sip status' => 'Show the current status of the daemon.',
      '  '  => '',
      'drush sip stop'              => 'Stop the daemon, allowing 45 seconds for processing.',
      'drush sip stop --timeout=10' => 'Allow 10 seconds for processing.',
      'drush sip stop --queue'      => 'Queue the sf import process daemon to stop.',
      '   ' => '',
      'drush sip restart'              => 'Restart the daemon, allowing 45 seconds for processing.',
      'drush sip restart --timeout=10' => 'Allow 10 seconds for processing.',
      'drush sip restart --queue'      => 'Queue the sf import process daemon to restart.',
      '    ' => '',
      'drush sip show-log' => 'Show the log file, using less.',
      'drush sip show-log --watch' => 'Watch the log file.',
      'drush sip show-log --tail' => 'Show just the tail of the log file, to see recent messages.',
    ),
  );

  $items['sf-import-queue-needs-processing'] = array(
    'hidden' => TRUE,
    'description' => 'Return or set whether the SF import queue needs processing.',
    'arguments' => array(
      'value' => 'Optional value to set sf import queue needs processing to, either 0 or 1.'
    ),
    'aliases' => array('sfinp'),
  );

  return $items;
}

/**
 * Retreive the current status, clearing the variable cache first.
 */
function _sf_import_queue_needs_processing() {
  global $conf;
  unset($conf['sf_import_queue_needs_processing']);
  $conf = variable_init($conf, TRUE);
  $count = db_result(db_query("SELECT COUNT(sfid) FROM {sf_import_queue}"));
  return ($count > 0) ? TRUE : FALSE;
}

/**
 * Callback for drush sf-import-needs-processing
 */
function drush_sf_import_queue_needs_processing() {
  if (($args = drush_get_arguments()) && count($args) > 1) {
    variable_set('sf_import_queue_needs_processing', (bool) $args[1]);
    drush_log(dt('Queue needs processing has been set to !val', array('!val' => $args[1])));
  }
  $message = 'SF import queue does not need to be processed.';
  if (_sf_import_queue_needs_processing()) {
    $message = 'SF import queue needs to be processed.';
  }
  drush_log(dt($message), 'ok');
}
