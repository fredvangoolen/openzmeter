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
var DialogElement           = null;
var PrevDialogContainer     = null;
var DialogResizeHandler     = null;
var DialogPrevResizeHandler = null;
var ShowCurrent             = false;
var EnergyTimeScale         = 'Day';
var EnergyKeepNow           = true;
var LastEnergyTime          = 0;
var ActiveEnergyData        = [];
var ReactiveEnergyData      = [];
var ActivePowerData         = [];
var ReactivePowerData       = [];
var ActiveEnergyChart       = null;
var ReactiveEnergyChart     = null;
var ActivePowerChart        = null;
var ReactivePowerChart      = null;
var ActivePowerChartUnits   = 'W';
var ReactivePowerChartUnits = 'Wh';
var ActiveEnergyChartUnits  = 'W';
var ReactiveEnergyChartUnits= 'Wh';
var EnergyAggregation       = '10m';
var SpinnerOpts             = { lines: 10, // The number of lines to draw
                                length: 10, // The length of each line
                                width: 5, // The line thickness
                                radius: 10, // The radius of the inner circle
                              };
var EnergySpinner           = new Spinner(SpinnerOpts);
function AjaxSuccess5S(result) {
  if(result != null && Object.keys(result).length > 0) {
    if(result['harmonics_v']) UpdateHarmonicsChart(result);
    if(result['fft_v']) UpdateFFTChart(result);
    if(result['wave_v']) UpdateWaveformChart(result);
    LastTimestamp = result['sampletime'];
    result.lastVoltage = result['rms_v'];
    result.lastCurrent = result['rms_i'];
    result.lastPower = result['active_power'];
    result.lastFrequency = result['freq'];
  } else {
    $('#voltage_integer').text('?');
    $('#voltage_decimal').text('');
    $('#voltage_last_day').text('?');
    $('#voltage_hour_min').text('?');
    $('#voltage_hour_max').text('?');
    $('#voltage_min_day').text('?');
    $('#voltage_max_day').text('?');
  }
  UpdateHeader(result);
}
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
function GetValues5S() {
  $.ajax({ url: "analysis/ajaxUpdateValues5sEnergyPower",
           timeout: 60000,
           type: "POST",
           dataType: "json",
           data: { },
           success: AjaxSuccess5S,
           complete: function (x) {
                       setTimeout(GetValues5S, 5000);
                     }
        });
}
function GetValues60S() {
  $.ajax({ url: "analysis/ajaxUpdateValues60sEnergyPower",
         timeout: 60000,
         type: "POST",
         dataType: "json",
         data: { },
         success: AjaxSuccess60S,
         complete: function (x) {
                     setTimeout(GetValues60S, 60000);
                   }
      });
}
function EnergyTickAggregationButtons(aggregation) {
  $('#energy_3s').removeClass('active');
  $('#energy_1m').removeClass('active');
  $('#energy_10m').removeClass('active');
  $('#energy_1h').removeClass('active');
  if(aggregation == '3 seconds') {
    $('#energy_3s').addClass('active');
    EnergyAggregation = '3s';
  } else if(aggregation == '1 minute') {
    $('#energy_1m').addClass('active');
    EnergyAggregation = '1m';
  } else if(aggregation == '10 minutes') {
    $('#energy_10m').addClass('active');
    EnergyAggregation = '10m';
  } else if(aggregation == '1 hour') {
    $('#energy_1h').addClass('active');
    EnergyAggregation = '1h';
  }
}
function EnergyGetData(period = 0, endDate = 0, AutoUpdate = true) {
  if((AutoUpdate == false) || (LastEnergyTime == 0)) {
    $('#EnergyPanel').find("*").prop("disabled", true);
    $('#EnergyPanel').css('opacity', '0.6');
    $('#EnergyPanel').append('<div id="SpinnerDiv" style="position:absolute; top:0; left:0; width:100%; height:100%; z-index:2;"></div>');
    EnergySpinner.spin(document.getElementById('SpinnerDiv'));
  }
  if(period == 0) {
    var Range = ActiveEnergyChart.getOption('dateWindow');
    period = (Range[1] - Range[0]) / 1000;
  }
  $.ajax({ url: "analysis/ajaxUpdateEnergyPower",
    timeout: 20000,
    type: "POST",
    dataType: "json",
    data: { period: period, to: endDate, agreg: EnergyAggregation, update: AutoUpdate, lastSample: LastEnergyTime / 1000 },
    success: AjaxEnergyUpdate,
    complete: function() {
      EnergySpinner.stop();
      $('#SpinnerDiv').remove();
      $('#EnergyPanel').css('opacity', '1.0');
      $('#EnergyPanel').find("*").prop("disabled", false);
      if(AutoUpdate && EnergyKeepNow) setTimeout(EnergyGetData, 5000);
    }
 });
}
function EnergyButtonsClick() {
  if($(this).val() == 'now') {
    EnergyKeepNow = !EnergyKeepNow;
    if(EnergyKeepNow) EnergyGetData(Period, 0, true);
    return;
  }
  if(this.checked == false) return;
  if($(this).val() == 'energy') {
    $('#EnergyPowerTitle').text('Energy');
    $('#EnergySubPanel').removeClass('hidden');
    $('#PowerSubPanel').addClass('hidden');
    $(ActiveEnergyChart.graphDiv).parent().removeAttr('style');
    $(ReactiveEnergyChart.graphDiv).parent().removeAttr('style');
    window.dispatchEvent(new Event('resize'));
    return;
  } else if($(this).val() == 'power') {
    $('#EnergyPowerTitle').text('Power');
    $('#EnergySubPanel').addClass('hidden');
    $('#PowerSubPanel').removeClass('hidden');
    $(ActivePowerChart.graphDiv).parent().removeAttr('style');
    $(ReactivePowerChart.graphDiv).parent().removeAttr('style');
    window.dispatchEvent(new Event('resize'));
    return;
  }    
  var Range = ActiveEnergyChart.getOption('dateWindow');
  var Period = (Range[1] - Range[0]) / 1000;
  var PrevScale = EnergyTimeScale;
  if(($(this).val() == '3s') || ($(this).val() == '1m') || ($(this).val() == '10m') || ($(this).val() == '1h')) {
    EnergyAggregation = $(this).val();
  } else if($(this).val() == 'day') {
    EnergyTimeScale = 'Day';
    Period = 86400;
  } else if($(this).val() == 'week') {
    EnergyTimeScale = 'Week';
    Period = 604800;
  } else if($(this).val() == 'month') {
    EnergyTimeScale = 'Month';
    Period = 2592000;
  } else if($(this).val() == 'year') {
    EnergyTimeScale = 'Year';
    Period = 31536000;
  }
  EnergyGetData(Period, EnergyKeepNow ? 0 : Range[1] / 1000, false);
  if(EnergyTimeScale != PrevScale) GetValues60S(true, false);
}
function AjaxEnergyUpdate(result) {
  if((result.update == true) && (EnergyKeepNow == false)) return;
  if(((EnergyAggregation == '1h') && (result.aggregation_source != '1 hour')) ||
     ((EnergyAggregation == '10m') && (result.aggregation_source != '10 minutes')) ||
     ((EnergyAggregation == '1m') && (result.aggregation_source != '1 minute')) ||
     ((EnergyAggregation == '3s') && (result.aggregation_source != '3 seconds'))) {
    if(result.update == true) return;
    $('#aggreg_modal').modal('show');
  }
  if(result.update == false) {
    ActiveEnergyData   = [];
    ReactiveEnergyData = [];
    ActivePowerData    = [];
    ReactivePowerData  = [];
  }
  for(i = 0; i < result.sampletime.length; i++) {
    LastEnergyTime = result.sampletime[i] * 1000 + 1000;
    ActiveEnergyData.push([new Date(LastEnergyTime), result.active_energy[i]]);
    ReactiveEnergyData.push([new Date(LastEnergyTime), result.reactive_energy[i]]);
    ActivePowerData.push([new Date(LastEnergyTime), result.active_power[i]]);
    ReactivePowerData.push([new Date(LastEnergyTime), result.reactive_power[i]]);
  }
  EnergyTickAggregationButtons(result.aggregation_source);
  $('#ActiveEnergyAggregation').text('From ' + result['aggregation_source'] + ' aggregation');
  $('#ReactiveEnergyAggregation').text('From ' + result['aggregation_source'] + ' aggregation');
  $('#ActivePowerAggregation').text('From ' + result['aggregation_source'] + ' aggregation');
  $('#ReactivePowerAggregation').text('From ' + result['aggregation_source'] + ' aggregation');
  if(result.update == true) {
    while(ActiveEnergyData.length > 10000) ActiveEnergyData.shift();
    while(ReactiveEnergyData.length > 10000) ReactiveEnergyData.shift();
    while(ActivePowerData.length > 10000) ActivePowerData.shift();
    while(ReactivePowerData.length > 10000) ReactivePowerData.shift();
  }
  var FirstLoad = (ActiveEnergyChart == null);
  UpdateActiveEnergyChart(result);
  UpdateReactiveEnergyChart(result);
  UpdateActivePowerChart(result);
  UpdateReactivePowerChart(result);
  if(FirstLoad) {
    $('#energy_day').addClass('active');
    window.dispatchEvent(new Event('resize'));
    Dygraph.synchronize([ActiveEnergyChart, ReactiveEnergyChart, ActivePowerChart, ReactivePowerChart], { range: false, zoom: false } );
  }
}
function UpdateActiveEnergyChart(result) {
  var UpperRange = -1000000;
  var LowerRange =  1000000;
  for (i = 0; i < ActiveEnergyData.length; i++) {
    if(ActiveEnergyData[i][1] > UpperRange) UpperRange = ActiveEnergyData[i][1];
    if(ActiveEnergyData[i][1] < LowerRange) LowerRange = ActiveEnergyData[i][1];
  }
  if((LowerRange > 0) && (UpperRange > 0)) LowerRange = 0;
  if((LowerRange < 0) && (UpperRange < 0)) UpperRange = 0;
  var Variation = (UpperRange - LowerRange);
  if(Variation < 1) Variation = 1.0;
  UpperRange = UpperRange + Variation * 0.1;
  LowerRange = LowerRange - Variation * 0.1;
  var toShow = [];
  if((Math.abs(UpperRange) > 1000) || (Math.abs(LowerRange) > 1000)) {
    ActiveEnergyChartUnits = 'kWh';
    UpperRange = UpperRange / 1000;
    LowerRange = LowerRange / 1000;
    for(i = 0; i < ActiveEnergyData.length; i++) {
      toShow.push([ActiveEnergyData[i][0], ActiveEnergyData[i][1] / 1000.0]);
    }
  } else {
    ActiveEnergyChartUnits = 'Wh';
    toShow = ActiveEnergyData;
  }
  if(ActiveEnergyChart == null) {
    ActiveEnergyChart = new Dygraph(document.getElementById("active_energy_chart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "ActiveEnergyLegend",
        labels: ["Time", "Active Energy"],
        plotter: barChartPlotterEnergy,
        stepPlot: true,
        fillGraph: true,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        valueRange: [LowerRange, UpperRange],
        dateWindow: [result.from * 1000 + 1000, result.to * 1000 + 1000],
        xAxisHeight: 26,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        zoomCallback: function(minDate, maxDate, yRanges) {
          EnergyGetData((maxDate - minDate) / 1000, maxDate / 1000, false);
          $('#energy_year').removeClass('active');
          $('#energy_month').removeClass('active');
          $('#energy_week').removeClass('active');
          $('#energy_day').removeClass('active');
        },
        interactionModel: {
          mousedown: function(event, g, contex) {
            mouseDownOZM(event, g, contex);
            $('#energynow').removeClass('active');
            EnergyKeepNow = false;
          },
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              var startDate = parseInt(g.dateWindow_[0]);
              var endDate = parseInt(g.dateWindow_[1]);
              EnergyGetData((endDate - startDate) / 1000, endDate / 1000, false);
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            var startDate = parseInt(g.dateWindow_[0]);
            var endDate = parseInt(g.dateWindow_[1]);
            EnergyGetData((endDate - startDate) / 1000, endDate / 1000, false);
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
    ActiveEnergyChart.updateOptions({'file': toShow, valueRange: [LowerRange, UpperRange], dateWindow: [result.from * 1000 + 1000, result.to * 1000 + 1000]});
  }
}
function UpdateReactiveEnergyChart(result) {
  var UpperRange = -1000000;
  var LowerRange =  1000000;
  for (i = 0; i < ReactiveEnergyData.length; i++) {
    if(ReactiveEnergyData[i][1] > UpperRange) UpperRange = ReactiveEnergyData[i][1];
    if(ReactiveEnergyData[i][1] < LowerRange) LowerRange = ReactiveEnergyData[i][1];
  }
  if((LowerRange > 0) && (UpperRange > 0)) LowerRange = 0;
  if((LowerRange < 0) && (UpperRange < 0)) UpperRange = 0;
  var Variation = (UpperRange - LowerRange);
  if(Variation < 1) Variation = 1.0;
  UpperRange = UpperRange + Variation * 0.1;
  LowerRange = LowerRange - Variation * 0.1;
  var toShow = [];
  if((Math.abs(UpperRange) > 1000) || (Math.abs(LowerRange) > 1000)) {
    ReactiveEnergyChartUnits = 'kvarh';
    UpperRange = UpperRange / 1000;
    LowerRange = LowerRange / 1000;
    for(i = 0; i < ReactiveEnergyData.length; i++) {
      toShow.push([ReactiveEnergyData[i][0], ReactiveEnergyData[i][1] / 1000.0]);
    }
  } else {
    ReactiveEnergyChartUnits = 'varh';
    toShow = ReactiveEnergyData;
  }
  if(ReactiveEnergyChart == null) {
    ReactiveEnergyChart = new Dygraph(document.getElementById("reactive_energy_chart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "ReactiveEnergyLegend",
        labels: ["Time", "Reactive Energy"],
        plotter: barChartPlotterEnergy,
        stepPlot: true,
        fillGraph: true,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        valueRange: [LowerRange, UpperRange],
        dateWindow: [result.from * 1000 + 1000, result.to * 1000 + 1000],
        xAxisHeight: 26,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        zoomCallback: function(minDate, maxDate, yRanges) {
          EnergyGetData((maxDate - minDate) / 1000, maxDate / 1000, false);
          $('#energy_year').removeClass('active');
          $('#energy_month').removeClass('active');
          $('#energy_week').removeClass('active');
          $('#energy_day').removeClass('active');
        },
        interactionModel: {
          mousedown: function(event, g, contex) {
            mouseDownOZM(event, g, contex);
            $('#energynow').removeClass('active');
            EnergyKeepNow = false;
          },
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              var startDate = parseInt(g.dateWindow_[0]);
              var endDate = parseInt(g.dateWindow_[1]);
              EnergyGetData((endDate - startDate) / 1000, endDate / 1000, false);
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            var startDate = parseInt(g.dateWindow_[0]);
            var endDate = parseInt(g.dateWindow_[1]);
            EnergyGetData((endDate - startDate) / 1000, endDate / 1000, false);
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
    ReactiveEnergyChart.updateOptions({'file': toShow, valueRange: [LowerRange, UpperRange], dateWindow: [result.from * 1000 + 1000, result.to * 1000 + 1000]});
  }
}
function UpdateActivePowerChart(result) {
  var UpperRange = -1000000;
  var LowerRange =  1000000;
  for (i = 0; i < ActivePowerData.length; i++) {
    if(ActivePowerData[i][1] > UpperRange) UpperRange = ActivePowerData[i][1];
    if(ActivePowerData[i][1] < UpperRange) LowerRange = ActivePowerData[i][1];
  }
  if((LowerRange > 0) && (UpperRange > 0)) LowerRange = 0;
  if((LowerRange < 0) && (UpperRange < 0)) UpperRange = 0;
  var Variation = (UpperRange - LowerRange);
  if(Variation < 1) Variation = 1.0;
  UpperRange = UpperRange + Variation * 0.1;
  LowerRange = LowerRange - Variation * 0.1;
  var toShow = [];
  if((Math.abs(UpperRange) > 1000) || (Math.abs(LowerRange) > 1000)) {
    ActivePowerChartUnits = 'kW';
    UpperRange = UpperRange / 1000;
    LowerRange = LowerRange / 1000;
    for(i = 0; i < ActivePowerData.length; i++) {
      toShow.push([ActivePowerData[i][0], ActivePowerData[i][1] / 1000.0]);
    }
  } else {
    ActivePowerChartUnits = 'W';
    toShow = ActivePowerData;
  }
  if(ActivePowerChart == null) {
    ActivePowerChart = new Dygraph(document.getElementById("active_power_chart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "ActivePowerLegend",
        labels: ["Time", "Active Power"],
        stepPlot: true,
        fillGraph: true,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        valueRange: [LowerRange, UpperRange],
        dateWindow: [result.from * 1000 + 1000, result.to * 1000 + 1000],
        xAxisHeight: 26,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        zoomCallback: function(minDate, maxDate, yRanges) {
          EnergyGetData((maxDate - minDate) / 1000, maxDate / 1000, false);
          $('#energy_year').removeClass('active');
          $('#energy_month').removeClass('active');
          $('#energy_week').removeClass('active');
          $('#energy_day').removeClass('active');
        },
        interactionModel: {
          mousedown: function(event, g, contex) {
            mouseDownOZM(event, g, contex);
            $('#energynow').removeClass('active');
            EnergyKeepNow = false;
          },
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              var startDate = parseInt(g.dateWindow_[0]);
              var endDate = parseInt(g.dateWindow_[1]);
              EnergyGetData((endDate - startDate) / 1000, endDate / 1000, false);
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            var startDate = parseInt(g.dateWindow_[0]);
            var endDate = parseInt(g.dateWindow_[1]);
            EnergyGetData((endDate - startDate) / 1000, endDate / 1000, false);
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
    ActivePowerChart.updateOptions({'file': toShow, valueRange: [LowerRange, UpperRange], dateWindow: [result.from * 1000 + 1000, result.to * 1000 + 1000]});
  }
}
function UpdateReactivePowerChart(result) {
  var UpperRange = -1000000;
  var LowerRange = 0;
  for (i = 0; i < ReactivePowerData.length; i++) {
    if(ReactivePowerData[i][1] > UpperRange) UpperRange = ReactivePowerData[i][1];
    if(ReactivePowerData[i][1] < LowerRange) LowerRange = ReactivePowerData[i][1];
  }
  if((LowerRange > 0) && (UpperRange > 0)) LowerRange = 0;
  if((LowerRange < 0) && (UpperRange < 0)) UpperRange = 0;
  var Variation = (UpperRange - LowerRange);
  if(Variation < 1) Variation = 1.0;
  UpperRange = UpperRange + Variation * 0.1;
  LowerRange = LowerRange - Variation * 0.1;
  var toSHow = [];
  if((Math.abs(UpperRange) > 1000) || (Math.abs(LowerRange) > 1000)) {
    ReactivePowerChartUnits = 'kvar';
    UpperRange = UpperRange / 1000;
    LowerRange = LowerRange / 1000;
    for(i = 0; i < ReactivePowerData.length; i++) {
      toShow.push([ReactivePowerData[i][0], ReactivePowerData[i][1] / 1000.0]);
    }
  } else {
    ReactivePowerChartUnits = 'var';
    toShow = ReactivePowerData;
  }
  if(ReactivePowerChart == null) {
    ReactivePowerChart = new Dygraph(document.getElementById("reactive_power_chart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "ReactivePowerLegend",
        labels: ["Time", "Reactive Power"],
        stepPlot: true,
        fillGraph: true,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        valueRange: [LowerRange, UpperRange],
        dateWindow: [result.from * 1000 + 1000, result.to * 1000 + 1000],
        xAxisHeight: 26,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        zoomCallback: function(minDate, maxDate, yRanges) {
          EnergyGetData((maxDate - minDate) / 1000, maxDate / 1000, false);
          $('#energy_year').removeClass('active');
          $('#energy_month').removeClass('active');
          $('#energy_week').removeClass('active');
          $('#energy_day').removeClass('active');
        },
        interactionModel: {
          mousedown: function(event, g, contex) {
            mouseDownOZM(event, g, contex);
            $('#energynow').removeClass('active');
            EnergyKeepNow = false;
          },
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              var startDate = parseInt(g.dateWindow_[0]);
              var endDate = parseInt(g.dateWindow_[1]);
              EnergyGetData((endDate - startDate) / 1000, endDate / 1000, false);
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            var startDate = parseInt(g.dateWindow_[0]);
            var endDate = parseInt(g.dateWindow_[1]);
            EnergyGetData((endDate - startDate) / 1000, endDate / 1000, false);
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
  } else {
    ReactivePowerChart.updateOptions({'file': toShow, valueRange: [LowerRange, UpperRange], dateWindow: [result.from * 1000 + 1000, result.to * 1000 + 1000]});
  }
}
$('#aggreg_modal_close').click(function() {
  $('#aggreg_modal').modal('hide');
});
$(':input[name="chart_range"]').change(EnergyButtonsClick);
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
$(document).ajaxError(function (event, jqxhr, settings, exception) {
  if(jqxhr.status === 200) window.location.href = 'login';
});
$(document).ready(EnergyGetData(86400, 0, true));
$(document).ready(GetValues5S());
$(document).ready(GetValues60S());