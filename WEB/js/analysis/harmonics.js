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

refreshTime = 3000;

$(':input[name="harmonics_chart_range"]').change(function () {
    switch ($(this).val()) {
        case '3s':
            refreshTime = 3000;
            break;
        case '10s':
            refreshTime = 10000;
            break;
        case '1m':
            refreshTime = 60000;
            break;
    }
});

$(':input[name="harmonics_chart_range"]').change(function () {
    switch ($(this).val()) {
        case '3s':
            refreshTime = 3000;
            break;
        case '10s':
            refreshTime = 10000;
            break;
        case '1m':
            refreshTime = 60000;
            break;
    }
});

var hideFundamental = false;
$('button.hideFundamental').click(function () {
    hideFundamental = !hideFundamental;
    var toShow = hideFundamental ? harmonicsDataset.slice(1) : harmonicsDataset;
    var max = null;
    if (hideFundamental) {
        max = toShow.reduce(function (a, b) {
            return [0, Math.max(a[1], b[1]), Math.max(a[2], b[2])];
        });
        max = Math.max(max[1], max[2]) + 0.5;
    }
    harmonicsChart.updateOptions({'file': toShow, 'dateWindow': [hideFundamental ? 1 : -1, 50], axes: {
            y: {
                valueRange: [1, hideFundamental ? max : 51]
            },
            y2: {
                valueRange: [1, hideFundamental ? max : 51]
            }
        }});
});

var harmonicsDataset = [];
var harmonicsChart = new Dygraph(document.getElementById("harmonics_chart"),
        harmonicsDataset,
        {
            strokeWidth: 2,
            labels: ["Time", "Voltage", "Current"],
            colors: ["#d9534f", "#428bca"],
            plotter: multiColumnBarPlotter, animatedZooms: true,
            dateWindow: [-1, 50],

            ylabel: 'Amplitude (dB)',
            xlabel: 'Frequency',
            series: {
                'Current': {
                    axis: 'y2'
                }
            },
            interactionModel: {
                'mousedown': mouseDownOZM,
                'mousemove': mouseMoveOZM,
                'mouseup': mouseUpOZM,
                'click': clickV3,
                'dblclick': dblClickV4,
                'mousewheel': scrollV3
            },
            axes: {
                y: {
                    valueRange: [1, 51],
                    valueFormatter: function (x, a, b, c, row, col) {
                        return roundTo(x - 50, 2).toFixed(2) + ' <em class="small">dB</em> => ' + (+harmonicsVoltageValues[row]).toFixed(2) + ' <em class="small">V</em>';
                    },
                    axisLabelFormatter: function (x) {
                        return '<small>' + roundTo(x - 50, 2).toFixed(2) + '</small>';
                    }
                },
                y2: {
                    valueRange: [1, 51],
                    valueFormatter: function (x, a, b, c, row, col) {
                        return roundTo(x - 50, 2).toFixed(2) + ' <em class="small">dB</em> => ' + (+harmonicsCurrentValues[row]).toFixed(2) + ' <em class="small">A</em>';
                    },
                    axisLabelFormatter: function (x) {
                        //return '<small>' + roundTo(x - 50, 2).toFixed(2) + '</small>';
                        return '';
                    }
                },
                x: {
                    ticker: function (min, max, pixels, opts, dygraph, vals) {
                        var ticks = [];
                        for (var i = 0; i < 500; i++) {
                            if (i % 2 == 0) {
                                ticks.push({v: i, label: '<small style="font-size:0.8vw">' + (i + 1) * 50 + '</small>'})
                            } else {
                                ticks.push({v: i, label: ''})
                            }
                        }
                        return ticks;
                        //return [{v: 0, label: "50"}, {v: 5, label: "5"}, {v: 10, label: "10"}];
                    },
                    valueFormatter: function (y) {
                        return roundTo(50 * y + 50, 0) + ' <em class="small">Hz</em>';
                    },
                    axisLabelFormatter: function (y) {
                        return '<small>' + roundTo(50 * y + 50, 0) + '</small>';
                    }
                }
            }
        }
);

var harmonicsVoltageValues = [];
var harmonicsCurrentValues = [];
function updateHarmonicsChart(result) {
    if (result != -1) {
        var timestamp = new Date(result.time);
        harmonicsDataset = [];
        var res = [];
        res['v'] = [];
        res['i'] = [];
        delete result['time'];
        var hideCurrent = false;
        if (result['harm_current'][i] < 0.1) {
            hideCurrent = true;
            $('.no-current').show(200);
        } else {
            $('.no-current').hide(200);
        }
        for (var i in result['harm_voltage']) {
            if (i == 0) {
                harmonicsDataset[i] = [+i, 50, !hideCurrent ? 50 : null];
            } else {
                harmonicsDataset[i] = [+i, 50 + 20.0 * Math.log10(result['harm_voltage'][i] / result['harm_voltage'][0]), !hideCurrent ? 50 + 20.0 * Math.log10(result['harm_current'][i] / result['harm_current'][0]) : null];
            }
        }
        harmonicsChart.updateOptions({'file': harmonicsDataset, 'dateWindow': [hideFundamental ? 1 : -1, 50]});
        harmonicsVoltageValues = result['harm_voltage'];
        harmonicsCurrentValues = result['harm_current'];
        $('span.harmonics_time').text(timestamp.toLocaleTimeString() + '.' + timestamp.getMilliseconds());
    } else {
        $('span.harmonics_time').text('No stored data');
        harmonicsChart.updateOptions({'file': []});
    }
}

function getHarmonics() {
    $('span.harmonics_time').text('Retrieving data...');
    var request = $.ajax({
        url: "analysis/ajaxGetHarmonics",
        data: {
            metric: 'all',
            interharmonics: 0
        },
        type: "POST",
        dataType: "json"
    });
    request.done(function (result) {
        updateHarmonicsChart(result);
        var pf = '?';
        var phi = '?';
        var thdv = '?';
        var thdi = '?';
        if (result != -1) {
            pf = roundTo(result['power_factor'], 2).toFixed(2);
            if(result['phi'] < 0) {
              $('span.phi_angle').text(roundTo(-result['phi'], 2).toFixed(2) + 'º (Capacitive)');
            } else if(result['phi'] > 0) {
              $('span.phi_angle').text(roundTo(result['phi'], 2).toFixed(2) + 'º (Inductive)');
            } else {
              $('span.phi_angle').text(roundTo(result['phi'], 2).toFixed(2) + 'º');
            }
            thdv = roundTo(result['thd_v'], 2).toFixed(2);
            thdi = roundTo(result['thd_i'], 2).toFixed(2);
        } else {
          $('span.phi_angle').text('?');  
        }
        $('span.power_factor').text(pf);
        
        $('span.thd_u').text(thdv);
        $('span.thd_i').text(thdi);
    });
    request.always(function () {
        setTimeout(getHarmonics, refreshTime);
    });
}
setTimeout(getHarmonics, refreshTime);