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

function AjaxGetEnergy() {
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 31536000000, To: 0, Aggreg: '1h', LastSample: 0, Series: 'active_energy', Node: Node}),
           success: AjaxEnergy,
           complete: function (x) {
//             $('#btnCurrentNow').removeClass('disabled');
//             if(parseInt(localStorage.getItem('GraphEnd')) === 0) CurrentTimer = setTimeout(AjaxGetCurrent, 3000);
           }
  }); 
  
  
}
function AjaxEnergy() {
  var scatterTip = d3.tip()
        .attr('class', 'd3-tip')
        //.offset([-10, 0])
        .html(function (d) {
            var date = new Date(d.key[2]);
            return "<p><em>" + getStringEventType(d.key[0])
                    + "</em></p><ul class='small'><li>" + date.toLocaleString() + "." + date.getMilliseconds() + "</li><li>RMS : " + roundTo(d.value, 2).toFixed(2) + "%</li><li>Duration: " + formatTimeUnit(d.key[1]) + "</li></ul>";
        });


var data = [];// [{duration: 1323133, rms: 3, id_event: 9, type: 0, time: 1434711599000}, {duration: 5234432, rms: 453, id_event: 10, type: 1, time: 1434721599000}, {duration: 2344, rms: 200, id_event: 15, type: 1, time: 1434731599000}, {duration: 10, rms: 80, id_event: 15, type: 2, time: 1434751599000}];
for (var i in energy) {
    var val = roundTo(energy[i].value / 1000, 2);
    data.push({
        value: isNaN(val) ? 0 : val,
        time: new Date(+energy[i].time)
    });
}


var ndx = crossfilter(data);
var all = ndx.groupAll();

/**
 * Events by date
 */

var barDateTip = d3.tip()
        .attr('class', 'd3-tip')
        //.offset([-10, 0])
        .html(function (d) {
            return "<em>" + d.x.toLocaleDateString()
                    + "</em> : " + d.y + (d.y == 1 ? ' events' : ' event');
        });

var dateDimension = ndx.dimension(function (d) {
    return new Date(d.time);
});

/*var dateDimensionGroup = dateDimension.group(function (d) {
 return d3.time.day(d);
 });*/

var dateDimensionGroup = dateDimension.group(function (d) {
    return d3.time.day(d);
}).reduceSum(function (d) {
    return d.value;
});

var dateSelectorChart = dc.barChart('#date_selector_chart');

var date = new Date();
dateSelectorChart
        .width($('#date_selector_chart').parent().width())
        .height(60)
        .margins({top: 10, right: 20, bottom: 20, left: 30})
        .dimension(dateDimension)
        .group(dateDimensionGroup)
        .centerBar(true)
        .gap(1)
        .x(d3.time.scale().domain([new Date(date.getFullYear() - 1, date.getMonth(), date.getDate()), date]))
        .round(d3.time.days.round)
        .renderHorizontalGridLines(true)
        .alwaysUseRounding(true)
        .xUnits(d3.time.days)
        .yAxis().tickFormat(d3.format("d")).ticks(1).tickSubdivide(0);

var dateChart = dc.barChart('#date_chart');


dateChart
        .width($('#date_chart').parent().width())
        .height(180)
        .rangeChart(dateSelectorChart)
        .dimension(dateDimension)
        .group(dateDimensionGroup)
        .margins({top: 20, right: 20, bottom: 20, left: 20})
        .mouseZoomable(true)
        .centerBar(true)
        .elasticY(true)
        .x(d3.time.scale().domain([new Date(date.getFullYear() - 1, date.getMonth(), date.getDate()), date]))
        .xUnits(d3.time.days)
        .renderHorizontalGridLines(true)
        .round(d3.time.days.round)
        .alwaysUseRounding(true)
        .brushOn(false)
        .title(function (d) {
            return roundTo(d.value, 2).toFixed(2) + ' kWh';
        })
        .zoomScale([1, 14]);

dateChart.yAxis().tickFormat(d3.format("d"))
        .tickSubdivide(0);

/**
 * DAY OF WEEK
 */

var dayOfWeekChart = dc.rowChart('#weekday_chart');

var dayOfWeek = ndx.dimension(function (d) {
    var date = new Date(d.time);
    var day = date.getDay();
    var name = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
    return day + '.' + name[day];
});
var dayOfWeekGroup = dayOfWeek.group().reduceSum(function (d) {
    return d.value;
});


dayOfWeekChart.width($('#weekday_chart').parent().width())
        .height($('#weekday_chart').parent().width() / 2)
        .margins({top: 20, left: 10, right: 10, bottom: 20})
        .group(dayOfWeekGroup)
        .dimension(dayOfWeek)
        .ordinalColors(['#3182bd', '#6baed6', '#9ecae1', '#c6dbef', '#dadaeb'])
        .label(function (d) {
            return d.key.split('.')[1];
        }).title(function (d) {
    return roundTo(d.value, 2).toFixed(2) + ' kWh';
})
        .elasticX(true)
        .xAxis().ticks(4);

/**
 * DAY OF MONTH
 */

var dayOfMonthChart = dc.rowChart('#monthday_chart');

var dayOfMonth = ndx.dimension(function (d) {
    var date = new Date(d.time);
    return date.getDate();
});
var dayOfMonthGroup = dayOfMonth.group().reduceSum(function (d) {
    return d.value;
});


dayOfMonthChart.width($('#monthday_chart').parent().width())
        .height($('#monthday_chart').parent().width() / 2)
        .margins({top: 20, left: 10, right: 10, bottom: 20})
        .group(dayOfMonthGroup)
        .dimension(dayOfMonth)
        .title(function (d) {
            return roundTo(d.value, 2).toFixed(2) + ' kWh';
        })
        .elasticX(true)
        .xAxis().ticks(4);

/**
 * WEEK NUMBER
 */

var weekChart = dc.rowChart('#week_chart');
var week = ndx.dimension(function (d) {
    var date = new Date(d.time);
    return date.getWeek();
});

var weekGroup = week.group().reduceSum(function (d) {
    return d.value;
});


weekChart.width($('#week_chart').parent().width())
        .height(300)
        .margins({top: 20, left: 10, right: 10, bottom: 20})
        .group(weekGroup)
        .dimension(week).title(function (d) {
    return roundTo(d.value, 2).toFixed(2) + ' kWh';
})
        .elasticX(true)
        .xAxis().ticks(4);



dc.renderAll();


filterByDate('week');

$(':input[name="date_chart_range"]').change(function () {
    filterByDate($(this).val());
});




}


