$('form').submit(function (e) {
    $('form button').attr('disabled', 'disabled');
    e.preventDefault();
    var formData = $('form').serializeObject();
    $.ajax({
      url: "Login/In",
      data: formData,
      timeout: 20000,
      type: "POST",
      dataType: "json",
      success: function (response) {
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
    }
    
    
    
    });
    

})