<div id="webform-export-status" class="webform-export-status webform-export-status-<?php print $node->nid; ?>">
  <table id="webform-export-status-<?php print $node->nid; ?>" class="pie webform-export-status">
    <thead>
      <tr>
        <th></th>
        <th>Entries</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <th><?php print t('Total'); ?></th>
        <td><?php print $values['total']; ?></td>
      </tr>
      <tr>
        <th><?php print t('Unique'); ?></th>
        <td><?php print $values['unique']; ?></td>
      </tr>
      <tr>
        <th><?php print t('Opt-ins'); ?></th>
        <td><?php print $values['optin']; ?></td>
      </tr>
    </tbody>
  </table>
</div>
