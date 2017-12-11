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
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 7200000, To: 0, Aggreg: "3s", LastSample: LastVoltageSample, Series: 'rms_v, flag', Node: Nodes['IDs'][Node] }),
           success: AjaxVoltage,
           complete: function() { setTimeout(AjaxGetVoltage, 5000); }
        });
}
function AjaxVoltage(result) {
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastVoltageSample === 0) {
    VoltageSpinner.stop();
    $('#SpinnerDivVoltage').remove();
    $('#VoltagePanel').css('opacity', '1.0');
    $('#VoltagePanel').find("*").prop("disabled", false);    
  }
  if(result['Sampletime'][0] <= LastVoltageSample) return;
  for(i = 0; i < result['Sampletime'].length; i++) VoltageData.push([new Date(result['Sampletime'][i]), result['rms_v'][i][0]]);
  while(VoltageData.length > result['MaxSamples']) VoltageData.shift();
  LastVoltageSample = result['Sampletime'][result['Sampletime'].length - 1];
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
  $('#labelVoltageAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
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
        dateWindow: [result['From'], result['To']],
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
    VoltageChart.updateOptions({'file': VoltageData, valueRange: [LowerRange, UpperRange], dateWindow: [result['From'], result['To']]});
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
  var Filename = "Voltage_" + Nodes['IDs'][Node] + '_' + LastVoltageSample + '.csv';
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
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 7200000, To: 0, Aggreg: "3s", LastSample: LastCurrentSample, Series: 'rms_i', Node: Nodes['IDs'][Node] }),
           success: AjaxCurrent,
           complete: function() { setTimeout(AjaxGetCurrent, 5000); }
        });
}
function AjaxCurrent(result) {
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastCurrentSample === 0) {
    CurrentSpinner.stop();
    $('#SpinnerDivCurrent').remove();
    $('#CurrentPanel').css('opacity', '1.0');
    $('#CurrentPanel').find("*").prop("disabled", false);    
  }
  if(result['Sampletime'][0] <= LastCurrentSample) return;
  for(i = 0; i < result['Sampletime'].length; i++) CurrentData.push([new Date(result['Sampletime'][i]), result['rms_i'][i][0]]);
  while(CurrentData.length > result['MaxSamples']) CurrentData.shift();
  LastCurrentSample = result['Sampletime'][result['Sampletime'].length - 1];
  var UpperRange = -Infinity;
  var LowerRange = Infinity;
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
  $('#CurrentAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
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
        dateWindow: [result['From'], result['To']],
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
    CurrentChart.updateOptions({'file': CurrentData, valueRange: [LowerRange, UpperRange], dateWindow: [result['From'], result['To']]});
  }
}
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
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 7200000, To: 0, Aggreg: "3s", LastSample: LastPowerSample, Series: 'active_power', Node: Nodes['IDs'][Node] }),
           success: AjaxPower,
           complete: function() { setTimeout(AjaxGetPower, 5000); }
        });
}
function AjaxPower(result) {
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastPowerSample === 0) {
    PowerSpinner.stop();
    $('#SpinnerDivPower').remove();
    $('#PowerPanel').css('opacity', '1.0');
    $('#PowerPanel').find("*").prop("disabled", false);    
  }
  if(result['Sampletime'][0] <= LastPowerSample) return;
  for(i = 0; i < result['Sampletime'].length; i++) PowerData.push([new Date(result['Sampletime'][i]), result['active_power'][i][0]]);
  while(PowerData.length > result['MaxSamples']) PowerData.shift();
  LastPowerSample = result['Sampletime'][result['Sampletime'].length - 1];
  var UpperRange = -Infinity;
  var LowerRange = Infinity;
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
  $('#PowerAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
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
        dateWindow: [result['From'], result['To']],
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
    PowerChart.updateOptions({'file': toShow, valueRange: [LowerRange, UpperRange], dateWindow: [result['From'], result['To']]});
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
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 7200000, To: 0, Aggreg: "3s", LastSample: LastFrequencySample, Series: 'freq', Node: Nodes['IDs'][Node] }),
           success: AjaxFrequency,
           complete: function() { setTimeout(AjaxGetFrequency, 5000); }
        });
}
function AjaxFrequency(result) {
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastFrequencySample === 0) {
    FrequencySpinner.stop();
    $('#SpinnerDivFrequency').remove();
    $('#FrequencyPanel').css('opacity', '1.0');
    $('#FrequencyPanel').find("*").prop("disabled", false);    
  }
  if(result['Sampletime'][0] <= LastFrequencySample) return;
  for(i = 0; i < result['Sampletime'].length; i++) FrequencyData.push([new Date(result['Sampletime'][i]), result['freq'][i]]);
  while(FrequencyData.length > result['MaxSamples']) FrequencyData.shift();
  LastFrequencySample = result['Sampletime'][result['Sampletime'].length - 1];
  var UpperRange = -Infinity;
  var LowerRange = Infinity;
  for (i = 0; i < FrequencyData.length; i++) {
    if (FrequencyData[i][1] > UpperRange) UpperRange = FrequencyData[i][1];
    if (FrequencyData[i][1] < LowerRange) LowerRange = FrequencyData[i][1];
  }
  Round2Labels(FrequencyData[FrequencyData.length - 1][1], $('#frequency_now_integer'), $('#frequency_now_decimal'));
  var Variation = (UpperRange - LowerRange);
  if(Variation < 0.5) Variation = 0.5;
  UpperRange = UpperRange + Variation * 0.1;
  LowerRange = LowerRange - Variation * 0.1;
  $('#FrequencyAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
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
        dateWindow: [result['From'], result['To']],
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
  } else {
    FrequencyChart.updateOptions({'file': FrequencyData, valueRange: [LowerRange, UpperRange], dateWindow: [result['From'], result['To']]});
  }
  if((VoltageChart != null) && (CurrentChart != null) && (PowerChart != null) && (FrequencyChart != null)) Dygraph.synchronize([VoltageChart, CurrentChart, PowerChart, FrequencyChart], { range: false, zoom: false } );
}

