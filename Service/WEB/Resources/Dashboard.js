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

/* Grafica de tension */
var VoltageChart        = null;
var LastVoltageSample   = 0;
var VoltageData         = [];
var VoltageSpinner      = new Spinner(SpinnerOpts);
function AjaxGetVoltage() {
  if(LastVoltageSample === 0) {
    $('#VoltagePanel').find("*").prop("disabled", true);
    $('#VoltagePanel').css('opacity', '0.6');
    $('#VoltagePanel').append('<div id="SpinnerDivVoltage" style="position:absolute; top:0; left:0; width:100%; height:100%; z-index:2;"></div>');
    VoltageSpinner.spin(document.getElementById('SpinnerDivVoltage'));
  }
  $.ajax({ url: "getSeriesData",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 7200000, To: 0, Aggreg: "3s", LastSample: LastVoltageSample, Series: 'rms_v, flag', Node: Nodes['IDs'][Node] }),
           success: AjaxVoltage
        });
}
function AjaxVoltage(result) {
  setTimeout(AjaxGetVoltage, 5000);
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastVoltageSample === 0) {
    VoltageSpinner.stop();
    $('#SpinnerDivVoltage').remove();
    $('#VoltagePanel').css('opacity', '1.0');
    $('#VoltagePanel').find("*").prop("disabled", false);    
  }
  if(result['Sampletime'][0] <= LastVoltageSample) return;
  for(i = 0; i < result['Sampletime'].length; i++) {
    VoltageData.push([new Date(result['Sampletime'][i]), result['rms_v'][i][0]]);
    while(VoltageData.length > result['MaxSamples'])   VoltageData.shift();
    LastVoltageSample = result['Sampletime'][i];
  }
  var UpperRange = -Infinity;
  var LowerRange =  Infinity;
  for (i = 0; i < VoltageData.length; i++) {
    if (VoltageData[i][1] > UpperRange) UpperRange = VoltageData[i][1];
    if (VoltageData[i][1] < LowerRange) LowerRange = VoltageData[i][1];
  }
  Round2Labels(VoltageData[VoltageData.length - 1][1], $('#labelVoltageInteger'), $('#labelVoltageDecimal'));
  var Variation = (UpperRange - LowerRange);
  if(Variation < 1) Variation = 1.0;
  UpperRange = UpperRange + Variation * 0.1;
  LowerRange = Math.max(0, LowerRange - Variation * 0.1);
  $('#labelVoltageAggregation').text('From ' + result['Aggregation_Source'] + ' aggregation');
  if(VoltageChart === null) {
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"),
      VoltageData,
      { strokeWidth: 1, 
        labelsDiv: "VoltageLegend",
        labels: ["Time", "Voltage"],
        stepPlot: true,
        drawGapEdgePoints: true,
        connectSeparatedPoints: false,
        fillGraph: false,
        colors: ["#d9534f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        animatedZooms: true,
        rollPeriod: 0,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        valueRange: [LowerRange, UpperRange],
        dateWindow: [LastVoltageSample - 7200000, LastVoltageSample],
        interactionModel: {},
        underlayCallback: function(canvas, area, g) {
          var NominalVoltage = Nodes['NominalVoltage'][Node];
          var YellowLowA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.1);
          var YellowHeightA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.07) - YellowLowA;
          var YellowLowB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.1);
          var YellowHeightB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.07) - YellowLowB;
          var RedLowA = g.toDomYCoord(1000);
          var RedHeightA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.10) - RedLowA;
          var RedLowB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.10);
          var RedHeightB = g.toDomYCoord(0) - RedLowB;
          canvas.fillStyle = "rgba(255, 255, 0, 0.15)";
          canvas.fillRect(0, YellowLowA, 10000, YellowHeightA);
          canvas.fillRect(0, YellowLowB, 10000, YellowHeightB);
          canvas.fillStyle = "rgba(180, 0, 0, 0.15)";
          canvas.fillRect(0, RedLowA, 10000, RedHeightA);
          canvas.fillRect(0, RedLowB, 10000, RedHeightB);
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">V</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -4px">' + x.toLocaleTimeString() + '</div>';
               }
             }
        }
      }
    );
  } else {
    VoltageChart.updateOptions({'file': VoltageData, valueRange: [LowerRange, UpperRange], dateWindow: [LastVoltageSample - 7200000, LastVoltageSample]});
  }
  AddEvents(result, "Voltage", VoltageChart);
}
$('#maximizeVoltage').click(function() {
  $('#GraphDialogTitle').text('Voltage');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#VoltageCol');
  DialogElement = $('#VoltageMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(VoltageChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    VoltageChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#btnExportVoltage').click(function() {
  var Data = [];
  var Filename = "Voltage_" + LastTimestamp + '.csv';
  Data.push([['Index', 'Voltage']]);
  Data = Data.concat(VoltageData);
  exportToCsv(Filename, Data);
  $(this).removeClass('active');
  $(this).blur();
});

/* Grafica de corriente */
var CurrentChart        = null;
var LastCurrentSample   = 0;
var CurrentData         = [];
var CurrentSpinner      = new Spinner(SpinnerOpts);
function AjaxGetCurrent() {
  if(LastCurrentSample === 0) {
    $('#CurrentPanel').find("*").prop("disabled", true);
    $('#CurrentPanel').css('opacity', '0.6');
    $('#CurrentPanel').append('<div id="SpinnerDivCurrent" style="position:absolute; top:0; left:0; width:100%; height:100%; z-index:2;"></div>');
    VoltageSpinner.spin(document.getElementById('SpinnerDivCurrent'));
  }
  $.ajax({ url: "getSeriesData",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 7200000, To: 0, Aggreg: "3s", LastSample: LastCurrentSample, Series: 'rms_i', Node: Nodes['IDs'][Node] }),
           success: AjaxCurrent
        });
}
function AjaxCurrent(result) {
  setTimeout(AjaxGetCurrent, 5000);
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastCurrentSample === 0) {
    CurrentSpinner.stop();
    $('#SpinnerDivCurrent').remove();
    $('#CurrentPanel').css('opacity', '1.0');
    $('#CurrentPanel').find("*").prop("disabled", false);    
  }
  if(result['Sampletime'][0] <= LastCurrentSample) return;
  for(i = 0; i < result['Sampletime'].length; i++) {
    CurrentData.push([new Date(result['Sampletime'][i]), result['rms_i'][i][0]]);
    while(CurrentData.length > result['MaxSamples']) CurrentData.shift();
    LastCurrentSample = result['Sampletime'][i];
  }
  var UpperRange = -1000000;
  var LowerRange = 1000000;
  var Avg = 0;
  for (i = 0; i < CurrentData.length; i++) {
    if (CurrentData[i][1] > UpperRange) UpperRange = CurrentData[i][1];
    if (CurrentData[i][1] < LowerRange) LowerRange = CurrentData[i][1];
    Avg += Math.pow(CurrentData[i][1], 2.0);
  }
  Round2Labels(CurrentData[CurrentData.length - 1][1], $('#current_now_integer'), $('#current_now_decimal'));
  var Variation = (UpperRange - LowerRange);
  if(Variation < 1) Variation = 1.0;
  UpperRange = UpperRange + Variation * 0.1;
  LowerRange = LowerRange - Variation * 0.1;
  $('#CurrentAggregation').text('From ' + result['Aggregation_Source'] + ' aggregation');
  if(CurrentChart == null) {
    CurrentChart = new Dygraph(document.getElementById("CurrentChart"),
      CurrentData,
      { strokeWidth: 1,
        labelsDiv: "CurrentLegend",
        labels: ["Time", "Current"],
        stepPlot: true,
        drawGapEdgePoints: true,
        connectSeparatedPoints: false,
        fillGraph: false,
        colors: ["#428bca"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        animatedZooms: true,
        rollPeriod: 0,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        valueRange: [LowerRange, UpperRange],
        dateWindow: [LastCurrentSample - 7200000, LastCurrentSample],
        interactionModel: {},
        underlayCallback: function(canvas, area, g) {
          var MaxCurrent = Nodes['MaxCurrent'][Node];
          var YellowLowA = g.toDomYCoord(MaxCurrent);
          var YellowHeightA = g.toDomYCoord(MaxCurrent - MaxCurrent * 0.10) - YellowLowA;
          var YellowLowB = g.toDomYCoord(-MaxCurrent + MaxCurrent * 0.10);
          var YellowHeightB = g.toDomYCoord(-MaxCurrent) - YellowLowB;
          var RedLowA = g.toDomYCoord(10000);
          var RedHeightA = g.toDomYCoord(MaxCurrent) - RedLowA;
          var RedLowB = g.toDomYCoord(-MaxCurrent);
          var RedHeightB = g.toDomYCoord(-10000) - RedLowB;
          canvas.fillStyle = "rgba(255, 255, 0, 0.15)";
          canvas.fillRect(0, YellowLowA, 10000, YellowHeightA);
          canvas.fillRect(0, YellowLowB, 10000, YellowHeightB);
          canvas.fillStyle = "rgba(180, 0, 0, 0.15)";
          canvas.fillRect(0, RedLowA, 10000, RedHeightA);
          canvas.fillRect(0, RedLowB, 10000, RedHeightB);
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">A</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -4px">' + x.toLocaleTimeString() + '</div>';
               }
             }
        }
      }
    );
  } else {
    CurrentChart.updateOptions({'file': CurrentData, valueRange: [LowerRange, UpperRange], dateWindow: [LastCurrentSample - 7200000, LastCurrentSample]});
  }
}

