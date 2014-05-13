
<?php
// $Id: views-view-table.tpl.php,v 1.8 2009/01/28 00:43:43 merlinofchaos Exp $
/**
 * @file views-view-table.tpl.php
 * Template to display a view as a table.
 *
 * - $title : The title of this group of rows.  May be empty.
 * - $header: An array of header labels keyed by field id.
 * - $fields: An array of CSS IDs to use for each field id.
 * - $class: A class or classes to apply to the table, based on settings.
 * - $row_classes: An array of classes to apply to each row, indexed by row
 *   number. This matches the index in $rows.
 * - $rows: An array of row items. Each row is an array of content.
 *   $rows are keyed by row number, fields within rows are keyed by field ID.
 * @ingroup views_templates
 */
 
 
 
?>
<?php
foreach ($rows as $count => $row):
 ?>
  <?php foreach ($row as $field => $content): ?>
  
 <?php 
 if($field=='title') $title=$content;
  if($field=='body') $body=$content;
   if($field=='field_eng_img') $field_eng_img=$content;
 if($field=='view_node') $view_node=$content;
 if($field=='field_model_no') $field_model_no=$content;
 ?>
	      <?php endforeach; ?>
              	<div class="engineered-systems">
                	<div class="engineering-img">
                    <?php print $field_eng_img;?>
                    </div>
                    <div class="engineering-content">
                    	<h1><?php print $title;?></h1>
                        <div class="model_num"><?php print $field_model_no;?></div>
                    	<p><?php print $body;?></p>
                        <span><?php print $view_node;?></span>
                    </div>
               </div>
		<?php endforeach; ?>
        
        
