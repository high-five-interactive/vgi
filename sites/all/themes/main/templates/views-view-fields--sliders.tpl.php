<?php

/**
 * @file
 * Default simple view template to all the fields as a row.
 *
 * - $view: The view in use.
 * - $fields: an array of $field objects. Each one contains:
 *   - $field->content: The output of the field.
 *   - $field->raw: The raw data for the field, if it exists. This is NOT output safe.
 *   - $field->class: The safe class id to use.
 *   - $field->handler: The Views field handler object controlling this field. Do not use
 *     var_export to dump this object, as it can't handle the recursion.
 *   - $field->inline: Whether or not the field should be inline.
 *   - $field->inline_html: either div or span based on the above flag.
 *   - $field->wrapper_prefix: A complete wrapper containing the inline_html to use.
 *   - $field->wrapper_suffix: The closing tag for the wrapper.
 *   - $field->separator: an optional separator that may appear before a field.
 *   - $field->label: The wrap label text to use.
 *   - $field->label_html: The full HTML of the label to use including
 *     configured element type.
 * - $row: The raw result object from the query, with all data it fetched.
 *
 * @ingroup views_templates
 */

$has_row_link = false;
if(isset($row->field_field_slider_link[0]['raw']['url']))
  $has_row_link = true;

if($has_row_link) {

  $attributes = "";

  if(@sizeof($row->field_field_slider_link[0]['raw']['attributes']) > 0) {
    $attr_array = $row->field_field_slider_link[0]['raw']['attributes'];

    $attr_values = array();

    foreach($attr_array as $key => $val) {
      $attr_values[] = $key . '="' . $val . '"';
    }

    $attributes = " " . join(' ', $attr_values);
  }

  echo '<a href="' . $row->field_field_slider_link[0]['raw']['url'] . '"' . $attributes . '>';
}
?>

<?php foreach ($fields as $id => $field): ?>
  <?php print $field->content; ?>
<?php endforeach; ?>

<?php if($has_row_link) {
echo '</a>';
} ?>
