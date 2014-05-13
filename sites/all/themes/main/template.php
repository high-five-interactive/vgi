<?php
/**
 * @file
 * Contains the theme's functions to manipulate Drupal's default markup.
 *
 * Complete documentation for this file is available online.
 * @see https://drupal.org/node/1728096
 */

function main_preprocess_html(&$variables) {
    $variables['mobile_menu'] = block_get_blocks_by_region('mobile_menu');
}

/**
 * Preprocesses the rendered tree for theme_menu_tree().
 */

function main_preprocess_menu_tree(&$variables) {

  $tree = new DOMDocument();
  @$tree->loadHTML($variables['tree']);

  $links = $tree->getElementsByTagname('li');

  // Is the menu coming from Merged Menus?
  if(strpos($links->item(0)->getAttribute('class'), "merged-menu") !== false) {

    $menu_title = "Menu";
    $is_submenu = false;

    preg_match('`plid-(\\d*)`', $links->item(0)->getAttribute('class'), $matches);

    if(isset($matches[1])) {
      $plid = $matches[1];

      $parent_link = menu_link_load($plid);
      $menu_title = $parent_link['link_title'];
      $is_submenu = true;
    }

    $variables['menu_title'] = $menu_title;
    $variables['is_merged_menu'] = true;
    $variables['is_submenu'] = $is_submenu;

  }
}

function main_menu_tree__main_menu($variables) {
  if(isset($variables['is_merged_menu'])) {
    $back_button = "";
    if($variables['is_submenu']) {
      $back_button = '<a class="mp-back" href="#">Back</a>';
    }

    return '<div class="mp-level"><h2 class="menu-title">' . $variables['menu_title'] . '</h2>' . $back_button . '<ul>' . $variables['tree'] . '</ul></div>';
  }

  return '<div class="menu-wrapper"><ul class="menu">' . $variables['tree'] . '</ul></div>';
}

function main_preprocess_region(&$variables, $hook) {
  $no_wrapper_regions = array('content');

  $double_wrapper_regions = array('header', 'top_bar', 'sub_footer');

  if (in_array($variables['region'], $no_wrapper_regions)) {
    $variables['theme_hook_suggestions'][] = 'region__no_wrapper';
  }

  if (in_array($variables['region'], $double_wrapper_regions)) {
    $variables['theme_hook_suggestions'][] = 'region__double_wrapper';
  }
}

function main_preprocess_block(&$variables, $hook) {
  $no_wrapper_blocks = array(
    'block-system-main', // Main System Block
    'block-block-2', // Logo Block
    'block-block-3', // Mobile Menu Button Block
    'block-block-4', // Apply Today Block
    'block-custom-search-blocks-1', // Header Search Block
    'block-block-5', // Header Tablet Search Button Block
    );

  if (in_array($variables['block_html_id'], $no_wrapper_blocks)) {
    $variables['theme_hook_suggestions'][] = 'block__no_wrapper';
  }
}


function main_pager($variables) {
  $tags = $variables['tags'];
  $element = $variables['element'];
  $parameters = $variables['parameters'];
  $quantity = 5;
  global $pager_page_array, $pager_total;

  // Calculate various markers within this pager piece:
  // Middle is used to "center" pages around the current page.
  $pager_middle = ceil($quantity / 2);
  // current is the page we are currently paged to
  $pager_current = $pager_page_array[$element] + 1;
  // first is the first page listed by this pager piece (re quantity)
  $pager_first = $pager_current - $pager_middle + 1;
  // last is the last page listed by this pager piece (re quantity)
  $pager_last = $pager_current + $quantity - $pager_middle;
  // max is the maximum page number
  $pager_max = $pager_total[$element];
  // End of marker calculations.

  // Prepare for generation loop.
  $i = $pager_first;
  if ($pager_last > $pager_max) {
    // Adjust "center" if at end of query.
    $i = $i + ($pager_max - $pager_last);
    $pager_last = $pager_max;
  }
  if ($i <= 0) {
    // Adjust "center" if at start of query.
    $pager_last = $pager_last + (1 - $i);
    $i = 1;
  }
  // End of generation loop preparation.

  $li_previous = theme('pager_previous', array('text' => (isset($tags[1]) ? $tags[1] : t('‹')), 'element' => $element, 'interval' => 1, 'parameters' => $parameters));
  $li_next = theme('pager_next', array('text' => (isset($tags[3]) ? $tags[3] : t('›')), 'element' => $element, 'interval' => 1, 'parameters' => $parameters));

  if ($pager_total[$element] > 1) {

    if ($li_previous) {
      $items[] = array(
        'class' => array('pager-previous pager-item'),
        'data' => $li_previous,
      );
    }

    // When there is more than one page, create the pager list.
    if ($i != $pager_max) {

      // Now generate the actual pager piece.
      for (; $i <= $pager_last && $i <= $pager_max; $i++) {
        if ($i < $pager_current) {
          $items[] = array(
            'class' => array('pager-item'),
            'data' => theme('pager_previous', array('text' => $i, 'element' => $element, 'interval' => ($pager_current - $i), 'parameters' => $parameters)),
          );
        }
        if ($i == $pager_current) {
          $items[] = array(
            'class' => array('pager-current pager-item'),
            'data' => $i,
          );
        }
        if ($i > $pager_current) {
          $items[] = array(
            'class' => array('pager-item'),
            'data' => theme('pager_next', array('text' => $i, 'element' => $element, 'interval' => ($i - $pager_current), 'parameters' => $parameters)),
          );
        }
      }

    }
    // End generation.
    if ($li_next) {
      $items[] = array(
        'class' => array('pager-next pager-item'),
        'data' => $li_next,
      );
    }

    return '<h2 class="element-invisible">' . t('Pages') . '</h2>' . theme('item_list', array(
      'items' => $items,
      'attributes' => array('class' => array('pager full-pager')),
    ));
  }
}

function main_preprocess_views_view_table(&$vars) {
    $vars['classes_array'][] = 'responsive table';
}
