function LoadHelpPage(name) {
  $.ajax({ url: "HelpPages/" + name + ".md",
           timeout: 20000,
           type: "GET",
           success: function(result) {
             var renderer = new marked.Renderer();
             renderer.image = function (href, title, text) {
               var out = '<img src="/HelpPages/' + href + '" alt="' + text + '"';
               if (title) {
                 out += ' title="' + title + '"';
               }
               out += this.options.xhtml ? '/>' : '>';
               return out;
             };
             document.getElementById('Contents').innerHTML = '<h3>' + name + '<hr/></h3>' + marked(result, {renderer:renderer}); 
           }
  });
}

function UpdatePage() {
  LoadHelpPage("API");  
}