/* Grafica de potencia */
var PowerChart          = null;
var LastPowerSample     = 0;
var PowerData           = [];
var PowerChartUnits     = 'W';
var PowerSpinner        = new Spinner(SpinnerOpts);
function AjaxGetPower() {
  if(LastPowerSample === 0) {
    $('#PowerPanel').find("*").prop("disabled", true);
    $('#PowerPanel').css('opacity', '0.6');
    $('#PowerPanel').append('<div id="SpinnerDivPower" style="position:absolute; top:0; left:0; width:100%; height:100%; z-index:2;"></div>');
    PowerSpinner.spin(document.getElementById('SpinnerDivPower'));
  }
  $.ajax({ url: "getSeriesData",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 7200000, To: 0, Aggreg: "3s", LastSample: LastPowerSample, Series: 'active_power', Node: Nodes['IDs'][Node] }),
           success: AjaxPower
        });
}
function AjaxPower(result) {
  setTimeout(AjaxGetPower, 5000);
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastPowerSample === 0) {
    PowerSpinner.stop();
    $('#SpinnerDivPower').remove();
    $('#PowerPanel').css('opacity', '1.0');
    $('#PowerPanel').find("*").prop("disabled", false);    
  }
  if(result['Sampletime'][0] <= LastPowerSample) return;
  for(i = 0; i < result['Sampletime'].length; i++) {
    PowerData.push([new Date(result['Sampletime'][i]), result['active_power'][i][0]]);
    while(PowerData.length > result['MaxSamples']) PowerData.shift();
    LastPowerSample = result['Sampletime'][i];
  }
  var UpperRange = -1000000;
  var LowerRange = 1000000;
  for (i = 0; i < PowerData.length; i++) {
    if (PowerData[i][1] > UpperRange) UpperRange = PowerData[i][1];
    if (PowerData[i][1] < LowerRange) LowerRange = PowerData[i][1];
  }
  if(PowerData[PowerData.length - 1][1] > 1000) {
    Round2Labels(PowerData[PowerData.length - 1][1] / 1000.0, $('#power_now_integer'), $('#power_now_decimal'));
    $('#power_units').text('kW');
  } else {
    Round2Labels(PowerData[PowerData.length - 1][1], $('#power_now_integer'), $('#power_now_decimal'));
    $('#power_units').text('W');
  }
  var Variation = (UpperRange - LowerRange);
  if(Variation < 1) Variation = 1.0;
  UpperRange = UpperRange + Variation * 0.1;
  LowerRange = LowerRange - Variation * 0.1;
  var toShow = [];
  if((Math.abs(UpperRange) > 1000) || (Math.abs(LowerRange) > 1000)) {
    PowerChartUnits = 'kW';
    UpperRange = UpperRange / 1000;
    LowerRange = LowerRange / 1000;
    for(i = 0; i < PowerData.length; i++) toShow.push([PowerData[i][0], PowerData[i][1] / 1000.0]);
  } else {
    toShow = PowerData;
    PowerChartUnits = 'W';
  }
  $('#PowerAggregation').text('From ' + result['Aggregation_Source'] + ' aggregation');
  if(PowerChart == null) {
    PowerChart = new Dygraph(document.getElementById("power_chart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "PowerLegend",
        labels: ["Time", "Power"],
        stepPlot: true,
        drawGapEdgePoints: true,
        connectSeparatedPoints: false,
        fillGraph: false,
        colors: ["#f0ad4e"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        animatedZooms: true,
        rollPeriod: 0,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        valueRange: [LowerRange, UpperRange],
        dateWindow: [LastPowerSample - 7200000, LastPowerSample],
        interactionModel: {},
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">' + PowerChartUnits + '</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -4px">' + x.toLocaleTimeString() + '</div>';
               }
             }
        }
      }
    );
  } else {
    PowerChart.updateOptions({'file': toShow, valueRange: [LowerRange, UpperRange], dateWindow: [LastPowerSample - 7200000, LastPowerSample]});
  }
}

