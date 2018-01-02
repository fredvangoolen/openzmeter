/*
 * Copyright (C) 2015 Pedro Sánchez Alguacil
 *               2017 Eduardo Viciana Gamez
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

/* Graficas de energia y potencia*/
var LastEnergyPowerSample    = 0;
var ActiveEnergyData         = [];
var ReactiveEnergyData       = [];
var ActivePowerData          = [];
var ReactivePowerData        = [];
var ActiveEnergyChart        = null;
var ReactiveEnergyChart      = null;
var ActivePowerChart         = null;
var ReactivePowerChart       = null;
var ActivePowerChartUnits    = 'W';
var ReactivePowerChartUnits  = 'Wh';
var ActiveEnergyChartUnits   = 'W';
var ReactiveEnergyChartUnits = 'Wh';
var EnergyPowerSpinner       = new Spinner(SpinnerOpts);
var EnergyPowerTimer         = null;
function AjaxGetEnergyPower() {
  if(EnergyPowerTimer !== null) clearInterval(EnergyPowerTimer);
  $('#btnEnergyPowerNow').addClass('disabled');
  if(LastEnergyPowerSample === 0) ShowSpinner(EnergyPowerSpinner, 'EnergyPowerSpinner');
  var Period = parseInt(localStorage.getItem('GraphPeriod'));
  if(!Period) {
    Period = 86400000;
    localStorage.setItem('GraphPeriod', Period);
  }
  var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
  if(!GraphEnd) {
    GraphEnd = 0;
    localStorage.setItem('GraphEnd', 0);
  } 
  var GraphAggregation = localStorage.getItem('GraphAggregation');
  if(!GraphAggregation || (GetAggregText(GraphAggregation) === false)) {
    GraphAggregation = '10m';
    localStorage.setItem('GraphAggregation', GraphAggregation);
  }
  $.ajax({ url: "getSeries",
    timeout: 20000,
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data: JSON.stringify({ Period: Period, To: GraphEnd, Aggreg: GraphAggregation, LastSample: LastEnergyPowerSample, Series: 'active_energy, active_power, fundamental_reactive_energy, fundamental_reactive_power', Node: Node }),
    success: AjaxEnergyPower,
    complete: function() {
      $('#btnEnergyPowerNow').removeClass('disabled');
      if(parseInt(localStorage.getItem('GraphEnd')) === 0) setTimeout(AjaxGetEnergyPower, 3000);
    }
 });
}
function AjaxEnergyPower(result) {
  if(LastEnergyPowerSample === 0) {
    StopSpinner(EnergyPowerSpinner, 'EnergyPowerSpinner');
    ActiveEnergyData   = [];
    ReactiveEnergyData = [];
    ActivePowerData    = [];
    ReactivePowerData  = [];
  }
  if(result['Sampletime'] !== null) {
    for(i = 0; i < result['Sampletime'].length; i++) {
      ActiveEnergyData.push([new Date(result['Sampletime'][i]), result['active_energy'][i][0]]);
      ReactiveEnergyData.push([new Date(result['Sampletime'][i]), result['fundamental_reactive_energy'][i][0]]);
      ActivePowerData.push([new Date(result['Sampletime'][i]), result['active_power'][i][0]]);
      ReactivePowerData.push([new Date(result['Sampletime'][i]), result['fundamental_reactive_power'][i][0]]);
    }
    while(ActiveEnergyData.length > result['MaxSamples']) ActiveEnergyData.shift();
    while(ReactiveEnergyData.length > result['MaxSamples']) ReactiveEnergyData.shift();
    while(ActivePowerData.length > result['MaxSamples']) ActivePowerData.shift();
    while(ReactivePowerData.length > result['MaxSamples']) ReactivePowerData.shift();
    LastEnergyPowerSample = result['Sampletime'][result['Sampletime'].length - 1];
  }
  $('#ActiveEnergyAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  $('#ReactiveEnergyAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  $('#ActivePowerAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  $('#ReactivePowerAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  var Range = CalcRanges(ActiveEnergyData, [1], true); 
  var toShow = [];
  if((Math.abs(Range[0]) > 1000) || (Math.abs(Range[1]) > 1000)) {
    ActiveEnergyChartUnits = 'kWh';
    Range[0] = Range[0] / 1000;
    Range[1] = Range[1] / 1000;
    toShow = Div1000(ActiveEnergyData);
  } else {
    ActiveEnergyChartUnits = 'Wh';
    toShow = ActiveEnergyData;
  }
  if(ActiveEnergyChart == null) {
    ActiveEnergyChart = new Dygraph(document.getElementById("ActiveEnergyChart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "ActiveEnergyLegend",
        labels: ["Time", "Active Energy"],
        plotter: barChartNoSpacedPlotter,
        stepPlot: true,
        fillGraph: true,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        valueRange: Range,
        dateWindow: [result['From'], result['To']],
        xAxisHeight: 26,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        underlayCallback: function(context, area, dygraph) {
          HightlightWeekends("#EEE", context, area, dygraph);
        },
        zoomCallback: function(minDate, maxDate, yRanges) {
          $('#btnEnergyPowerNow').removeClass('active');
          localStorage.setItem('GraphEnd', maxDate);
          localStorage.setItem('GraphPeriod', maxDate - minDate);
          LastEnergyPowerSample = 0;
          AjaxGetEnergyPower();
        },
        interactionModel: {
          mousedown: mouseDownOZM,
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              $('#btnEnergyPowerNow').removeClass('active');
              localStorage.setItem('GraphEnd', g.dateWindow_[1]);
              localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastEnergyPowerSample = 0;
              AjaxGetEnergyPower();
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            $('#btnEnergyPowerNow').removeClass('active');
              localStorage.setItem('GraphEnd', g.dateWindow_[1]);
              localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastEnergyPowerSample = 0;
              AjaxGetEnergyPower();
          }
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">' + ActiveEnergyChartUnits + '</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleDateString() + ' - ' + date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px; line-height: 10px">' + x.toLocaleTimeString() + '<br>' + x.toLocaleDateString() + '</div>';
               }
             }
          }
        }
    );
  } else {
    ActiveEnergyChart.updateOptions({'file': toShow, valueRange: Range, dateWindow: [result['From'], result['To']] });
  }
  Range = CalcRanges(ReactiveEnergyData, [1], true);
  var toShow = [];
  if((Math.abs(Range[0]) > 1000) || (Math.abs(Range[1]) > 1000)) {
    ReactiveEnergyChartUnits = 'kvarh';
    Range[0] = Range[0] / 1000;
    Range[1] = Range[1] / 1000;
    toShow = Div1000(ReactiveEnergyData);
  } else {
    ReactiveEnergyChartUnits = 'varh';
    toShow = ReactiveEnergyData;
  }
  if(ReactiveEnergyChart == null) {
    ReactiveEnergyChart = new Dygraph(document.getElementById("ReactiveEnergyChart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "ReactiveEnergyLegend",
        labels: ["Time", "Reactive Energy"],
        plotter: barChartNoSpacedPlotter,
        stepPlot: true,
        fillGraph: true,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        valueRange: Range,
        dateWindow: [result['From'], result['To']],
        xAxisHeight: 26,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        underlayCallback: function(context, area, dygraph) {
          HightlightWeekends("#EEE", context, area, dygraph);
        },
        zoomCallback: function(minDate, maxDate, yRanges) {
          $('#btnEnergyPowerNow').removeClass('active');
          localStorage.setItem('GraphEnd', maxDate);
          localStorage.setItem('GraphPeriod', maxDate - minDate);
          LastEnergyPowerSample = 0;
          AjaxGetEnergyPower();
        },
        interactionModel: {
          mousedown: mouseDownOZM,
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              $('#btnEnergyPowerNow').removeClass('active');
              localStorage.setItem('GraphEnd', g.dateWindow_[1]);
              localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastEnergyPowerSample = 0;
              AjaxGetEnergyPower();
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            $('#btnEnergyPowerNow').removeClass('active');
              localStorage.setItem('GraphEnd', g.dateWindow_[1]);
              localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastEnergyPowerSample = 0;
              AjaxGetEnergyPower();
          }
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">' + ReactiveEnergyChartUnits + '</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleDateString() + ' - ' + date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px; line-height: 10px">' + x.toLocaleTimeString() + '<br>' + x.toLocaleDateString() + '</div>';
               }
             }
          }
        }
    );
  } else {
    ReactiveEnergyChart.updateOptions({'file': toShow, valueRange: Range, dateWindow: [result['From'], result['To']]});
  }

  Range = CalcRanges(ActivePowerData, [1], true);
  var toShow = [];
  if((Math.abs(Range[0]) > 1000) || (Math.abs(Range[1]) > 1000)) {
    ActivePowerChartUnits = 'kW';
    Range[0] = Range[0] / 1000;
    Range[1] = Range[1] / 1000;
    Div1000(ActivePowerData);
  } else {
    ActivePowerChartUnits = 'W';
    toShow = ActivePowerData;
  }
  if(ActivePowerChart == null) {
    ActivePowerChart = new Dygraph(document.getElementById("ActivePowerChart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "ActivePowerLegend",
        labels: ["Time", "Active Power"],
        stepPlot: true,
        fillGraph: true,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        valueRange: Range,
        dateWindow: [result['From'], result['To']],
        xAxisHeight: 26,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        underlayCallback: function(context, area, dygraph) {
          HightlightWeekends("#EEE", context, area, dygraph);
        },
        zoomCallback: function(minDate, maxDate, yRanges) {
          $('#btnEnergyPowerNow').removeClass('active');
          localStorage.setItem('GraphEnd', maxDate);
          localStorage.setItem('GraphPeriod', maxDate - minDate);
          LastEnergyPowerSample = 0;
          AjaxGetEnergyPower();
        },
        interactionModel: {
          mousedown: mouseDownOZM,
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              $('#btnEnergyPowerNow').removeClass('active');
              localStorage.setItem('GraphEnd', g.dateWindow_[1]);
              localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastEnergyPowerSample = 0;
              AjaxGetEnergyPower();
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            $('#btnEnergyPowerNow').removeClass('active');
              localStorage.setItem('GraphEnd', g.dateWindow_[1]);
              localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastEnergyPowerSample = 0;
              AjaxGetEnergyPower();
          }
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">' + ActivePowerChartUnits + '</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleDateString() + ' - ' + date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px; line-height: 10px">' + x.toLocaleTimeString() + '<br>' + x.toLocaleDateString() + '</div>';
               }
             }
          }
        }
    );
  } else {
    ActivePowerChart.updateOptions({'file': toShow, valueRange: Range, dateWindow: [result['From'], result['To']]});
  }

  Range = CalcRanges(ReactivePowerData, [1], true);
  var toSHow = [];
  if((Math.abs(Range[0]) > 1000) || (Math.abs(Range[1]) > 1000)) {
    ReactivePowerChartUnits = 'kvar';
    Ramge[0] = Range[0] / 1000;
    Range[1] = Range[1] / 1000;
    toShow = Div1000(ReactivePowerData);
  } else {
    ReactivePowerChartUnits = 'var';
    toShow = ReactivePowerData;
  }
  if(ReactivePowerChart == null) {
    ReactivePowerChart = new Dygraph(document.getElementById("ReactivePowerChart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "ReactivePowerLegend",
        labels: ["Time", "Reactive Power"],
        stepPlot: true,
        fillGraph: true,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        valueRange: Range,
        dateWindow: [result['From'], result['To']],
        xAxisHeight: 26,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        underlayCallback: function(context, area, dygraph) {
          HightlightWeekends("#EEE", context, area, dygraph);
        },
        zoomCallback: function(minDate, maxDate, yRanges) {
          $('#btnEnergyPowerNow').removeClass('active');
          localStorage.setItem('GraphEnd', maxDate);
          localStorage.setItem('GraphPeriod', maxDate - minDate);
          LastEnergyPowerSample = 0;
          AjaxGetEnergyPower();
        },
        interactionModel: {
          mousedown: mouseDownOZM,
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              $('#btnEnergyPowerNow').removeClass('active');
              localStorage.setItem('GraphEnd', g.dateWindow_[1]);
              localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastEnergyPowerSample = 0;
              AjaxGetEnergyPower();
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            $('#btnEnergyPowerNow').removeClass('active');
              localStorage.setItem('GraphEnd', g.dateWindow_[1]);
              localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastEnergyPowerSample = 0;
              AjaxGetEnergyPower();
          }
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">' + ReactivePowerChartUnits + '</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleDateString() + ' - ' + date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px; line-height: 10px">' + x.toLocaleTimeString() + '<br>' + x.toLocaleDateString() + '</div>';
               }
             }
          }
        }
    );
    window.dispatchEvent(new Event('resize'));
    Dygraph.synchronize([ActiveEnergyChart, ReactiveEnergyChart, ActivePowerChart, ReactivePowerChart], { range: false, zoom: false } );
  } else {
    ReactivePowerChart.updateOptions({'file': toShow, valueRange: Range, dateWindow: [result['From'], result['To']]});
  }
  $('#btnShowEnergyPowerYear').removeClass('active');
  $('#btnShowEnergyPowerMonth').removeClass('active');
  $('#btnShowEnergyPowerWeek').removeClass('active');
  $('#btnShowEnergyPowerDay').removeClass('active');
  if((result['To'] - result['From']) === 86400000) {
    $('#btnShowEnergyPowerDay').addClass('active');
  } else if((result['To'] - result['From']) === 604800000) {
    $('#btnShowEnergyPowerWeek').addClass('active');
  } else if((result['To'] - result['From']) === 2592000000) {
    $('#btnShowEnergyPowerMonth').addClass('active');
  } else if((result['To'] - result['From']) === 31536000000) {
    $('#btnShowEnergyPowerYear').addClass('active');
  }
  var GraphAggregation = localStorage.getItem('GraphAggregation');
  if(GraphAggregation !== result['Aggregation_Source']) {
    $('#aggreg_modal').modal('show');
    GraphAggregation = result['Aggregation_Source'];
  }
  $('#btnEnergyPower3S').removeClass('active');
  $('#btnEnergyPower1M').removeClass('active');
  $('#btnEnergyPower10M').removeClass('active');
  $('#btnEnergyPower1H').removeClass('active');
  if(GraphAggregation == '3s') {
    $('#btnEnergyPower3S').addClass('active');
  } else if(GraphAggregation == '1m') {
    $('#btnEnergyPower1M').addClass('active');
  } else if(GraphAggregation == '10m') {
    $('#btnEnergyPower10M').addClass('active');
  } else if(GraphAggregation == '1h') {
    $('#btnEnergyPower1H').addClass('active');
  }
  var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
  if(GraphEnd === 0) {
    $('#btnEnergyPowerNow').addClass('active');
  } else {
    $('#btnEnergyPowerNow').removeClass('active');
  }
  localStorage.setItem('GraphAggregation', GraphAggregation);
}
$('#btnEnergyPower3S').click(function() {
  localStorage.setItem('GraphAggregation', '3s');
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
});
$('#btnEnergyPower1M').click(function() {
  localStorage.setItem('GraphAggregation', '1m');
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
});
$('#btnEnergyPower10M').click(function() {
  localStorage.setItem('GraphAggregation', '10m');
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
});
$('#btnEnergyPower1H').click(function() {
  localStorage.setItem('GraphAggregation', '1h');
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
});
$('#btnShowEnergyPowerYear').click(function() {
  localStorage.setItem('GraphPeriod', 31536000000);
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
  localStorage.setItem('CountersPeriod', 'Year');
  AjaxGetEnergyPowerCounters();
});
$('#btnShowEnergyPowerMonth').click(function() {
  localStorage.setItem('GraphPeriod', 2592000000);
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
  localStorage.setItem('CountersPeriod', 'Month');
  AjaxGetEnergyPowerCounters();
});
$('#btnShowEnergyPowerWeek').click(function() {
  localStorage.setItem('GraphPeriod', 604800000);
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
  localStorage.setItem('CountersPeriod', 'Week');
  AjaxGetEnergyPowerCounters();
});
$('#btnShowEnergyPowerDay').click(function() {
  localStorage.setItem('GraphPeriod', 86400000);
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
  localStorage.setItem('CountersPeriod', 'Day');
  AjaxGetEnergyPowerCounters();
});
$('#btnEnergyPowerNow').click(function() {
  var End = parseInt(localStorage.getItem('GraphEnd'));
  if(End === 0) {
    $('#btnEnergyPowerNow').removeClass('active');
    localStorage.setItem('GraphEnd', ActiveEnergyChart.getOption('dateWindow')[1]);
    if(EnergyPowerTimer !== null) clearInterval(EnergyPowerTimer);
  } else {
    $('#btnEnergyPowerNow').addClass('active');
    localStorage.setItem('GraphEnd', 0);
    LastEnergyPowerSample = 0;
    AjaxGetEnergyPower();
  }
});
$('#btnShowEnergy').click(function() {
  $('#EnergyPowerTitle').text('Energy');
  $('#EnergySubPanel').removeClass('hidden');
  $('#PowerSubPanel').addClass('hidden');
  if(ActiveEnergyChart) $(ActiveEnergyChart.graphDiv).parent().removeAttr('style');
  if(ReactiveEnergyChart) $(ReactiveEnergyChart.graphDiv).parent().removeAttr('style');
  window.dispatchEvent(new Event('resize'));
  $('#btnShowEnergy').addClass('active');
  $('#btnShowPower').removeClass('active');
  localStorage.setItem('EnergyPowerTab', 'Energy');
});
$('#btnShowPower').click(function() {
  $('#EnergyPowerTitle').text('Power');
  $('#EnergySubPanel').addClass('hidden');
  $('#PowerSubPanel').removeClass('hidden');
  if(ActivePowerChart) $(ActivePowerChart.graphDiv).parent().removeAttr('style');
  if(ReactivePowerChart) $(ReactivePowerChart.graphDiv).parent().removeAttr('style');
  window.dispatchEvent(new Event('resize'));
  $('#btnShowEnergy').removeClass('active');
  $('#btnShowPower').addClass('active');
  localStorage.setItem('EnergyPowerTab', 'Power');
});



