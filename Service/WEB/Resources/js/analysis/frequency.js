/*
 * Copyright (C) 2015 Pedro Sánchez Alguacil
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


function generateFrequencyChart(start, end, dataset, minVal, maxVal) {
    frequencyChart.updateOptions({'file': dataset, dateWindow: [start, end], valueRange: [minVal - 0.1, maxVal + 0.1]});
}

$(':input[name="frequency_chart_range"]').change(function () {
    var date = new Date();
    switch ($(this).val()) {
        case 'day':
            var end = date;
            var start = new Date(end.getTime());
            start.setDate(end.getDate() - 1);
            break;
        case 'week':
            var end = date;
            var start = new Date(end.getTime());
            start.setDate(end.getDate() - 7);
            break;
        case 'month':
            var end = date;
            var start = new Date(end.getTime());
            start.setDate(end.getDate() - 31);
            break;
        case 'year':
            var end = date;
            var start = new Date(end.getTime());
            start.setDate(end.getDate() - 365);
            break;
    }
    generateFrequencyChart(start.getTime(), end.getTime());
});
function chartDrawCallback(startDate, endDate) {
    if (!redraw) {
        redraw = true;
        return;
    }
    clearTimeout(timer);
    timer = setTimeout(function () {
        $('#frequency_chart div.no-data').fadeOut(200, function () {
            $(this).remove();
        });
        spinner.spin(frequencyChart.graphDiv);
        var request = $.ajax({
            url: "analysis/ajaxGetRangedValues",
            timeout: 20000,
            type: "POST",
            dataType: "json",
            data: {
                startDate: startDate,
                endDate: endDate,
                metric: 'freq'
            }
        });
        request.done(function (result) {
            var frequencyDataset = [];
            var maxVal = 45;
            var minVal = 55;
            for (var i in result) {
                frequencyDataset.unshift([new Date(+result[i].time), result[i].value]);
                if (result[i].value == null || result[i].value === undefined) {
                    continue;
                }
                if (+result[i].value > maxVal) {
                    maxVal = +result[i].value;
                }
                if (+result[i].value < minVal) {
                    minVal = +result[i].value;
                }
            }
            redraw = false;
            generateFrequencyChart(startDate, endDate, frequencyDataset, minVal, maxVal);
            if (frequencyDataset.length == 0) {
                $('#frequency_chart').append('<div class="no-data text-center"><div class="alert alert-danger inline-block">No data at this zoom level</div></div>')
            }
            spinner.stop();
        });
    }, 500);
}

var frequencyDataset = [];
var maxVal = -11111111111;
var minVal = 99999999999;
for (var i in frequency.one_minute) {
    frequencyDataset.unshift([new Date(+frequency.one_minute[i].time), +frequency.one_minute[i].value]);
    if (+frequency.one_minute[i].value > maxVal) {
        maxVal = +frequency.one_minute[i].value;
    }
    if (+frequency.one_minute[i].value < minVal) {
        minVal = +frequency.one_minute[i].value;
    }
}
frequency.one_minute = frequencyDataset;

var timer;
var redraw = true;
var frequencyChart = new Dygraph(document.getElementById("frequency_chart"),
        frequencyDataset,
        {
            strokeWidth: 2,
            labels: ["Time", "Frequency"],
            colors: ["#31708f"],
            animatedZooms: true,
            valueRange: [minVal - 0.1, maxVal + 0.1],
            ylabel: 'Hz',
            drawCallback: function (g, is_initial) {
                if (!is_initial) {
                    if (g.dateWindow_ !== null) {
                        var startDate = parseInt(g.dateWindow_[0]);
                        var endDate = parseInt(g.dateWindow_[1]);
                        chartDrawCallback(startDate, endDate, is_initial);
                    }
                }
            },
            interactionModel: {
                'mousedown': mouseDownOZM,
                'mousemove': mouseMoveOZM,
                'mouseup': mouseUpOZM,
                'click': clickV3,
                'dblclick': function () {
                    $(':input[name="frequency_chart_range"]:checked').change()
                },
                'mousewheel': scrollV3
            },
            axes: {
                y: {
                    ticker: zTicker,
                    valueFormatter: function (x) {
                        return roundTo(x, 2).toFixed(2) + ' <em class="small">Hz</em>';
                    },
                    axisLabelFormatter: function (x) {
                        return '<small>' + roundTo(x, 2).toFixed(2) + '</small>';
                    }
                }
            }
        }
);
var spinner = new Spinner({
    lines: 10 // The number of lines to draw
    , length: 10 // The length of each line
    , width: 5 // The line thickness
    , radius: 10 // The radius of the inner circle
    , scale: 1 // Scales overall size of the spinner
    , corners: 1 // Corner roundness (0..1)
    , color: '#000' // #rgb or #rrggbb or array of colors
    , opacity: 0.25 // Opacity of the lines
    , rotate: 0 // The rotation offset
    , direction: 1 // 1: clockwise, -1: counterclockwise
    , speed: 1 // Rounds per second
    , trail: 60 // Afterglow percentage
    , fps: 20 // Frames per second when using setTimeout() as a fallback for CSS
    , zIndex: 2e9 // The z-index (defaults to 2000000000)
    , className: 'spinner' // The CSS class to assign to the spinner
    , top: '50%' // Top position relative to parent
    , left: '50%' // Left position relative to parent
    , shadow: false // Whether to render a shadow
    , hwaccel: false // Whether to use hardware acceleration
    , position: 'absolute' // Element positioning
});