<template>
  <div class="panel panel-voltage" id="VoltagePanel">
    <div class="panel-heading">
      <div class="header-text" style="font-size: 24px; line-height: 16px;">Voltage</div>
      <div class="header-buttons clearfix" ref="Toolbar">
        <toolbar-aggregation variable="Aggregation" :options="['3S', '1M', '10M', '1H']" v-on:AggregationChanged="AggregationChanged" ref="aggregation"></toolbar-aggregation>
        <toolbar-period variable="Period" v-on:PeriodChanged="PeriodChange" ref="period"></toolbar-period>
        <toolbar-phases variable="VoltageViewPhases" type="VOLTAGE" :multi="true" v-on:PhasesChanged="PhasesChanged"></toolbar-phases>
        <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE', 'NOW']" v-on:NowStatus="SetAutoUpdate" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <counters class="col-md-6 col-lg-4 col-xl-3" ref="counters"></counters>
        <div class="col-md-6 col-lg-8 col-xl-9">
          <graph ref="graph" :options="GraphOptions"></graph>
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
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import Counters from './Counters.vue';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    data() {
      return { ShowSpinner: true };
    },
    created: function() {
      this.Data = [];
      this.LastSample = 0;
      this.Timer = null;
      this.Phases = null;
      this.Aggreg = null;
      this.Period = null;
      this.DateWindow = [0, 0];
      this.AutoUpdate = null;
      this.To = null;
      this.GraphOptions = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow,
        labels: ['Time', 'Voltage'], zoomCallback: this.ZoomEvt.bind(this), interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: this.MouseUpEvt.bind(this), dblclick: this.DoubleClickEvt.bind(this) }, colors: [oZm.Series['FREQUENCY'].Color], xAxisHeight: 24, 
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Result = oZm.Series['VOLTAGE'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
               axisLabelFormatter: function (x) { var Result = oZm.Series['VOLTAGE'].FormatValue(x); return Result.Value; }
             },
          x: { axisLabelFormatter: function(x) { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); },
               valueFormatter: function (x) { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); }
             }
        }
      };
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      $(window).on('resize', this.ResizeHandler.bind(this));
      window.dispatchEvent(new Event('resize'));
      if(this.$root.SelectedAnalyzer !== null) this.SetAnalyzer();
      this.UpdateGraph();
    },
    beforeDestroy: function() {
      clearInterval(this.Timer);
      $(window).off('resize', this.ResizeHandler);
    },
    methods: {
      SetAnalyzer() {
        this.LastSample = 0;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Options = {
          labels: ['Time'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)), colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length)
        };
        this.$refs.graph.SetOptions( Options );
        this.UpdateGraph();
      },
      SetAutoUpdate(autoUpdate, to) {
        this.AutoUpdate = autoUpdate;
        this.To = to;
        clearInterval(this.Timer);
        this.LastSample = 0;
        if(autoUpdate === false) this.$refs.tools.WriteTo(this.DateWindow[1]);
        if((autoUpdate === true) || (this.Data === [])) this.UpdateGraph();
      },
      AggregationChanged(aggreg) {
        this.Aggreg = aggreg;
        this.LastSample = 0;
        this.UpdateGraph();
      },
      PeriodChange(time) {
        this.Period = time;
        this.LastSample = 0;
        this.UpdateGraph();
      },
      PhasesChanged(phases) {
        this.Phases = phases;
        this.$refs.graph.SetOptions( { visibility: this.Phases } );
        if(this.Data.length === 0) this.UpdateGraph();
      },
      ResizeHandler: function() {
        if(typeof this.$refs.graph !== 'undefined') this.$refs.graph.Redraw('100%', $(this.$refs.counters.$el).height());
      },
      MaximizeClick: function() {
        $(window).unbind('resize', this.ResizeHandler);
        this.$refs.tools.Maximize('Voltage', this.$refs.graph.Graph, this.$refs.Toolbar);
      },
      MinimizeClick: function() {
        $(window).bind('resize', this.ResizeHandler.bind(this));
      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
//        this.$refs.tools.SaveCVS('Frequency.csv', [['Time', 'Frequency']].concat(this.Data));
      },
      UpdateGraph() {
        if((this.$root.SelectedAnalyzer === null) || (this.Aggreg === null) || (this.Period === null) || (this.AutoUpdate === null) || (this.Phases === null) || (this.To === null)) return;
        if(this.LastSample === 0) {
          this.Data = [];
          this.ShowSpinner = true;
        }        
        this.$refs.tools.BeginUpdate();
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Aggreg: this.Aggreg, Series: ['VOLTAGE'], To: this.To, LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.ID, Period: this.Period }), 
          success: function(result) {
            if((result['Sampletime'] === null) || (result['Sampletime'].length === 0)) return;
            this.Data = oZm.GraphDataAppend(result, 'Voltage', this.Data);
            this.DateWindow = [result['From'], result['To']];
            this.To = this.$refs.tools.WriteTo(result['To']);
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
            this.$refs.graph.SetOptions({ dateWindow: this.DateWindow, file: this.Data, valueRange: oZm.Series['VOLTAGE'].Range(this.Data, this.Phases) } );
            this.$refs.counters.SetStats(this.DateWindow, this.Data, result["Aggregation_Source"]);
            this.ShowSpinner = false;
          }.bind(this),
          complete: function() {
            this.$refs.tools.EndUpdate();
            if(this.Timer !== null) clearTimeout(this.Timer);
            if(this.AutoUpdate === true) this.Timer = setTimeout(this.UpdateGraph.bind(this), 5000);
          }.bind(this),
          error: function(response) {
            if(response.status === 400) this.$refs.aggregation.AggregationFail();
            this.ShowSpinner = true;
          }.bind(this)
        });
      },
      MouseUpEvt(event, g, contex) {
        oZm.mouseUp(event, g, contex);
        if((contex.isPanning) && (contex.regionWidth > 0)) this.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
      },
      DoubleClickEvt(event, g, context) {
        oZm.dblClick(event, g, context);
        this.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
      },
      ZoomEvt(minDate, maxDate, yRanges) {
        clearInterval(this.Timer);
        this.$refs.tools.WriteTo(maxDate, true);
        this.$refs.period.SetPeriod(maxDate - minDate);
        this.DateWindow = [minDate, maxDate];
        this.LastSample = 0;
        this.UpdateGraph();
      }
    },
    components: { Spinner, ToolbarAggregation, ToolbarPeriod, ToolbarPhases, ToolbarTools, Counters, Graph }
  }
</script>


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
