function LoadHelpPage(name) {
  $.ajax({ url: name,
           timeout: 20000,
           type: "GET",
           success: function(result) {
             $('#Contents').html('<h3>' + name + '<hr/></h3>' + result); 
             $('#Contents').find('a').bind('click', function(e) {
               if((location.host == this.host) && (true)) {
                 LoadHelpPage($(this).attr('href'));
                 e.preventDefault();
               }
             });
//             var renderer = new marked.Renderer();
//             renderer.image = function (href, title, text) {
//               var out = '<img src="/HelpPages/' + href + '" alt="' + text + '"';
//               if (title) {
//                 out += ' title="' + title + '"';
//               }
//               out += this.options.xhtml ? '/>' : '>';
//               return out;
//             };
             
           }
  });
}
function HandleRebase() {
  $('#Contents').find('a').bind('click', function(e) {
    if((location.host == this.host) && (this.href.endsWith(".html"))) {
      $('#Contents').load($(this).attr('href'));
      e.preventDefault();
    }
  });
  $('#Contents').find('img').bind('load', function(e) {
    $(this).attr('src', "HelpPages/" + $(this).attr('href')) 
//    if((location.host == this.host) && (true)) {
//      $('#Contents').load("/HelpPages/" + $(this).attr('href'));
//      e.preventDefault();
//    }
  });
}

function UpdatePage() {
  $.get("Help/Topics.json", function(result) { 
    JSON.parse(result).forEach(function(name) { $('#Topics').append('<li><a href="' + name + '">' + name + '</a></li>'); });
    $('#Topics a').click(function(e) {
      $('#Contents .panel-heading').html($(this).attr('href')); e.preventDefault();
      $('#Contents .panel-body').load('Help/' + $(this).attr('href') + '.html'); e.preventDefault();
    });
  });
}