/* Grafica de frequencia */
var FrequencyChart      = null;
var LastFrequencySample = 0;
var FrequencyData       = [];
var FrequencySpinner    = new Spinner(SpinnerOpts);
function AjaxGetFrequency() {
  if(LastPowerSample === 0) {
    $('#FrequencyPanel').find("*").prop("disabled", true);
    $('#FrequencyPanel').css('opacity', '0.6');
    $('#FrequencyPanel').append('<div id="SpinnerDivFrequency" style="position:absolute; top:0; left:0; width:100%; height:100%; z-index:2;"></div>');
    PowerSpinner.spin(document.getElementById('SpinnerDivPower'));
  }
  $.ajax({ url: "getSeriesData",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 7200000, To: 0, Aggreg: "3s", LastSample: LastFrequencySample, Series: 'freq', Node: Nodes['IDs'][Node] }),
           success: AjaxFrequency
        });
}
function AjaxFrequency(result) {
  setTimeout(AjaxGetFrequency, 5000);
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastFrequencySample === 0) {
    FrequencySpinner.stop();
    $('#SpinnerDivFrequency').remove();
    $('#FrequencyPanel').css('opacity', '1.0');
    $('#FrequencyPanel').find("*").prop("disabled", false);    
  }
  if(result['Sampletime'][0] <= LastFrequencySample) return;
  for(i = 0; i < result['Sampletime'].length; i++) {
    FrequencyData.push([new Date(result['Sampletime'][i]), result['freq'][i]]);
    while(FrequencyData.length > result['MaxSamples']) FrequencyData.shift();
    LastFrequencySample = result['Sampletime'][i];
  }
  var UpperRange = -1000000;
  var LowerRange = 1000000;
  for (i = 0; i < FrequencyData.length; i++) {
    if (FrequencyData[i][1] > UpperRange) UpperRange = FrequencyData[i][1];
    if (FrequencyData[i][1] < LowerRange) LowerRange = FrequencyData[i][1];
  }
  Round2Labels(FrequencyData[FrequencyData.length - 1][1], $('#frequency_now_integer'), $('#frequency_now_decimal'));
  var Variation = (UpperRange - LowerRange);
  if(Variation < 0.5) Variation = 0.5;
  UpperRange = UpperRange + Variation * 0.1;
  LowerRange = LowerRange - Variation * 0.1;
  $('#FrequencyAggregation').text('From ' + result['Aggregation_Source'] + ' aggregation');
  if(FrequencyChart == null) {
    FrequencyChart = new Dygraph(document.getElementById("frequency_chart"),
      FrequencyData,
      { strokeWidth: 1,
        labelsDiv: "FrequencyLegend",
        labels: ["Time", "Frequency"],
        stepPlot: true,
        drawGapEdgePoints: true,
        connectSeparatedPoints: false,
        fillGraph: false,
        colors: ["#5cb85c"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        animatedZooms: true,
        rollPeriod: 0,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        valueRange: [LowerRange, UpperRange],
        dateWindow: [LastFrequencySample - 7200000, LastFrequencySample],
        interactionModel: {},
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">Hz</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -4px">' + x.toLocaleTimeString() + '</div>';
               }
             }
        }
      }
    );
    Dygraph.synchronize([VoltageChart, CurrentChart, PowerChart, FrequencyChart], { range: false, zoom: false } );
  } else {
    FrequencyChart.updateOptions({'file': FrequencyData, valueRange: [LowerRange, UpperRange], dateWindow: [LastFrequencySample - 7200000, LastFrequencySample]});
  }
}