/* Grafico de energia */
var LastEnergySample    = 0;
var EnergyData          = [];
var EnergyChart         = null;
var EnergyChartUnits    = 'Wh';
var EnergySpinner       = new Spinner(SpinnerOpts);
var EnergyTimer         = null;
function AjaxGetEnergy() {
  if(EnergyTimer !== null) clearInterval(EnergyTimer);
  $('#btnEnergyNow').addClass('disabled');
  if(LastEnergySample === 0) ShowSpinner(EnergySpinner, 'EnergySpinner');
  var Period = parseInt(Cookies.get('GraphPeriod'));
  if(!Period) {
    Period = 86400000;
    Cookies.set('GraphPeriod', Period);
  }
  var GraphEnd = parseInt(Cookies.get('GraphEnd'));
  if(!GraphEnd) {
    GraphEnd = 0;
    Cookies.set('GraphEnd', 0);
  } 
  var GraphAggregation = Cookies.get('GraphAggregation');
  if(!GraphAggregation || (GetAggregText(GraphAggregation) === false)) {
    GraphAggregation = '10m';
    Cookies.set('GraphAggregation', GraphAggregation);
  }
  $.ajax({ url: "getSeries",
    timeout: 20000,
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data: JSON.stringify({ Period: Period, To: GraphEnd, Aggreg: GraphAggregation, LastSample: LastEnergySample, Series: 'active_energy', Node: Nodes['IDs'][Node]}),
    success: AjaxEnergy,
    complete: function() { 
      $('#btnEnergyNow').removeClass('disabled');
      if(parseInt(Cookies.get('GraphEnd')) === 0) EnergyTimer = setTimeout(AjaxGetEnergy, 3000);
    }
 });
}
function AjaxEnergy(result) {
  if(LastEnergySample === 0) {
    StopSpinner(EnergySpinner, 'EnergySpinner');
    EnergyData = [];
  }
  if(result['Sampletime'] !== null) {
    for(i = 0; i < result['Sampletime'].length; i++) EnergyData.push([new Date(result['Sampletime'][i]), result['active_energy'][i][0]]);
    while(EnergyData.length > result['MaxSamples']) EnergyData.shift();
    LastEnergySample = result['Sampletime'][result['Sampletime'].length - 1];
  }
  $('#EnergyAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');  
  var UpperRange = -Infinity;
  var LowerRange =  Infinity;
  for (i = 0; i < EnergyData.length; i++) {
    if(EnergyData[i][1] > UpperRange) UpperRange = EnergyData[i][1];
    if(EnergyData[i][1] < LowerRange) LowerRange = EnergyData[i][1];
  }
  var Variation = (UpperRange - LowerRange);
  if(Variation < 1) Variation = 1.0;
  UpperRange = UpperRange + Variation * 0.1;
  LowerRange = LowerRange - Variation * 0.1;
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
        dateWindow: [result['From'], result['To']],
        zoomCallback: function(minDate, maxDate, yRanges) {
          clearInterval(EnergyTimer);
          $('#btnEnergyNow').removeClass('active');
          Cookies.set('GraphEnd', maxDate);
          Cookies.set('GraphPeriod', maxDate - minDate);
          LastEnergySample = 0;
          AjaxGetEnergy();
        },
        interactionModel: {
          mousedown: mouseDownOZM,
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              clearInterval(EnergyTimer);
              $('#btnEnergyNow').removeClass('active');
              Cookies.set('GraphEnd', g.dateWindow_[1]);
              Cookies.set('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastEnergySample = 0;
              AjaxGetEnergy();
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            clearInterval(EnergyTimer);
            $('#btnEnergyNow').removeClass('active');
            Cookies.set('GraphEnd', g.dateWindow_[1]);
            Cookies.set('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
            LastEnergySample = 0;
            AjaxGetEnergy();
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
    window.dispatchEvent(new Event('resize'));
  } else {
    EnergyChart.updateOptions({'file': toShow, valueRange: [LowerRange, UpperRange], dateWindow: [result['From'], result['To']]});
  }
  $('#btnShowEnergyYear').removeClass('active');
  $('#btnShowEnergyMonth').removeClass('active');
  $('#btnShowEnergyWeek').removeClass('active');
  $('#btnShowEnergyDay').removeClass('active');
  if((result['To'] - result['From']) === 86400000) {
    $('#btnShowEnergyDay').addClass('active');
  } else if((result['To'] - result['From']) === 604800000) {
    $('#btnShowEnergyWeek').addClass('active');
  } else if((result['To'] - result['From']) === 2592000000) {
    $('#btnShowEnergyMonth').addClass('active');
  } else if((result['To'] - result['From']) === 31536000000) {
    $('#btnShowEnergyYear').addClass('active');
  }
  var GraphAggregation = Cookies.get('GraphAggregation');
  if(GraphAggregation !== result['Aggregation_Source']) {
    $('#aggreg_modal').modal('show');
    GraphAggregation = result['Aggregation_Source'];
  }
  $('#btnEnergy3S').removeClass('active');
  $('#btnEnergy1M').removeClass('active');
  $('#btnEnergy10M').removeClass('active');
  $('#btnEnergy1H').removeClass('active');
  if(GraphAggregation == '3s') {
    $('#btnEnergy3S').addClass('active');
  } else if(GraphAggregation == '1m') {
    $('#btnEnergy1M').addClass('active');
  } else if(GraphAggregation == '10m') {
    $('#btnEnergy10M').addClass('active');
  } else if(GraphAggregation == '1h') {
    $('#btnEnergy1H').addClass('active');
  }
  var GraphEnd = parseInt(Cookies.get('GraphEnd'));
  if(GraphEnd === 0) {
    $('#btnEnergyNow').addClass('active');
  } else {
    $('#btnEnergyNow').removeClass('active');
  }
  Cookies.set('GraphAggregation', GraphAggregation);
}
function EnergyResizeEvent() {
  var Height = $('#EnergyBox').height() + $('#BudgetBox').height();
  if(EnergyChart != null) EnergyChart.resize($('#EnergyCol').width(), Height - 12);
}
$('#btnEnergy3S').click(function() {
  Cookies.set('GraphAggregation', '3s');
  LastEnergySample = 0;
  AjaxGetEnergy();
});
$('#btnEnergy1M').click(function() {
  Cookies.set('GraphAggregation', '1m');
  LastEnergySample = 0;
  AjaxGetEnergy();
});
$('#btnEnergy10M').click(function() {
  Cookies.set('GraphAggregation', '10m');
  LastEnergySample = 0;
  AjaxGetEnergy();
});
$('#btnEnergy1H').click(function() {
  Cookies.set('GraphAggregation', '1h');
  LastEnergySample = 0;
  AjaxGetEnergy();
});
$('#btnShowEnergyYear').click(function() {
  Cookies.set('GraphPeriod', 31536000000);
  LastEnergySample = 0;
  AjaxGetEnergy();
  Cookies.set('CountersPeriod', 'Year');
  AjaxGetEnergyCounters();
});
$('#btnShowEnergyMonth').click(function() {
  Cookies.set('GraphPeriod', 2592000000);
  LastEnergySample = 0;
  AjaxGetEnergy();
  Cookies.set('CountersPeriod', 'Month');
  AjaxGetEnergyCounters();
});
$('#btnShowEnergyWeek').click(function() {
  Cookies.set('GraphPeriod', 604800000);
  LastEnergySample = 0;
  AjaxGetEnergy();
  Cookies.set('CountersPeriod', 'Week');
  AjaxGetEnergyCounters();
});
$('#btnShowEnergyDay').click(function() {
  Cookies.set('GraphPeriod', 86400000);
  LastEnergySample = 0;
  AjaxGetEnergy();
  Cookies.set('CountersPeriod', 'Day');
  AjaxGetEnergyCounters();
});
$('#btnEnergyNow').click(function() {
  var End = parseInt(Cookies.get('GraphEnd'));
  clearInterval(EnergyTimer);
  if(End === 0) {
    $('#btnEnergyNow').removeClass('active');
    Cookies.set('GraphEnd', EnergyChart.getOption('dateWindow')[1]);
  } else {
    $('#btnEnergyNow').addClass('active');
    Cookies.set('GraphEnd', 0);
    LastEnergySample = 0;
    AjaxGetEnergy();
  }
});
$('#btnMaximizeEnergy').click(function() {
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
$('#btnExportEnergy').click(function() {
  var Data = [];
  var Filename = "Energy_" + LastEnergyTime + '.csv';
  Data.push([['Index', 'Energy']]);
  Data = Data.concat(EnergyData);
  exportToCsv(Filename, Data);
  $(this).blur();
});

/* Contadores de consumo */
var EnergyCountersTimer  = null;
function AjaxGetEnergyCounters() {
  if(EnergyCountersTimer !== null) clearInterval(EnergyCountersTimer);
  EnergyCountersTimer = setTimeout(AjaxGetEnergyCounters, 60000);
  var CountersPeriod = Cookies.get('CountersPeriod');
  var Current, Period;
  if(CountersPeriod === 'Week') {
    Period = 86400000 * 7;
    Current = GetWeekTimestamp();
    Aggreg = '10m';
  } else if(CountersPeriod === 'Month') {
    Period = 86400000 * 30;
    Current = GetMonthTimestamp();
    Aggreg = '1h';
  } else if(CountersPeriod === 'Year') {
    Period = 86400000 * 365;
    Current = GetYearTimestamp();
    Aggreg = '1h';
  } else {
    CountersPeriod = 'Day';
    Cookies.set('CountersPeriod', CountersPeriod);
    Period = 86400000;
    Current = GetDayTimestamp();
    Aggreg = '1m';
  }
  $.ajax({ url: "getSeriesAggreg",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: "sum(active_energy)", Aggreg: Aggreg, Period: Period, To: Current + Period }),
           success: EnergyUpdateCountersCurrent
  });
  $.ajax({ url: "getSeriesAggreg",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: "sum(active_energy)", Aggreg: Aggreg, Period: Period, To: Current }),
           success: EnergyUpdateCountersPrevs
  });
}
  
  

  
  
  //Voy a obtener varios tipos de datos:
  //Si
