jQuery(document).ready(function($) {



 jQuery("#edit-search-block-form--2").val('Search');
 jQuery("#edit-search-block-form--2").focus(function(){
  if(jQuery(this).val() == "Search"){
   jQuery(this).val('');
  }
 });
 
 jQuery("#edit-search-block-form--2").blur(function(){
  if(jQuery(this).val() == ""){
   jQuery(this).val('Search');
  }
 });
 
 
 /* Decode HTML Characters for Vendors and Prodcuts*/
	jQuery.fn.decHTML = function() {
		return this.each(function(){
			var me   = jQuery(this);
			var html = me.html();
			me.html(html.replace(/&amp;/g,'&').replace(/&lt;/g,'<').replace(/&gt;/g,'>'));
		});
	};
	 
	jQuery.fn.decHTMLifEnc = function(){
		return this.each(function(){
			var me   = jQuery(this);
			var html = me.html();
			if(jQuery.fn.isEncHTML(html))
				me.html(html.replace(/&amp;/g,'&').replace(/&lt;/g,'<').replace(/&gt;/g,'>'));
		});
	};


	$(".field-name-field-category").decHTML();
	$(".field-name-field-links-vendors ").decHTML();

  $('body.not-logged-in.page-user ul.primary li:first-child').remove();


$('.field-group-htab').jScrollPane();



//$('.vendors-and-products').jScrollPane();
 
//  $('#quicktabs-tabpage-view__vendors_and_products__page-1.quicktabs-tabpage').removeClass('quicktabs-hide');
 
     //Apply quicktab scrollbars here
//     $('#quicktabs-tabpage-view__vendors_and_products__page-1 .vendors-and-products').jScrollPane();
 
     // Return hidden quicktab content to original state
//     $('#quicktabs-tabpage-view__vendors_and_products__page-1.quicktabs-tabpage').addClass('quicktabs-hide');
 
 $('.quicktabs-tabpage').each(function(){
   var hide = $(this).hasClass('quicktabs-hide');
   $(this).removeClass('quicktabs-hide');
   $(this).find('.vendors-and-products, .view.view-engineered-systems-detail').jScrollPane({verticalGutter: 30});
   if (hide) {
     $(this).addClass('quicktabs-hide');
   }
 });

 if ($('.quicktabs-tabpage:visible').size() == 0) {
  $('.quicktabs-style-nostyle ul li a').eq(0).click();
 }

 $('#webform-component-reset-button').appendTo('#content .form-actions');
});
