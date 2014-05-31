<?php
/**
 * @file
 * Returns the HTML for a single Drupal page.
 *
 * Complete documentation for this file is available online.
 * @see https://drupal.org/node/1728148
 */
?>

<div id="page">

  <div id="top-bar">
    <div id="top-bar-inner">
      <?php print render($page['top_bar']); ?>
    </div>
  </div>

  <header class="header" id="header" role="banner">
    <div id="header-inner">
      <?php print render($page['header']); ?>
    </div>
  </header>

  <div id="main">
    <div id="main-inner">
      <?php if(!$is_front) print $breadcrumb; ?>

      <?php if(arg(0) == "products"): ?>
        <div class="search-filter">
          <div class="wrapper">
            <a class="search">Search</a>
            <a class="filter">Filter</a>
          </div>
        </div>
        <div class="dropdown-search">
          <div class="wrapper">
            <?php
              $block = module_invoke('search_by_page', 'block_view', '1');
              print render($block);
            ?>
          </div>
        </div>
        <div class="dropdown-filter">
          <div class="wrapper">
            <?php
              function printTaxTree($vid, $terms = false) {
                if($terms || $terms = taxonomy_get_tree($vid, 0, 1)) {
                  print('<ul>');
                  foreach ($terms as $term) {
                    print('<li>');

                    $is_parent = false;
                    if($child_terms = taxonomy_get_tree($vid, $term->tid, 1)) {
                      $is_parent = true;
                    }

                    if($is_parent) {
                      print l($term->name, 'taxonomy/term/' . $term->tid, array('attributes' => array('class' => "parent")));
                      printTaxTree($vid, $child_terms);
                    } else {
                      print l($term->name, 'taxonomy/term/' . $term->tid);
                    }

                    print('</li>');
                  }
                  print('</ul>');
                }
              }
            ?>
            <div class="categories">
              <a class="title">Category</a>
            <?php
              $vocab_object = taxonomy_vocabulary_machine_name_load("catalog");
              $vid = $vocab_object->vid;
              printTaxTree($vid);
            ?>
            </div>
            <div class="brands">
              <a class="title">Brand</a>
              <?php
                $vocab_object = taxonomy_vocabulary_machine_name_load("manufacturer");
                $vid = $vocab_object->vid;
                printTaxTree($vid);
              ?>
            </div>
          </div>
        </div>
      <?php endif; ?>
      <div id="content" class="column" role="main">
        <div id="content-inner">
          <?php print render($page['above_content']); ?>

          <?php print render($page['highlighted']); ?>

          <a id="main-content"></a>

          <?php print render($title_prefix); ?>
          <?php if ($title): ?>
            <h1 class="page__title title" id="page-title"><?php print $title; ?></h1>
          <?php endif; ?>
          <?php print render($title_suffix); ?>

          <?php print $messages; ?>
          <?php print render($tabs); ?>
          <?php print render($page['help']); ?>
          <?php if ($action_links): ?>
            <ul class="action-links"><?php print render($action_links); ?></ul>
          <?php endif; ?>
          <?php print render($page['content']); ?>
          <?php print $feed_icons; ?>

        </div> <!-- /#content-inner -->
      </div> <!-- /#content -->

      <?php
        // Render the sidebars to see if there's anything in them.
        $sidebar_first  = render($page['sidebar_first']);
        $sidebar_second = render($page['sidebar_second']);
      ?>

      <?php if ($sidebar_first || $sidebar_second): ?>
        <aside class="sidebars">
          <?php print $sidebar_first; ?>
          <?php print $sidebar_second; ?>
        </aside>
      <?php endif; ?>

      </div> <!-- /#main-inner -->
    </div> <!-- /#main -->

  <div id="footer-wrapper">
    <?php print render($page['sub_footer']); ?>
    <?php print render($page['footer']); ?>
  </div>

</div> <!-- /#page -->

<?php print render($page['bottom']); ?>