//  $.ajax({ url: "dashboard/ajaxUpdateValues60S",
//           timeout: 20000,
//           type: "POST",
//           dataType: "json",
//           data: { energyperiod: EnergyTimeScale, lastevolutiontimestamp: LastEvolutionSample },
//         success: AjaxSuccess60S,
//      });
}
function EnergyUpdateCounters() {
  EnergyCountersTimer = setTimeout(AjaxGetEnergyCounters, 60000);
  var CountersPeriod = Cookies.get('CountersPeriod');
  if(CountersPeriod === 'Week') {
    $('#labelEnergyPeriod').text('This week');
    $('#labelBudgetPeriod').text('This week');
    $('#labelBudgetPeriod').text('Previous week');    
  } else if(CountersPeriod === 'Month') {
    $('#labelEnergyPeriod').text('This month');
    $('#labelBudgetPeriod').text('This Mont');
    $('#labelBudgetPeriod').text('Previous month');
  } else if(CountersPeriod === 'Year') {
    $('#labelEnergyPeriod').text('This year');
    $('#labelBudgetPeriod').text('This year');
    $('#labelBudgetPeriod').text('Previous year');
  } else if(CountersPeriod === 'Day') {
    $('#labelEnergyPeriod').text('Today');
    $('#labelBudgetPeriod').text('Today');
    $('#labelPrevEnergyPeriod').text('Yesterday');
  }
    var date = new Date(result['budget_lastcalc'] * 1000);
  $('#budget_lastcalc').text(date.toLocaleDateString() + ' - ' + date.toLocaleTimeString());
  
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
  
  
  
  
}

