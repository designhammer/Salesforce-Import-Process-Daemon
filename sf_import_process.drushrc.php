<?php

/**
 * By default, sf_import_process will store log files
 * in sites/default/files. This is not great for security
 * so we store the log files in a users home directory.
 */
$command_specific['sip'] = array(
  // Save all files to a log directory.
  // @staging
  'filepath' => '/path/to/log/file',
  // @production
  // 'filepath' => ''
  // Optionally, specify the exact filenames:
  //'logfile' => '/var/log/example-com-sip.log',
  //'statusfile' => '/var/run/example-com-sip.status',
);
