<template>
  <div class="panel" :class="Color">
    <div class="panel-heading">
      <div class="huge header-text">{{ Title }}</div>
      <div class="header-buttons">
        <div><span id="labelVoltage" class="huge-small">{{ LastValue }}</span><em class="huge-small-unit">{{ Units }}</em></div>
        <div id="labelVoltageAggregation">{{ Aggregation }}</div>
      </div>
    </div>
    <div class="panel-body">
      <div ref="Spinner" class="spinnerdiv" v-bind:class="{ 'hide': !ShowSpinner }"></div>
      <div ref="Chart" class="chart"></div>
    </div>
    <div class="panel-footer">
      <router-link :to="Target"><span class="pull-left">View Details</span></router-link>
      <div class="pull-right" id="toolbarVoltage">
        <toolbar-phases></toolbar-phases>
        <toolbar-tools></toolbar-tools>
      </div>
      <div class="clearfix"></div>
    </div>
  </div>
</template>

<script>
  import * as oZm from '../JS/openZmeter.js';
  import Dygraph from 'dygraphs';
  import ToolbarPhases from './ToolbarPhases.vue';
  import ToolbarTools from './ToolbarTools.vue';
  export default {
    props: ['type'],
    data() {
      return {
        ShowSpinner: true,
        Color: '',
        Units: '',
        Target: '',
        Title: '',
        Serie: '',
        Aggregation: 'Loading data...',
        LastValue: [],
        Graph: null,
        Data: [],
        LastSample: 0,
        Timer: null
      }
    },
    methods: {
      GetSeries() {
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '3S', Series: [this.type], From: 0, LastSample: this.LastSample, Analyzer: 'VIRTUAL_1PHASE' }), 
          success: function(result) {
            this.ShowSpinner = false;
            if(result['Sampletime'] === null) return;
            if(result['Sampletime'].length === 0) return;
              this.Data = oZm.GraphDataAppend(result, this.Serie, this.Data);
//              this.LastValue = oZm.Array_Round2(result['Voltage'][result['Voltage'].length - 1], ColorArray($('#VoltagePanel').css('border-color'), 3));
              this.Aggregation = 'From ' + oZm.GetAggregLongName(result['Aggregation_Source']) + ' aggregation';
//  if(VoltageChart === null) {

//    new GraphTools('Voltage', VoltageChart, '#toolbarVoltage', undefined, function() { exportToCsv('Voltage.csv', [['Time'].concat(Analyzer.GetSeriesNames('Voltage'))].concat(VoltageData)); });
//  } else {
    this.Graph.updateOptions({ file: this.Data, dateWindow: [result['From'], result['To']]});
//  }
//  for(var n = 0; n < Analyzer.GetSeriesNames('Voltage').length; n++) AddEvents(result, Analyzer.GetSeriesNames('Voltage')[n], VoltageChart);
//  VoltageSeries();


//            
//            Self.Opt.OnSuccess(result);
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
          }.bind(this),
          complete: function() {
            if(this.UpdateIndicatorsTimer !== null) clearTimeout(this.UpdateIndicatorsTimer);
            this.UpdateIndicatorsTimer = setTimeout(this.GetSeries, 5000);
          }.bind(this), 
          error: function() {
            this.ShowSpinner = true;
    //this.$refs.indicator_bar.setValue(null);
          }.bind(this)
        });
      },
      AdjustSeries(event) {
        GraphPhaseButtons(VoltageChart, $('#groupVoltagePhases'), 'DashboardVoltageVisiblePhases', false, function(series) { return CalcRanges(VoltageData, series, false, true); }, (event !== undefined) ? event.target : undefined); 
      }
    },
    mounted: function() {
      if(this.type === 'VOLTAGE') {
        this.Color = 'panel-voltage';
        this.Title = 'Voltage';
        this.Serie = 'Voltage';
        this.Units = 'V';
        this.Target = '/Analysis/Voltage';
      } else if(this.type === 'CURRENT') {
        this.Color = 'panel-current';
        this.Title = 'Current';
        this.Serie = 'Current';
        this.Units = 'A';
        this.Target = '/Analysis/Current';
      } else if(this.type === 'ACTIVE_POWER') {
        this.Color = 'panel-active-power';
        this.Title = 'Active power';
        this.Serie = 'Active_Power';
        this.Units = 'w';
        this.Target = '/Analysis/Power';
      } else if(this.type === 'FREQUENCY') {
        this.Color = 'panel-frequency';
        this.Title = 'Frequency';
        this.Serie = 'Frequency';
        this.Units = 'Hz';
        this.Target = '/Analysis/Frequency';
      }
      this.Graph = new Dygraph(this.$refs.Chart, this.Data,
        { strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: false, rollPeriod: 0, legend: 'follow', interactionModel: {},
//        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], 
         legendFormatter: oZm.LegendFormatter_Follow, 
        labels: ['Time', 'bliu'], /*.concat(Analyzer.GetSeriesNames('Voltage')),*/
//        colors: ColorArray($('#VoltagePanel').css('border-color'), 3), dateWindow: [result['From'], result['To']],
//        underlayCallback: function(canvas, area, g) {
//          var NV = Analyzer.GetSelected().NominalVoltage, Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
//          GraphHBackgroundRanges([[NV * 1.1, NV * 1.07, Yellow], [NV * 0.93, NV * 0.9, Yellow], [Infinity, NV * 1.1, Red], [NV * 0.9, -Infinity, Red]], g, canvas);
//        },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
//               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">V</em>'; },
//               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return x.toLocaleTimeString(); }
             }
          }
        }
      );
      this.GetSeries();
    },
    components: { ToolbarPhases, ToolbarTools }
  }
</script>

<style>
  
</style>
