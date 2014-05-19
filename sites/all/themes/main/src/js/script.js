/**
 * @file
 * A JavaScript file for the theme.
 *
 * In order for this JavaScript to be loaded on pages, see the instructions in
 * the README.txt next to this file.
 */

// JavaScript should be made compatible with libraries other than jQuery by
// wrapping it with an "anonymous closure". See:
// - https://drupal.org/node/1446420
// - http://www.adequatelygood.com/2010/3/JavaScript-Module-Pattern-In-Depth
(function ($, Drupal, window, document, undefined) {

	$(function() { // Ready

    $('.menu-button').click(function() {
      $('.primary-navigation').toggleClass('open');
      return false;
    });

    // Label hiding
    var form_elements = $('.form-type-textfield, .webform-component-textfield, .webform-component-textfield, .webform-component-textarea, .form-email, .form-type-password, .webform-component-email');

    form_elements.find('input, textarea').change(function() {
      var elem = $(this);
      if(elem.val() != "")
        $('label[for="'+elem.attr('id')+'"]').hide();
    });

    form_elements.find('input, textarea').focus(function() {
      var elem = $(this);
      $('label[for="'+elem.attr('id')+'"]').hide();
    }).blur(function() {
      var elem = $(this);
      if(elem.val() == "")
        $('label[for="'+elem.attr('id')+'"]').show();
    });
    // End Label Hiding

	}); // End Ready

})(jQuery, Drupal, this, this.document);
