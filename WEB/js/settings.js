
function ajaxSaveSettings(database, electrical, budget, budget_country) {
    var request = $.ajax({
        url: "settings/ajaxSaveSettings",
        data: {
            database: database,
            electrical: electrical,
            budget: budget,
            budget_country: budget_country
        },
        timeout: 20000,
        type: "POST",
        dataType: "json"
    });
    return request;
}

$(':input[name="fixed_price"]').change(function () {
    $('form[id^="budget-"]').slideUp(500);
    if ($(this).val() == '') {
        $('div.price_container').fadeOut(200, function () {
            $('div.country_container').fadeIn(200);
        });
        $(':input[name="country"]').change();
    } else {
        $(this).val('true');
        $('div.country_container').fadeOut(200, function () {
            $('div.price_container').fadeIn(200);
        });
    }
});

$(':input[name="country"]').change(function () {
    if ($(':input[name="fixed_price"]').val() == '') {
        switch ($(':input[name="country"]').val()) {
            case 'spain':
                $('form#budget-spain').clearQueue();
                $('form#budget-spain').slideDown(500);
                break;
        }
    }
});

fillForm(settings['database'], 'form#database');
fillForm(settings['electrical'], 'form#electrical');
fillForm(settings['budget'], 'form#budget');
if (settings['budget'].fixed_price != '') {
    $(':input[name="price"]').val(settings['budget'].fixed_price);
} else {
    fillForm(settings['budget-' + settings['budget'].country], 'form#budget-' + settings['budget'].country);
}

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
    $('form#database').data('bootstrapValidator').validate();
    $('form#electrical').data('bootstrapValidator').validate();
    if ($(':input[name="fixed_price"]').val() == 'true') {
        $('form#budget').data('bootstrapValidator').validate();
        var validBudget = $('form#budget').data('bootstrapValidator').isValid();
    } else {
        $('form#budget-' + $(':input[name="country"]').val()).data('bootstrapValidator').validate();
        validBudget = $('form#budget-' + $(':input[name="country"]').val()).data('bootstrapValidator').isValid();
    }
    if ($('form#database').data('bootstrapValidator').isValid() && validBudget) {
        var database = $('form#database').serializeObject();
        var electrical = $('form#electrical').serializeObject();
        var budget = $('form#budget').serializeObject();
        if (budget.fixed_price == '') {
            budget.fixed_price = false;
            var budget_country = $('form#budget-' + $(':input[name="country"]').val()).serializeObject();
        } else {
            budget.fixed_price = budget.price;
            budget_country = null;
        }
        budget.price = undefined;
        $('#save_modal .alert-info').show();
        $('#save_modal .alert-success').hide();
        $('#save_modal .alert-danger').hide();
        $('#save_modal .modal-footer').hide();
        $('#save_modal').modal('show');
        var request = ajaxSaveSettings(database, electrical, budget, budget_country);
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