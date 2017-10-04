setInterval(function () {
    var request = $.ajax({
        url: "dashboard/ajaxGetMainValues",
        timeout: 6000,
        type: "POST",
        dataType: "json"
    });
    request.done(function (result) {
        if (Object.keys(result).length > 3) {
            $('#voltage_indicator').text(roundTo(result['voltage_rms'], 2).toFixed(2));
            $('#current_indicator').text(roundTo(result['current_rms'], 2).toFixed(2));
            $('#frequency_indicator').text(roundTo(result['freq'], 2).toFixed(2));
            $('#power_indicator').text(roundTo(result['power_p'] / 1000, 2).toFixed(2));
            $('#energy_indicator').text(roundTo(Number(result['energy_w']) / 1000, 2).toFixed(2));
        } else {
            $('#voltage_indicator').text('?');
            $('#current_indicator').text('?');
            $('#frequency_indicator').text('?');
            $('#power_indicator').text('?');
            $('#energy_indicator').text('?');
        }
    });

    var requestEvents = $.ajax({
        url: "events/ajaxGetNumUnreadEvents",
        timeout: 6000,
        type: "POST",
        dataType: "json"
    });
    requestEvents.done(function (numEvents) {
        $('#unread_events').text(numEvents == 0 ? '' : numEvents);
    });

}, 5000);