function UpdatePage() {
  AjaxGetEnergy();
}
function filterByDate(val) {
    var date = new Date();
    switch (val) {
        case 'today':
            var end = date;
            end.setHours(23);
            end.setMinutes(59);
            end.setSeconds(59);
            var start = new Date(end.getFullYear(), end.getMonth(), end.getDate());
            break;
        case 'yesterday':
            var end = date;
            end.setHours(23);
            end.setMinutes(59);
            end.setSeconds(59);
            end = new Date(end.getFullYear(), end.getMonth(), end.getDate() - 1);
            end.setHours(23);
            end.setMinutes(59);
            end.setSeconds(59);
            start = new Date(end.getFullYear(), end.getMonth(), end.getDate());
            break;
        case 'week':
            end = date;
            start = new Date(end.getFullYear(), end.getMonth(), end.getDate() - end.getDay());
            break;
        case 'month':
            end = date;
            start = new Date(end.getFullYear(), end.getMonth(), 1);
            break;
    }

    brush = dateSelectorChart.brush().extent([start, end]);

    gBrush = dateSelectorChart.svg().select('.brush')
            .call(brush);

    gBrush.call(brush.event);
}

function resetCharts() {
    dc.filterAll();
    dc.renderAll();
    filterByDate($(':input[name="date_chart_range"]:checked').val());
}



