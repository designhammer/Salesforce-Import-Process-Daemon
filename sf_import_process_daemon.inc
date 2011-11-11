<?php

/**
 * @file
 * Continually process items stored in the {sf_import_queue}.
 */

class SfImportProcessDaemon extends ProcessItemsDaemon {
  /**
   * Process failure threshold. Decrease the default value to 1%.
   * @var float
   */
  protected $failureThreshold = 0.01;

  /**
   * Duration of hibernation in number of seconds.
   *
   * @var int
   */
  protected $hibernateDuration = 120;

  /**
   * Implementation of process method, to iterate over process items.
   */
  protected function process() {
    // This loads up 100 items from the sf_import_queue to process.
    // To adjust this value, pass an integer to getItems().
    $sfids = $this->getItems();

    return parent::process();
  }

  /**
   * Where the actual item processing occurs. Must return TRUE for a successful
   * process operation.
   *
   * @param string $sfid
   *   A string containing the SFID to be processed.
   * @return
   *   Boolean TRUE if the item was successfully processed.
   * @debug
   * //watchdog('processItem', 'sfid: !sfid, fieldmap !map, type !type, id !id maps <pre>!maps</pre>', array(
   *  //'!sfid' => $sfid, '!map' => $fieldmap, '!type' => $type, '!id' => $drupal_id, '!maps' => print_r($fieldmaps, TRUE)));
   */
  protected function processItem($sfid) {
    // Load the fieldmap associated with the SFID
    $fieldmap = db_result(db_query("SELECT fieldmap FROM {sf_import_queue} WHERE sfid = '%s'", check_plain($sfid)));
    // Get a list of all fieldmaps defined for the site
    $fieldmaps = salesforce_api_salesforce_field_map_load_all();
    // Get the Drupal type (node, user, etc)
    $type = $fieldmaps[$fieldmap]->drupal;
    if (strpos($type, 'node_') === 0) {
      $type = 'node';
    }
    // Build the import function
    $function = 'sf_' . $type . '_import';
    // Check to see if we have a Drupal ID associated with the SFID
    $drupal_id = salesforce_api_get_id_with_sfid($sfid, $type);
    // Run the import function
    if (function_exists($function)) {
      // @TODO We need to discuss if we should delete from the import queue
      // even if the import function fails.
      // Some queue items will always fail (i.e. if there are duplicate e-mail
      // addresses), so we need a better way to handle those errors.
      // For now, we will delete failed items from the queue.
      $oid = $function($sfid, $fieldmap, $drupal_id);
      db_query("DELETE FROM {sf_import_queue} WHERE sfid = '%s'", $sfid);
      if ($oid == FALSE) {
        watchdog('sf_import_process', t('There was an error in processing SFID !sfid with fieldmap !map and id !id'),
          array('!sfid' => $sfid, '!map' => $fieldmap, '!id' => $drupal_id));
      }
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Check to see if the sf_import_queue needs processing.
   */
  protected function needsProcessing() {
    // If we're hibernating, we need to use backend invoke to get the
    // sf_import_needs_processing status, so as to get a fresh value from the
    // database. Otherwise, we'd just be retrieving what exists in the static
    // cache, which wouldn't do us much good. :)
    if ($this->getStatus() == StatusHandler::HIBERNATING) {
      $needs_processing = _sf_import_queue_needs_processing() == 1;
      // Now store the fresh copy in sf_import_queue_needs_processing, for performance.
      variable_set('sf_import_queue_needs_processing', $needs_processing);
    }

    $needs_processing = db_result(db_query("SELECT COUNT(sfid) FROM {sf_import_queue}"));

    if ($needs_processing > 0) {
      // If we don't have any queued sfids, fill up the processItems array.
      $count = count($this->getAllItems());
      if ($count === 0) {
        drush_log(dt('Selected all SFIDS from the sf_import_queue.'));
        $sql_result = db_query("SELECT sfid FROM {sf_import_queue} ORDER BY time ASC LIMIT 100");
        while ($sfid = db_result($sql_result)) {
          $sfids[$sfid] = $sfid;
        }

        if ($count = count($sfids)) {
          $this->setItems($sfids);
        }
        // If we don't have any sfids, no reason to process.
        else {
          variable_set('sf_import_queue_needs_processing', FALSE);
          $needs_processing = FALSE;
        }
      }
      drush_log(dt('There are !num SFIDs to process.', array('!num' => $count)));
    }
    // Clean out the items if we don't need processing.
    else {
      $this->setItems();
    }

    return $needs_processing;
  }

  /**
   * There are no SFIDs left to process.
   */
  protected function processItemsEmpty() {
    $this->progressMessage(DrushDaemon::RESULT_HIBERNATING);
    variable_set('sf_import_queue_needs_processing', FALSE);
    // For some reason, this is getting invoked after every item is processed
    // which really slows things down.
    //drush_backend_invoke('cache-clear all');
  }
}
