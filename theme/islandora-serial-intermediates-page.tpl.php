<?php

/**
 * @file
 * The default view of serial intermediates object view.
 *
 */
?>
<div class="islandora-object islandora">
  <div class="islandora-default-metadata">
    <?php print $description; ?>
    <?php print $metadata; ?>
  </div>
</div>

<?php if ($parent_collections): ?>
  <div>
    <h2><?php print t('In collections'); ?></h2>
    <ul>
      <?php foreach ($parent_collections as $collection): ?>
        <li><?php print l($collection->label, "islandora/object/{$collection->id}"); ?></li>
      <?php endforeach; ?>
    </ul>
  </div>
<?php endif; ?>
