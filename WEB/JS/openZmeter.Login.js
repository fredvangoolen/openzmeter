$('form').submit(function (e) {
  $('form button').attr('disabled', 'disabled');
  e.preventDefault();
  $.ajax({
    url: "LogIn",
    timeout: 20000,
    type: "POST",
    contentType: 'application/json',
    dataType: "json",
    data: JSON.stringify({ Username: $('#username').val(), Password: md5($('#password').val()) }),
    success: function (response) {
      $('.alert-danger').slideUp(500, function () {
        if (response === null || response['Result'] === false) {
          $('.alert-danger').slideDown(500);
          $('form button').removeAttr('disabled');
          localStorage.removeItem('UserName');
        } else {
          $('.alert-success').slideDown(1000, function () {
            localStorage.setItem('UserName', $('#username').val());
            localStorage.setItem('LastLogin', response['LastLogin']);
            location.reload(true);
          });
        }
      });
    }
  });
});
particlesJS("divParticles", {particles:{number:{value:50},color:{value:"#2b2e2e"},size:{value:2},line_linked:{enable:true,distance:150,color:"#e8701c",opacity:0.3,width:1}}});
function StartPageLoad() {
  $('button').removeClass("disabled");
}