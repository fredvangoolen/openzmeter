$('form').submit(function (e) {
  $('form button').attr('disabled', 'disabled');
  e.preventDefault();
  $.ajax({
    url: "LogIn",
    timeout: 20000,
    type: "POST",
    contentType: 'application/json',
    dataType: "json",
    data: JSON.stringify({ Username: $('#username').val(), Password: $('#password').val() }),
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
function StartPageLoad() {
  $('button').removeClass("disabled");
}