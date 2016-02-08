

<?php dsm($file); ?>
<p>
<span class="dl-link"><?php print l($file->filename, $file->url); ?></span><br/>
<?php print t('Size').': '.format_size($file->filesize) ?><br/>
<span><?php print l(t('Download'), 'download-direct/'.$file->fid, array('attributes' => array('class' => 'btn btn-danger'))); ?></span><br/>
</p><p>
<span><?php print l(t('Save to Dropbox'), $file->url, array('attributes' => array('class' => 'dropbox-saver'))); ?></span><br/>
<div class="g-savetodrive"
  data-src="//<?php print $file->url; ?>"
  data-filename="<?php print $file->filename; ?>"
  data-sitename="<?php print $_SERVER['HTTP_HOST'] ?>">
</div>
</p>
<script type="text/javascript" src="https://www.dropbox.com/static/api/2/dropins.js" id="dropboxjs" data-app-key="qfh4skus62t26ur"></script>
<script src="https://apis.google.com/js/platform.js" async defer></script>      
      

