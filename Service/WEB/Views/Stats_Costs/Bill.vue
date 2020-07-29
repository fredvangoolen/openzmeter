<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">
        <span>From {{ analyzer.FormatDate(bill.From) }} to {{ analyzer.FormatDate(bill.To) }} ({{ bill.Name }})</span>
        <div v-if="bill.Complete==false" class="title-warning"><img :src="require('Imgs/exclamation.png')">Bill is not yet complete, partial data is shown</div>
      </div>
      <div class="float-right"><strong style="font-size: 16px" v-html="Round(bill.Total, 2, true) + ' ' + bill.Currency"></strong></div>
    </div>
    <div class="panel-body">
      <div class="row">
        <div class="col-md-12 col-lg-6">
          <div v-for="(p, idx) in bill.Periods" :key="idx" class="main-div">
            <table class="main-table">
              <tr><td class="border-bottom">Period P{{ idx + 1 }}
                <div class="float-right"><strong v-html="Round(p.reduce((sum, i) => { return sum + ((i.Type != 'INFO') ? i.Value : 0.0); }, 0.0), 2, true) + ' ' + bill.Currency"></strong></div>
              </td></tr>
              <tr v-if="p.reduce((sum, i) => { return sum + i.Value; }, 0.0)!=0"><td>
                <vuetable :fields="ColumnsInfo" :api-mode="false" v-show="p.filter((v)=>v.Type=='INFO').length>0" :data="p.filter((v) => v.Type == 'INFO')" class="table-info table small table-striped"></vuetable>
                <vuetable :fields="ColumnsCost" :api-mode="false" v-show="p.filter((v)=>v.Type!='INFO').length>0" :data="p.filter((v) => v.Type != 'INFO')" class="table-cost table small table-striped"></vuetable>
              </td></tr>
            </table>
          </div>
        </div>
        <div class="col-md-12 col-lg-6">
          <table class="main-table" >
            <tr><td class="border-bottom">Evolution
              <portal to="fs-toolbar" :disabled="!Expanded" class="float-right">
                <div class="btn-group">
                  <button type="button" class="btn btn-default btn-xs" :class="{ active: GroupDay }" @click="GroupDay=!GroupDay"><img :src="require('Imgs/calendar_view_day.png')"></button>
                </div>
                <div class="btn-group">
                  <button type="button" class="btn btn-default btn-xs" :class="{ active: GraphSeries=='Energy' }" @click="GraphSeries='Energy'">Energy</button>
                  <button type="button" class="btn btn-default btn-xs" :class="{ active: GraphSeries=='Power' }" @click="GraphSeries='Power'">Power</button>
                </div>
                <div class="btn-group">
                  <button-export :rows="Export" file="Evolution"></button-export>
                  <button-expand v-model="Expanded"></button-expand>
                </div>
              </portal>
            </td></tr>
            <tr><td style="padding: 5px 0px">
              <portal to="fs-body" :disabled="!Expanded" class="panel-body">
                <graph :options="GraphOptions" :height="Expanded?DialogHeight: 200"></graph>
              </portal>
            </td></tr>
          </table>
          <table class="main-table" v-show="bill.General.length>0">
            <tr><td class="border-bottom">General
              <div class="float-right"><strong v-html="Round(bill.General.reduce((sum, i) => { return sum + ((i.Type != 'INFO') ? i.Value : 0.0); }, 0.0), 2, true) + ' ' + bill.Currency"></strong></div>
            </td></tr>
            <tr><td>
              <vuetable :fields="ColumnsInfo" :api-mode="false" v-show="bill.General.filter((v)=>v.Type=='INFO').length>0" :data="bill.General.filter((v) => v.Type == 'INFO')" class="table-info table small table-striped"></vuetable>
              <vuetable :fields="ColumnsCost" :api-mode="false" v-show="bill.General.filter((v)=>v.Type!='INFO').length>0" :data="bill.General.filter((v) => v.Type != 'INFO')" class="table-cost table small table-striped"></vuetable>
            </td></tr>
          </table>
          <table class="main-table" v-show="bill.Taxes.length>0">
            <tr><td class="border-bottom">Tax and rents</td></tr>
            <tr><td>
              <vuetable :fields="ColumnsTax" :api-mode="false" :data="bill.Taxes" class="table-info table small table-striped"></vuetable>
            </td></tr>
          </table>
        </div>
      </div>
    </div>
    <dlg-fs v-if="Expanded" title="Evolution" :height.sync="DialogHeight"></dlg-fs>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  import moment from 'moment';
  import Graph from 'Components/Graph.vue';
  import vuetable from 'vuetable-2';
  export default {
    props: { bill: Object, analyzer: Object },
    data: function() {
      return { Expanded: false, DialogHeight: 200, GraphSeries: 'Energy', GroupDay: true,
        ColumnsCost: [
          { name: 'Description', title: 'Concept' },
          { name: 'Type', title: 'Term', callback: this.TermFormat },
          { name: 'Value', title: 'Cost', callback: this.CurrencyFormat }
        ],
        ColumnsInfo: [
          { name: 'Description', title: 'Concept' },
          { name: 'Value', title: 'Value', callback: this.ValueFormat }
        ],
        ColumnsTax: [
          { name: 'Description', title: 'Concept' },
          { name: 'Value', title: 'Value', callback: this.CurrencyFormat }
        ],
      };
    },
    computed: {
      GraphOptions: function() {

        var Opt = { file: [], axes: { x: {}, y: {}, y2: {} }};
        return Opt;
        Opt.interactionModel = Graph.InteractionModels.Fixed(this);
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.showRangeSelector = false;
        Opt.strokeWidth = 0.5;
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.legend = 'follow';
        Opt.selectMode = 'euclidian';
        Opt.includeZero = true;

        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 30;
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.axisLabelWidth = 30;
        Opt.dateWindow = [this.bill.From, this.bill.To];

        if(this.GroupDay) {
          Opt.xAxisHeight = 10;
          Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatDate(x); };
          Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatDate(x); };
          if(this.GraphSeries=='Energy') {
            var PrevDay     = null;
            var EnergySum   = 0;
            var EnergyPrice = 0;
            var Samples     = 0;
            for(var n = 0; n < this.bill.Sampletime.length; n++) {
              var Date = moment(this.bill.Sampletime[n]).tz(this.analyzer.TimeZone);
              if((PrevDay != null) && (PrevDay.isSame(Date, 'day'))) {
                EnergySum += this.bill.Active_Power[n] / 4.0;
                EnergyPrice += this.bill.Period_Energy_Cost[n];
                Samples++;
              } else {
                if(Samples != 0) Opt.file.push([PrevDay.valueOf(), EnergySum, EnergyPrice /Samples]);
                Samples = 0;
                EnergySum = this.bill.Active_Power[n] / 4.0;
                EnergyPrice = this.bill.Period_Energy_Cost[n];
                PrevDay = Date;
              }
            }
            if(Samples != 0) Opt.file.push([PrevDay.valueOf(), EnergySum, EnergyPrice /Samples]);
          }
        } else {
          Opt.xAxisHeight = 20;
          Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatDateTime(x); };
          Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatDateTime(x, '<br/>'); };
          if(this.GraphSeries=='Energy') {
            for(var n = 0; n < this.bill.Sampletime.length; n++) Opt.file.push([this.bill.Sampletime[n], this.bill.Active_Power[n] / 4.0, this.bill.Period_Energy_Cost[n]]);
          }
        }
        if(this.GraphSeries=='Energy') {
          Opt.labels = ['Time', 'Energy', 'Cost'];
          Opt.colors = [this.analyzer.Color.Active_Energy, '#d9534f'];
          Opt.series = { 'Energy': { axis: 'y', plotter: Graph.Plotters.Bar }, 'Cost': { axis: 'y2' } };
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true, false], { incZero: 1.0 });
          var Units = this.BestUnits(Math.max(Math.abs(Opt.axes.y.valueRange[0]), Math.abs(Opt.axes.y.valueRange[1])));
          Opt.axes.y.valueFormatter = (x) => { return this.Round(this.ScaleUnits(x, Units), 2, true) + ' ' + this.bill.EnergyUnits; };
          Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(this.ScaleUnits(x, Units), 2, true); };
        } else {
          Opt.labels = ['Time', 'Power', 'Contracted', 'Cost'];
          Opt.colors = [this.analyzer.Color.Active_Power, '#428bca', '#d9534f'];
          Opt.series = { 'Power': { axis: 'y', fillGraph: true }, 'Power': { axis: 'y' }, 'Cost': { axis: 'y2' } };
          for(var n = 0; n < this.bill.Sampletime.length; n++) Opt.file.push([this.bill.Sampletime[n], this.bill.Active_Power[n], this.bill.Period_Power_Contracted[n], this.bill.Period_Power_Cost[n]]);
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true, true, false], { incZero: 1.0 });
          var Units = this.BestUnits(Math.max(Math.abs(Opt.axes.y.valueRange[0]), Math.abs(Opt.axes.y.valueRange[1])));
          Opt.axes.y.valueFormatter = (x) => { return this.Round(this.ScaleUnits(x, Units), 2, true) + ' ' + this.bill.PowerUnits; };
          Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(this.ScaleUnits(x, Units), 2, true); };
        }
        Opt.axes.y2.valueFormatter = (x) => { return this.Round(x, 2, true) + ' ' + this.bill.Currency + '/' + this.bill.EnergyUnits; };
        Opt.axes.y2.axisLabelFormatter = (x) => { return this.Round(x, 2, true); };
        return Object.seal(Opt);
      }
    },
    methods: {
      Export: function() {

      },
      ValueFormat: function(val) {
        return this.Round(val, 2);
      },
      CurrencyFormat: function(val) {
        return this.Round(val, 2) + this.bill.Currency;
      },
      TermFormat: function(val) {
        return (val == 'COST_ENERGY') ? 'Energy' : (val == 'COST_POWER') ? 'Power' : 'General';
      }
    },
    components: { vuetable }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
  .panel-other {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
  .main-table {
    border: 1px solid #ccc;
  }
  .main-div {
    margin-bottom: 5px;
  }
  .main-table:not(:first-child) {
    margin: 5px 0px;
  }
  .main-table .border-bottom {
    border-bottom: 1px solid #ccc;
    background: #eee;
    font-weight: bold;
    padding: 3px;
  }
  table {
    width: 100%;
  }
  .table-cost >>> th:nth-child(2) {
    width: 80px;
    text-align: center;
  }
  .table-cost >>> td:nth-child(2) {
    text-align: center;
  }
  .table-cost >>> th:nth-child(3), .table-info >>> th:nth-child(2) {
    width: 80px;
    text-align: right;
  }
  .table-cost >>> td:nth-child(3), .table-info >>> td:nth-child(2) {
    text-align: right;
  }
</style>