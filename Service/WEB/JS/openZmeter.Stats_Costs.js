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
var CostData = [], CostChart = null;
function Scrollable_OnSuccess(result, clear) {
  if(clear) CostData = [];
  if(result['Sampletime'] !== null && result['Sampletime'].length > 0) {
    if(CostData.length === 0 || CostData[CostData.length - 1][0] < result['Sampletime'][0]) {
      for(var i = 0; i < result['Sampletime'].length; i++) CostData.push([new Date(result['Sampletime'][i]), result['active_energy'][i], result['price'][i], result['active_energy'][i] * result['price'][i] / 1000.0]);
      ShiftArray(CostData, result['MaxSamples']);
    }
  }
  if(CostChart == null) {
    CostChart = new Dygraph(document.getElementById("CostChart"), CostData,
      { strokeWidth: 0.5, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], dateWindow: [result['From'], result['To']], includeZero: true, plotter: barChartLineNoSpacedPlotter,
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time', 'Consumption', 'Price', 'Cost'],
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: [$('#CostPanel').css('border-color'), '#d9534f'], plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        underlayCallback: function(context, area, dygraph) { GraphHightlightWeekends("#EEE", context, area, dygraph); },
        series: {'Price': { axis: 'y2' }, 'Cost': { strokeWidth: 0 } }, xAxisHeight: 20,
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Units = GetBestUnits(x); return Round2(ScaleToUnits(x, Units)) + ' <em class="x-small">' + Units + 'Wh</em>'; },
               axisLabelFormatter: function (x) {var Units = GetBestUnits(x); return Round2(ScaleToUnits(x, Units)); }
             },
          y2: { pixelsPerLabel: 15, axisLabelWidth: 26,
                valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">€/kWh</em>'; },
                axisLabelFormatter: function (x) { return Round2(x); }
              },
          x: MainScrollable.XAxis
          }
        }
    );
    $(window).bind('resize', CostResizeEvent);
    window.dispatchEvent(new Event('resize'));
    new GraphTools('Energy costs', CostChart, '#toolbarCost', CostResizeEvent, function() { exportToCsv('Costs.csv', [['Time', 'Consumption', 'Price', 'Cost']].concat(CostData)); });
  } else {
    CostChart.updateOptions({file: CostData, dateWindow: [result['From'], result['To']] });
  }
}
function CostResizeEvent() {
  if(CostChart !== null) CostChart.resize('100%', $('#CostPanel > .panel-body > .row > div:first-child').height());
}
/* Costs counters */
function Scrollable_UpdateCounters(times, renew) {
  if(renew === true) {
    $('#labelPricePrimary').text(times.PrimaryText + ' average kWh price');
    $('#labelEnergyPrimary').text(times.PrimaryText + ' energy');
    $('#labelBudgetPrimary').text(times.PrimaryText + ' budget');
    $('#labelPricePrimaryValue, #labelEnergyPrimaryValue, #labelBudgetPrimaryValue').text('Calculating...');
    $('#labelPricePrimaryUnits #labelEnergyPrimaryUnits, #labelBudgetUnits').text('');
    $('#labelPriceSecondary, #labelEnergySecondary, #labelBudgetSecondary').text(times.SecondaryText);
    $('#labelPriceSecondaryValue, #labelEnergySecondaryValue, #labelBudgetSecondaryValue').text((times.SecondaryTo !== undefined) ? 'Calculating...' : '');
    if(times.SecondaryTo !== undefined) {
      Ajax.GetSeriesAggreg(times.SecondaryPeriod, times.SecondaryTo, times.PreferedAggreg, 'sum(active_energy)', 
        function(result) {
          if(result['Samples'] !== 0) {
            var Sum = ArraySum(result['sum_active_energy']);
            var Units = GetBestUnits(Sum);
            var Text = Round2(ScaleToUnits(Sum, Units)) + '&nbsp;' + Units + 'Wh';
            if(result['sum_active_energy'].length > 1) Text += '(' + Array_Round2(ScaleArrayToUnits(result['sum_active_energy'], Units), '&nbsp;&nbsp;') + ')';
            $('#labelEnergySecondaryValue').html(Text);
          } else {
            $('#labelEnergySecondaryValue').text('No data');
          }
          window.dispatchEvent(new Event('resize'));
        }
      );
      Ajax.GetPrices(times.SecondaryPeriod, times.SecondaryTo, 0,
        function(result) {
          if(result['Sampletime'].length !== 0) {
            $('#labelBudgetUnits').text('€');
            var Budget = 0.0;
            for(var i = 0; i < result['Sampletime'].length; i++) Budget += result['active_energy'][i] * result['price'][i] / 1000.0;
            $('#labelBudgetSecondaryValue').html(Round2(Budget));
          } else {
            $('#labelBudgetSecondaryValue').text('No data');
          }
        }
      );
    }
  }
  Ajax.GetSeriesAggreg(times.PrimaryPeriod, times.PrimaryTo, times.PreferedAggreg, 'sum(active_energy)', 
    function(result) {
      var Sum = ArraySum(result['sum_active_energy']);
      var Units = GetBestUnits(Sum);
      $('#labelEnergyPrimaryValue').html(Round2(ScaleToUnits(Sum, Units)))
      $('#labelEnergyPrimaryUnits').text(Units + 'Wh');
      if(result['sum_active_energy'].length > 1) $('#labelEnergyPrimaryPhases').html('By phases:&nbsp;&nbsp;' + Array_Round2(ScaleArrayToUnits(result['sum_active_energy'], Units), '&nbsp;&nbsp;') + '&nbsp;&nbsp;' + Units + 'Wh');
      window.dispatchEvent(new Event('resize'));
    }
  );
  Ajax.GetPrices(times.PrimaryPeriod, times.PrimaryTo, 0,
    function(result) {
      $('#labelBudgetUnits').text('€');
      var Budget = 0.0;
      for(var i = 0; i < result['Sampletime'].length; i++) Budget += result['active_energy'][i] * result['price'][i] / 1000.0;
      $('#labelBudgetPrimaryValue').html(Round2(Budget));
    }
  );
}
/* Page startup sequence */
var MainScrollable = null;
function UpdatePage() {
  MainScrollable = new Scrollable( {Spinners: ['#CostSpinner'], Series: 'Prices', OnSuccess: Scrollable_OnSuccess, Counters: Scrollable_UpdateCounters } );
}