/* Cuadro Main parameters */
var EvolutionDatasets   = [[],[],[],[],[],[]];
var EvolutionCharts     = [];
var LastEvolutionSample = 0;
function AjaxGetMainParameters() {
  if(LastEvolutionSample == 0) {
    $('#MainParamPanel').find("*").prop("disabled", true);
    $('#MainParamPanel').css('opacity', '0.6');
    $('#MainParamPanel').append('<div id="SpinnerDivMain" style="position:absolute; top:0; left:0; width:100%; height:100%; z-index:2;"></div>');
    MainParamSpinner.spin(document.getElementById('SpinnerDivMain'));
  }
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 86400000, To: 0, Aggreg: "1m", LastSample: LastEvolutionSample, Series: 'active_power, fundamental_reactive_power, rms_v, rms_i, power_factor, phi', Node: Nodes['IDs'][Node] }),
           success: AjaxMainParameters
      });
}
function AjaxMainParameters(result) {
  setTimeout(AjaxGetMainParameters, 60000);
  if(LastEvolutionSample === 0) {
    MainParamSpinner.stop();
    $('#SpinnerDivMain').remove();
    $('#MainParamPanel').css('opacity', '1.0');
    $('#MainParamPanel').find("*").prop("disabled", false);
  }
  for(j = 0; j < result['Sampletime'].length; j++) {
    EvolutionDatasets[0].push([new Date(result['Sampletime'][j]), result.active_power[j][0]]);
    EvolutionDatasets[1].push([new Date(result['Sampletime'][j]), result.fundamental_reactive_power[j][0]]);
    EvolutionDatasets[2].push([new Date(result['Sampletime'][j]), result.rms_v[j][0]]);
    EvolutionDatasets[3].push([new Date(result['Sampletime'][j]), result.rms_i[j][0]]);
    EvolutionDatasets[4].push([new Date(result['Sampletime'][j]), result.power_factor[j][0]]);
    EvolutionDatasets[5].push([new Date(result['Sampletime'][j]), result.phi[j][0]]);
    LastEvolutionSample = result['Sampletime'][j];
  }
  for(i = 0; i < 6; i++) {
    while(EvolutionDatasets[i].length > result['Max_Samples']) EvolutionDatasets[i].shift();
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
          dateWindow: [result['From'], result['To']],
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
      EvolutionCharts[i].updateOptions({ file: EvolutionDatasets[i],dateWindow: [result['From'], result['To']] });
    }
  }
}







var DialogElement           = null;
var PrevDialogContainer     = null;
var DialogResizeHandler     = null;
var DialogPrevResizeHandler = null;







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

  UpdateHeader(result);
}
$('#aggreg_modal_close').click(function() {
  $('#aggreg_modal').modal('hide');
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



function UpdatePage() {
  AjaxGetEnergy();
  AjaxGetEnergyCounters();
  AjaxGetVoltage();
  AjaxGetCurrent();
  AjaxGetPower();
  AjaxGetFrequency();
  AjaxGetMainParameters();

//$(document).ready(EnergyGetData(86400, 0, true));


//GetValues60S();
//$(document).ready(GetValues60S, 60000);
//GetValues60SMainParameters();
//$(document).ready(setInterval(GetValues60SMainParameters, 60000));
}