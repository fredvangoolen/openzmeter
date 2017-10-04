

function ajaxSaveAccount(credentials) {
    var request = $.ajax({
        url: "account/ajaxSaveAccount",
        data: {
            credentials: credentials
        },
        timeout: 20000,
        type: "POST",
        dataType: "json"
    });
    return request;
}


fillForm(settings['credentials'], 'form#credentials');

$('form').bootstrapValidator({
    message: 'This field is required',
    live: 'enabled',
    feedbackIcons: {
        valid: 'glyphicon glyphicon-ok',
        invalid: 'glyphicon glyphicon-remove',
        validating: 'glyphicon glyphicon-refresh'
    },
    excluded: ['.disabled']
});

$('.save').click(function () {
    $('form#credentials').data('bootstrapValidator').validate();
    if ($('form#credentials').data('bootstrapValidator').isValid()) {
        var credentials = $('form#credentials').serializeObject();
        $('#save_modal .alert-info').show();
        $('#save_modal .alert-success').hide();
        $('#save_modal .alert-danger').hide();
        $('#save_modal .modal-footer').hide();
        $('#save_modal').modal('show');
        var request = ajaxSaveAccount(credentials);
        request.done(function (response) {
            $('#save_modal .alert-info').slideUp(500, function () {
                if (response == null || response['result'] == false) {
                    $('#save_modal .alert-danger').slideDown(500);
                } else {
                    $('#save_modal .alert-success').slideDown(500);
                    $('#save_modal .modal-footer').slideDown(500);
                }
            });
        });
    }
});
$('form').submit(function (e) {
    e.preventDefault();
})