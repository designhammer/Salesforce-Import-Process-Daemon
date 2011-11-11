----------------
GET IT INSTALLED
================

To install the sf import process daemon, do the following:

    drush dl drushd

This will install drushd into a .drush directory by default.

-----------------
USING DRUSHRC.PHP
=================

If possible, you should use a drushrc.php file to pass a custom filepath for the
daemon logs.

Now, copy the `sf_import_daemon` directory into the .drush/ directory.
Edit the `sf_import_process.drushrc.php` file to make sure the log is written to
the correct location.

------------------
RUNNING THE DAEMON
==================


Run this command to start the sf import process daemon

    # The command in its simplest form, but you may prefer one of these other
    # options, below.
    drush sip start
    # To run the command, logging feedback every 60 seconds.
    drush sip start --feedback="60 seconds"
    # To run the command, with verbose logging and feedback.
    drush sip start --verbose --feedback="100 items"

-------------------
SEE WHATS HAPPENING
===================

Keep in mind, if the `sf_import_queue` is empty, the daemon
will go into hibernation, checking sf import process status every 15 minutes
or so. To see the current status of the sf import process process, run:

    drush sip status

If you'd like to manually trigger sf import processing, run this command:

    drush sfinp 1

If the daemon is hibernating, it may take 15 minutes for processing to start.
Once it has started, you can watch the progress by running:

    drush sip show-log --watch

CTRL-C out of this command when you are done watching. Keep in mind, if you
didn't specify any feedback, you won't see much happening in the log. If you'd
like to just see the end of the log, run:

    drush sip show-log --tail

To browse the whole log file, run:

    drush sip show-log

-------------------
STOPPING THE DAEMON
===================

To stop the daemon, run:

    drush sip stop

If it's hibernating, it will still have to go through the whole 15 minute cycle
before the daemon will stop. You can queue it up to happen behind the scenes, or
wait around for it to stop.

If PHP has the `posix_get_pid()` function available, you can also force kill the
process by running:

    # Use with caution, you'll have to delete the status file in order to
    # restart the daemon.
    drush sip kill

--------------------
OTHER THINGS OF NOTE
====================

The status file and log are saved to the drupal files directory. They will be
named `drushd_sf_import_process.log` and `drushd_sf_import_process.txt`.

If you would like to store these files in a separate directory, it is recommended
to use a drushrc.php file to do so. Inside the drushrc.php file, add an adaptation
of the following:

    <?php
    $command_specific['sip'] = array(
      // Save all files to a log directory.
      'filepath' => '/var/www/html/example.com/log',
      // Optionally, specify the exact filenames:
      //'logfile' => '/var/log/example-com-sip.log',
      //'statusfile' => '/var/run/example-com-sip.status',
    );
    ?>

Keep in mind, the user running the command will need write privileges to those
files.



