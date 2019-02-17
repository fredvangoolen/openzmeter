<template>
  <div class="panel panel-voltage" id="VoltagePanel">
    <div class="panel-heading">
      <div class="header-text" style="font-size: 24px; line-height: 16px;">Voltage</div>
      <div class="header-buttons clearfix" id="toolbarVoltage">
        <toolbar-aggregation :options="['3S', '1M', '10M', '1H']" variable="VoltageAggregation" v-on:AggregationChanged="SetAggregation"></toolbar-aggregation>
        <toolbar-period variable="GraphPeriod" v-on:PeriodChanged="SetPeriod"></toolbar-period>
        <toolbar-phases></toolbar-phases>
        <toolbar-tools :options="['EXPORT', 'MAXIMIZE', 'NOW']"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <div class="col-md-6 col-lg-4 col-xl-3">
          <div class="subpanel">
            <div class="huge-small" id="labelVoltageAvgPrimary">Loading data...</div>
            <div class="huge-small" id="labelVoltageAvgPrimaryValue">?</div>
            <div><span id="labelVoltageAvgSecondary">Loading data...</span>&nbsp;<span id="labelVoltageAvgSecondaryValue"></span></div>
          </div>
          <div class="subpanel">
            <div class="huge-small" id="labelVoltageMaxPrimary">Loading data...</div>
            <div class="huge-small" id="labelVoltageMaxPrimaryValue">?</div>
            <div><span id="labelVoltageMaxSecondary">Loading data...</span>&nbsp;<span id="labelVoltageMaxSecondaryValue"></span></div>
          </div>
          <div class="subpanel">
            <div class="huge-small" id="labelVoltageMinPrimary">Loading data...</div>
            <div class="huge-small" id="labelVoltageMinPrimaryValue">?</div>
            <div><span id="labelVoltageMinSecondary">Loading data...</span>&nbsp;<span id="labelVoltageMinSecondaryValue"></span></div> 
          </div>
        </div>
        <div class="col-md-6 col-lg-8 col-xl-9">
          <div class="chart" ref="graph"></div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
  import ToolbarPeriod from 'Components/ToolbarPeriod.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import * as oZm from 'JS/openZmeter.js';

  export default {
    data() {
      return { Graph: null, Data: [], ShowSpinner: true, LastSample: 0, Timer: null, Phases: [], Aggreg: '1H', Period: 0, DateWindow: [0, 0], Now: false };
    },
    created: function() {
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      this.Graph = new Dygraph(this.$refs.graph, [[0, 0]], {});
//      if(this.$root.SelectedAnalyzer !== null) this.UpdateGraph();
    },
    methods: {
      SetAnalyzer() {
        this.Data = [];
        this.LastSample = 0;
//        this.UpdateGraph();
      },
      SetAutoUpdate(autoUpdate) {
        //  $(Self.Opt.NowBtn).click(function() {
//    var End = parseInt(localStorage.getItem('GraphEnd'));
//    clearInterval(Self.Timer);
//    if(End === 0) {
//      $(Self.Opt.NowBtn).removeClass('active');
//      localStorage.setItem('GraphEnd', Self.Range[1]);
//    } else {
//      $(Self.Opt.NowBtn).addClass('active');
//      localStorage.setItem('GraphEnd', 0);
//      Self.LastSample = 0;
//      Self.GetSeries();
//    }
//  });
      },
      SetPhases(phases) {
        this.Phases = phases;
//        this.CalcOptions();
      },
      SetAggregation(aggreg) {
        console.log(aggreg);
        this.Aggreg = aggreg;
        this.Data = [];
        this.LastSample = 0;
//        this.UpdateGraph();
      },
      SetPeriod(ms) {
        console.log(ms);
        this.Period = ms;
        this.Data = [];
        this.LastSample = 0;
//        this.UpdateGraph();
      }
    },
    components: { ToolbarAggregation, ToolbarPeriod, ToolbarPhases, ToolbarTools }
  }
</script>

<style>
</style>


      var VoltageChart     = null;