function AjaxSuccess60S(result) {
  if(result['evolution_time']) {
    UpdateEvolutionChart(result);
    $('#HarmonicEvolutionBtn').removeClass("disabled");
    $('#HarmonicEvolutionLoad').addClass("hidden");
  }
  $('#voltage_hour').text(Round2(result['hour_rms']));
  $('#voltage_last_day').text(Round2(result['day_rms']));
  $('#voltage_hour_min').text(Round2(result['hour_min']));
  $('#voltage_hour_max').text(Round2(result['hour_max']));
  $('#voltage_min_day').text(Round2(result['day_min']));
  $('#voltage_max_day').text(Round2(result['day_max']));
  UpdateHeader(result);
}


/* Maximizar y minimizar graficos */
var DialogElement           = null;
var PrevDialogContainer     = null;
var DialogResizeHandler     = null;
var DialogPrevResizeHandler = null;
$('#aggreg_modal_close').click(function() {
  $('#aggreg_modal').modal('hide');
});
$('#maximizeActiveEnergy').click(function() {
  $('#GraphDialogTitle').text('Active Energy');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#ActiveEnergyCol');
  DialogElement = $('#ActiveEnergyMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(ActiveEnergyChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    ActiveEnergyChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#maximizeReactiveEnergy').click(function() {
  $('#GraphDialogTitle').text('Reactive Energy');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#ReactiveEnergyCol');
  DialogElement = $('#ReactiveEnergyMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(ReactiveEnergyChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    ReactiveEnergyChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#maximizeActivePower').click(function() {
  $('#GraphDialogTitle').text('Active Power');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#ActivePowerCol');
  DialogElement = $('#ActivePowerMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(ActivePowerChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    ActivePowerChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#maximizeReactivePower').click(function() {
  $('#GraphDialogTitle').text('Reactive Power');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#ReactivePowerCol');
  DialogElement = $('#ReactivePowerMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(ReactivePowerChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    ReactivePowerChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#GraphDialogClose').click(function() {
  $(window).unbind('resize', DialogResizeHandler);
  $(window).bind('resize', DialogPrevResizeHandler);
  $(PrevDialogContainer).append(DialogElement);
  $('#GraphDialog').modal('hide');
  window.dispatchEvent(new Event('resize'));
});



/* Secuencia de inicio para cargar la pagina */
function UpdatePage() {
  AjaxGetEnergyPower();
  var View = localStorage.getItem('EnergyPowerTab');
  if(View === 'Power') {
    $('#btnShowPower').trigger('click');
  } else {
    $('#btnShowEnergy').trigger('click');
  }
//  AjaxGetEnergyPowerCounters();
}