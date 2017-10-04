var url = window.location;
var element = $('.analysis_sections a').filter(function () {
  return this.href == url || url.href.indexOf(this.href) == 0;
}).addClass('active');