var VoltageData      = [];
function Scrollable_OnSuccess(result, clear) {
  if(clear) VoltageData = [];
  AjaxGetEvents(result['From'], result['To'], clear);
  VoltageData = GraphDataAppend(result, 'Voltage', VoltageData);
  if(VoltageChart === null) {
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"), VoltageData,
      { strokeWidth: 0.5, stepPlot: true, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(Analyzer.GetSeriesNames('Voltage')), dateWindow: [result['From'], result['To']],
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: ColorArray($('#VoltagePanel').css('border-color'), 3), plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ], xAxisHeight: 24,
        underlayCallback: function(canvas, area, g) {
          var NV = Analyzer.GetSelected().NominalVoltage, Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
          GraphHBackgroundRanges([[NV * 1.1, NV * 1.07, Yellow], [NV * 0.93, NV * 0.9, Yellow], [Infinity, NV * 1.1, Red], [NV * 0.9, -Infinity, Red]], g, canvas);
        },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">V</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: MainScrollable.XAxis
          }
        }  
    );
    $(window).bind('resize', VoltageResizeEvent);
    window.dispatchEvent(new Event('resize'));
    new GraphTools('Voltage', VoltageChart, '#toolbarVoltage', VoltageResizeEvent, function() { exportToCsv('Voltage.csv', [['Time'].concat(Analyzer.GetSeriesNames('Voltage'))].concat(VoltageData)); });
  } else {
    VoltageChart.updateOptions({file: VoltageData, dateWindow: [result['From'], result['To']]});
  }
  VoltageSeries();
}
function VoltageResizeEvent() { 
  if(VoltageChart !== null) VoltageChart.resize('100%', $('#VoltagePanel > .panel-body > .row > div:first-child').height());
}
function VoltageSeries(event) {
  GraphPhaseButtons(VoltageChart, $('#groupVoltagePhases'), 'VisibleVoltageMainPhases', false, function(series) { return CalcRanges(VoltageData, series, false); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupVoltagePhases').on('click', 'button', VoltageSeries);
function Scrollable_UpdateCounters(times, renew) {
  if(renew === true) {
    $('#labelVoltageAvgPrimary').text(times.PrimaryText + ' average');
    $('#labelVoltageMaxPrimary').text(times.PrimaryText + ' maximun');
    $('#labelVoltageMinPrimary').text(times.PrimaryText + ' minimun');
    $('#labelVoltageAvgPrimaryValue, #labelVoltageMaxPrimaryValue, #labelVoltageMinPrimaryValue').text('Calculating...');
    $('#labelVoltageAvgSecondary, #labelVoltageMaxSecondary, #labelVoltageMinSecondary').text(times.SecondaryText);
    $('#labelVoltageAvgSecondaryValue, #labelVoltageMaxSecondaryValue, #labelVoltageMinSecondaryValue').text((times.SecondaryTo !== undefined) ? 'Calculating...' : '');
    if(times.SecondaryTo !== undefined) {
      Analyzer.GetSeriesAggreg(times.SecondaryPeriod, times.SecondaryTo, times.PreferedAggreg, 'avg(rms_v), max(rms_v), min(rms_v)',
        function(result) {
          if(result['Samples'] !== 0) {
            $('#labelVoltageAvgSecondaryValue').html(Array_Round2(result['avg_rms_v']) + '&nbsp;<em>V</em>');
            $('#labelVoltageMinSecondaryValue').html(Array_Round2(result['min_rms_v']) + '&nbsp;<em>V</em>');
            $('#labelVoltageMaxSecondaryValue').html(Array_Round2(result['max_rms_v']) + '&nbsp;<em>V</em>');
          } else {
            $('#labelVoltageAvgSecondaryValue, #labelVoltageMaxSecondaryValue, #labelVoltageMinSecondaryValue').text('No data'); 
          }
          window.dispatchEvent(new Event('resize'));
        }
      );
    }
  }
  Analyzer.GetSeriesAggreg(times.PrimaryPeriod, times.PrimaryTo, times.PreferedAggreg, 'avg(rms_v), max(rms_v), min(rms_v)', 
    function(result) {
      $('#labelVoltageAvgPrimaryValue').html(Array_Round2(result['avg_rms_v'], ColorArray($('#VoltagePanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">V</em>');
      $('#labelVoltageMinPrimaryValue').html(Array_Round2(result['min_rms_v'], ColorArray($('#VoltagePanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">V</em>');
      $('#labelVoltageMaxPrimaryValue').html(Array_Round2(result['max_rms_v'], ColorArray($('#VoltagePanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">V</em>');
      window.dispatchEvent(new Event('resize'));
    }
  );
}