var DialogElement           = null;
var PrevDialogContainer     = null;
var DialogResizeHandler     = null;
var DialogPrevResizeHandler = null;
var EnergyTimeScale     = 'Day';
var EnergyKeepNow       = true;
var LastEnergyTime      = 0;
var EnergyData          = [];
var EvolutionDatasets   = [[],[],[],[],[],[]];
var EvolutionCharts     = [];
var LastEvolutionSample = 0;



var EnergyChart         = null;

var EnergyChartUnits    = 'Wh';
var EnergyAggregation   = '10m';





var EnergySpinner       = new Spinner(SpinnerOpts);


var PowerSpinner        = new Spinner(SpinnerOpts);
var MainParamSpinner    = new Spinner(SpinnerOpts);

function AjaxSuccess5S(result) {
  if(result != null && Object.keys(result).length > 0) {
    if(result['sampletime'][0] <= LastTimestamp) return;
    for(i = 0; i < result['sampletime'].length; i++) {
//      VoltageData.push([new Date(result['sampletime'][i] * 1000), result['rms_v'][i][0]]);
//      CurrentData.push([new Date(result['sampletime'][i] * 1000), result['rms_i'][i][0]]);
//      PowerData.push([new Date(result['sampletime'][i] * 1000), result['active_power'][i][0]]);
//      FrequencyData.push([new Date(result['sampletime'][i] * 1000), result['freq'][i]]);
//      while(VoltageData.length > result['max_samples'])   VoltageData.shift();
//      while(CurrentData.length > result['max_samples'])   CurrentData.shift();
//      while(PowerData.length > result['max_samples'])     PowerData.shift();
//      while(FrequencyData.length > result['max_samples']) FrequencyData.shift();
//      LastTimestamp = result['sampletime'][i];
    }
//    UpdateVoltageChart(result);
//    UpdateCurrentChart(result);
//    UpdatePowerChart(result);
//    UpdateFrequencyChart(result);
    result.lastVoltage = VoltageData[VoltageData.length - 1][1];
    result.lastCurrent = CurrentData[CurrentData.length - 1][1];
    result.lastPower = PowerData[PowerData.length - 1][1];
    result.lastFrequency = FrequencyData[FrequencyData.length - 1][1];
  }
  UpdateHeader(result);
}





