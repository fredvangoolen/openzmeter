
function ajaxLogin(formData) {
    var request = $.ajax({
        url: "login/in",
        data: formData,
        timeout: 20000,
        type: "POST",
        dataType: "json"
    });
    return request;
}
$('form').submit(function (e) {
    $('form button').attr('disabled', 'disabled');
    e.preventDefault();
    var formData = $('form').serializeObject();
    var request = ajaxLogin(formData);
    request.done(function (response) {
        $('.alert-danger').slideUp(500, function () {
            if (response == null || response['result'] == false) {
                $('.alert-danger').slideDown(500);
                $('form button').removeAttr('disabled');
            } else {
                $('.alert-success').slideDown(500, function () {
                    location.reload(true);
                });
            }
        });
    });
})