function AjaxSuccess60S(result) {
  var date = new Date(result['budget_lastcalc'] * 1000);
  $('#budget_lastcalc').text(date.toLocaleDateString() + ' - ' + date.toLocaleTimeString());
  if(EnergyTimeScale == 'Day') {
    $('#EnergyPeriod').text('Today');
    $('#BudgetPeriod').text('Today');
    $('#PrevEnergyPeriod').text('Yesterday');
  } else if(EnergyTimeScale == 'Week') {
    $('#EnergyPeriod').text('This week');
    $('#BudgetPeriod').text('This week');
    $('#BudgetPeriod').text('Previous week');
  } else if(EnergyTimeScale == 'Month') {
    $('#EnergyPeriod').text('This month');
    $('#BudgetPeriod').text('This Mont');
    $('#BudgetPeriod').text('Previous month');
  } else if(EnergyTimeScale == 'Year') {
    $('#EnergyPeriod').text('This year');
    $('#BudgetPeriod').text('This year');
    $('#BudgetPeriod').text('Previous year');
  }     
  Round2Labels(result['current_budget'], $('#budget_integer'), $('#budget_decimal'));
  if(Math.abs(result['current_energy']) < 1000.0) {
    Round2Labels(result['current_energy'], $('#energy_integer'), $('#energy_decimal'));
    $('#EnergyUnits').text('Wh');
  } else {
    Round2Labels(result['current_energy'] / 1000.0, $('#energy_integer'), $('#energy_decimal'));
    $('#EnergyUnits').text('kWh');
  }
  if(Math.abs(result['prev_energy']) < 1000.0) {
    $('#PrevEnergy').text(Round2(result['prev_energy']));
    $('#PrevEnergyUnits').text('Wh');
  } else {
    $('#PrevEnergy').text(Round2(result['prev_energy'] / 1000.0));
    $('#PrevEnergyUnits').text('kWh');
  }
  UpdateHeader(result);
}
function GetValues60SMainParameters() {
  if(LastEvolutionSample == 0) {
    $('#MainParamPanel').find("*").prop("disabled", true);
    $('#MainParamPanel').css('opacity', '0.6');
    $('#MainParamPanel').append('<div id="SpinnerDivMain" style="position:absolute; top:0; left:0; width:100%; height:100%; z-index:2;"></div>');
    MainParamSpinner.spin(document.getElementById('SpinnerDivMain'));
  }
  $.ajax({ url: "dashboard/getSeriesData",
         timeout: 20000,
         type: "POST",
         dataType: "json",
         contentType: 'application/json',
         data: JSON.stringify({ Period: 86400, To: 0, Aggreg: "1m", LastSample: LastEvolutionSample, Series: 'active_power, fundamental_reactive_power, rms_v, rms_i, power_factor, phi', Node: 0, IncludeFlag: false }),
         success: UpdateEvolutionChart,
         complete: function (x) {
                     MainParamSpinner.stop();
                     $('#SpinnerDivMain').remove();
                     $('#MainParamPanel').css('opacity', '1.0');
                     $('#MainParamPanel').find("*").prop("disabled", false);
                  }
      });
}
function GetValues60S() {
  $.ajax({ url: "dashboard/ajaxUpdateValues60S",
         timeout: 20000,
         type: "POST",
         dataType: "json",
         data: { energyperiod: EnergyTimeScale, lastevolutiontimestamp: LastEvolutionSample },
         success: AjaxSuccess60S,
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
    var Range = EnergyChart.getOption('dateWindow');
    period = (Range[1] - Range[0]) / 1000;
  }
  $.ajax({ url: "dashboard/getSeriesData",
    timeout: 20000,
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data: JSON.stringify({ Period: period, To: endDate, Aggreg: EnergyAggregation, LastSample: (AutoUpdate ? LastEnergyTime / 1000 : 0), Series: 'active_energy', Node: 0, IncludeFlag: true }),
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
  if(this.checked == false) return;
  var Range = EnergyChart.getOption('dateWindow');
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
  } else if(this.id == 'energynow') {
    EnergyKeepNow = !EnergyKeepNow;
    if(EnergyKeepNow) {
      $(this).addClass('active');
    } else {
      $(this).removeClass('active');
    }
    $(this).blur();
    if(EnergyKeepNow) EnergyGetData(Period, 0, true);
    return;
  }
  EnergyGetData(Period, EnergyKeepNow ? 0 : Range[1] / 1000, false);
  if(EnergyTimeScale != PrevScale) GetValues60S(true, false);
}
function AjaxEnergyUpdate(result) {
  if((result['update'] == true) && (EnergyKeepNow == false)) return;
  if(((EnergyAggregation == '1h') && (result['aggregation_source'] != '1 hour')) ||
     ((EnergyAggregation == '10m') && (result['aggregation_source'] != '10 minutes')) ||
     ((EnergyAggregation == '1m') && (result['aggregation_source'] != '1 minute')) ||
     ((EnergyAggregation == '3s') && (result['aggregation_source'] != '3 seconds'))) {
    if(result['update'] == true) return;
    $('#aggreg_modal').modal('show');
  }
  if(result['update'] == false) EnergyData = [];
  for(i = 0; i < result['sampletime'].length; i++) {
    LastEnergyTime = result['sampletime'][i] * 1000 + 1000;
    EnergyData.push([new Date(LastEnergyTime), result['active_energy'][i][0]]);
  }
  EnergyTickAggregationButtons(result['aggregation_source']);
  $('#EnergyAggregation').text('From ' + result['aggregation_source'] + ' aggregation');
  if(result['update'] == true) {
    while(EnergyData.length > result['max_samples']) EnergyData.shift();
  }
  var UpperRange = -1000000;
  var LowerRange =  1000000;
  for (i = 0; i < EnergyData.length; i++) {
    if(EnergyData[i][1] > UpperRange) UpperRange = EnergyData[i][1];
    if(EnergyData[i][1] < LowerRange) LowerRange = EnergyData[i][1];
  }
  var Variation = (UpperRange - LowerRange);
  if(Variation < 1) Variation = 1.0;
  if(UpperRange > 0) UpperRange = UpperRange + Variation * 0.1;
  if(LowerRange < 0) LowerRange = LowerRange - Variation * 0.1;
  var toShow = [];
  if((Math.abs(UpperRange) > 1000) || (Math.abs(LowerRange) > 1000)) {
    EnergyChartUnits = 'kWh';
    UpperRange = UpperRange / 1000;
    LowerRange = LowerRange / 1000;
    for(i = 0; i < EnergyData.length; i++) {
      toShow.push([EnergyData[i][0], EnergyData[i][1] / 1000.0]);
    }
  } else {
    EnergyChartUnits = 'Wh';
    for(i = 0; i < EnergyData.length; i++) {
      toShow.push([EnergyData[i][0], EnergyData[i][1]]);
    }
  }
  if(EnergyChart == null) {
    EnergyChart = new Dygraph(document.getElementById("EnergyChart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "EnergyLegend",
        labels: ["Time", "Energy"],
        plotter: barChartPlotterEnergy,
        stepPlot: true,
        fillGraph: true,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        valueRange: [LowerRange, UpperRange],
        dateWindow: [result['from'] * 1000 + 1000, result['to'] * 1000 + 1000],
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
                 return Round2(x) + ' <em class="x-small">' + EnergyChartUnits + '</em>';
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
    $(window).bind('resize', EnergyResizeEvent);
    $('#energy_day').addClass('active');
    $('#maximizeEnergy').removeClass('disabled');
    window.dispatchEvent(new Event('resize'));
  } else {
    EnergyChart.updateOptions({'file': toShow, valueRange: [LowerRange, UpperRange], dateWindow: [result['from'] * 1000 + 1000, result['to'] * 1000 + 1000]});
  }
  AddEvents(result, "Energy", EnergyChart);
}
function EnergyResizeEvent() {
  var Height = $('#EnergyBox').height() + $('#BudgetBox').height();
  if(EnergyChart != null) EnergyChart.resize($('#EnergyCol').width(), Height - 12);
}
function UpdateEvolutionChart(result) {
  for(j = 0; j < result['sampletime'].length; j++) {
    EvolutionDatasets[0].push([new Date(result['sampletime'][j] * 1000 + 1000), result.active_power[j]]);
    EvolutionDatasets[1].push([new Date(result['sampletime'][j] * 1000 + 1000), result.fundamental_reactive_power[j]]);
    EvolutionDatasets[2].push([new Date(result['sampletime'][j] * 1000 + 1000), result.rms_v[j]]);
    EvolutionDatasets[3].push([new Date(result['sampletime'][j] * 1000 + 1000), result.rms_i[j]]);
    EvolutionDatasets[4].push([new Date(result['sampletime'][j] * 1000 + 1000), result.power_factor[j]]);
    EvolutionDatasets[5].push([new Date(result['sampletime'][j] * 1000 + 1000), result.phi[j]]);
    LastEvolutionSample = result['sampletime'][j];
  }
  for(i = 0; i < 6; i++) {
    while(EvolutionDatasets[i].length > result['max_samples']) EvolutionDatasets[i].shift();
  }
  var SerieNames = ['Active power', 'Reactive power', 'Voltage', 'Current', 'Power factor', 'Phi'];
  if(EvolutionCharts.length == 0) {
    for(i = 0; i < 6; i++) {
      var Str = '<div style="position: relative">' +
                '  <div id="EvolutionChart' + i + '" class="chart" style="height: 50px' + ((i == 0) ? '; margin-top: 16px; border-top: 1px solid #999"' : '"') + '></div>' +       
                '  <div style="pointer-events: none; position: absolute; top: -2px; text-align: left; left: 2px" id="EvolutionChartLegend' + i + '"></div>' +                    
                '</div>';
      $("#EvolutionPanel").append(Str);
      EvolutionCharts[i] = new Dygraph(document.getElementById("EvolutionChart" + i),
        EvolutionDatasets[i],
        { strokeWidth: 0.5,
          labels: ["Time", SerieNames[i]],
          colors: ["#d9534f"],
          gridLineColor: "#999",
          gridLinePattern: [4, 4],
          highlightCircleSize: 0,
          interactionModel: { },
          labelsDiv: 'EvolutionChartLegend' + i,
          legend: 'always',
          legendFormatter: function(data) {
            if(data.x == null) {
              return '<span style="font-size: 18px; font-weight: bold">' + data.series[0].label + '</span>';
            } else {
              return '<span style="font-size: 15px; font-weight: bold">' + data.series[0].label + '</span><br/>' +
                      data.xHTML + data.series[0].yHTML;
            }
          },
          xAxisHeight: 0.1,
          xLabelHeight: 1,
          plotter: barChartPlotterCubism,
          dateWindow: [(LastEvolutionSample * 1000) - 86400000, LastEvolutionSample * 1000],
          plugins: [
            new Dygraph.Plugins.Crosshair({
              direction: "vertical"
            })
          ],
          axes: {
            y: { drawAxis: false,
                 drawGrid:false,
                 valueFormatter: function (x) {
                   var Units;
                   if(this.attributes_.labels_[0] == SerieNames[0]) {
                     if(x > 1000) {
                       Units = 'kW';
                       x = x / 1000;
                     } else {
                       Units = 'W';
                     }
                   } else if(this.attributes_.labels_[0] == SerieNames[1]) {
                     if(x > 1000) {
                       Units = 'kVAr';
                       x = x / 1000;
                     } else {
                       Units = 'VAr';
                     }
                   } else if(this.attributes_.labels_[0] == SerieNames[2]) {
                     Units = 'V';
                   } else if(this.attributes_.labels_[0] == SerieNames[3]) {
                     Units = 'A';  
                   } else if(this.attributes_.labels_[0] == SerieNames[4]) {
                     Units = '';
                   } else if(this.attributes_.labels_[0] == SerieNames[5]) {
                     if(x < 0) {
                       Units = 'º CAP';
                       x = -x;
                     } else if(x > 0) {
                       Units = 'º IND';
                     } else {
                       Units = 'º';
                     }
                   }
                   return '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + Round2(x) + '<em>' + Units + '</em></div>';
                 }
               },
            x: { valueFormatter: function (x) {
                   var date = new Date(x);
                   return '<div style="font-size: 10px; line-height: 10px">' + date.toLocaleTimeString() + '</div>';
                 },
                 axisLabelFormatter: function(x, i) {
                   if(this.attributes_.labels_[0] == SerieNames[0]) {
                     var date = new Date(x);
                     if((date.getHours() == 0) && (date.getMinutes() == 0) && (date.getSeconds() == 0)) {
                       return '<div style="font-size: 10px; margin-top: -63px; line-height: 10px; font-weight: bold">' + date.toLocaleDateString() + '</div>';
                     } else {
                       return '<div style="font-size: 10px; margin-top: -63px; line-height: 10px">' + date.toLocaleTimeString() + '</div>';                       
                     }
                   } else {
                     return null;
                   }
                 }
            }
          }
        }
      );
    }
    Dygraph.synchronize(EvolutionCharts, { range: false, zoom: false });
  } else {
    for(i = 0; i < 6; i++) {
      EvolutionCharts[i].updateOptions({ file: EvolutionDatasets[i],dateWindow: [(LastEvolutionSample * 1000) - 86400000, LastEvolutionSample * 1000] });
    }
  }
}
$('#aggreg_modal_close').click(function() {
  $('#aggreg_modal').modal('hide');
});
$(':input[name="energy_chart_range"]').change(EnergyButtonsClick);
$('#energynow').click(EnergyButtonsClick);
$('#maximizeEnergy').click(function() {
  $('#GraphDialogTitle').text('Energy');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#EnergyCol');
  DialogElement = $('#EnergyMaximizeDiv').detach();
  DialogPrevResizeHandler = EnergyResizeEvent;
  DialogResizeHandler = function() {
    EnergyChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
  };
  $(window).unbind('resize', EnergyResizeEvent);
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});

$('#maximizeCurrent').click(function() {
  $('#GraphDialogTitle').text('Current');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#CurrentCol');
  DialogElement = $('#CurrentMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(CurrentChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    CurrentChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#maximizePower').click(function() {
  $('#GraphDialogTitle').text('Active power');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#PowerCol');
  DialogElement = $('#PowerMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(PowerChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    PowerChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#maximizeFrequency').click(function() {
  $('#GraphDialogTitle').text('Frequency');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#FrequencyCol');
  DialogElement = $('#FrequencyMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(FrequencyChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    FrequencyChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
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
$('#btnExportEnergy').click(function() {
  var Data = [];
  var Filename = "Energy_" + LastEnergyTime + '.csv';
  Data.push([['Index', 'Energy']]);
  Data = Data.concat(EnergyData);
  exportToCsv(Filename, Data);
  $(this).blur();
});


function UpdatePage() {
  AjaxGetVoltage();
  AjaxGetCurrent();
  AjaxGetPower();
  AjaxGetFrequency();

$(document).ajaxError(function (event, jqxhr, settings, exception) {
//  if(jqxhr.status === 200) window.location.href = 'login';
});
$(document).ready(EnergyGetData(86400, 0, true));


GetValues60S();
$(document).ready(GetValues60S, 60000);
GetValues60SMainParameters();
$(document).ready(setInterval(GetValues60SMainParameters, 60000));
}