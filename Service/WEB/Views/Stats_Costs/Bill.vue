<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">
        <span>From {{ analyzer.FormatDate(bill.From) }} to {{ analyzer.FormatDate(bill.To) }} ({{ bill.Name }})</span>
        <div v-if="bill.Complete==false" class="title-warning"><img :src="require('Imgs/exclamation.png')">Bill is not yet complete, partial data is shown</div>
      </div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="float-right">
        <toolbar-aggregation storage="AggregationBill" v-model="Aggreg" :options="['15M', '1H', '1D']"></toolbar-aggregation>
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" :class="{ active: GraphSeries=='Energy' }" @click="GraphSeries='Energy'">Energy</button>
            <button type="button" class="btn btn-default btn-xs" :class="{ active: GraphSeries=='Power' }" @click="GraphSeries='Power'">Power</button>
          </div>
          <div class="btn-group">
            <!--button-export :rows="Export" file="Evolution"></button-export-->
            <button-expand v-model="Expanded"></button-expand>
          </div>
        </portal>
      <div class="float-right"><strong style="font-size: 16px; padding-right: 10px" v-html="Round(bill.Total, 2, true) + ' ' + bill.Currency"></strong></div>
    </div>
    <div class="panel-body">
      <div class="row">
        <div class="col-md-12">
          <portal to="fs-body" :disabled="!Expanded" class="panel-body">
            <spinner v-if="TransferGraph.Data==null"></spinner>
            <graph :options="GraphOptions" :height="Expanded?DialogHeight:200"></graph>
          </portal>
        </div>
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
  import Dygraph from 'JS/dygraph.js';
  import moment from 'moment';
  import Graph from 'Components/Graph.vue';
  export default {
    props: { bill: Object, analyzer: Object },
    data: function() {
      return { Expanded: false, DialogHeight: 200, GraphSeries: 'Energy', Aggreg: null, TransferGraph: this.PriceTransfer(['Active_Energy', 'Active_Power', 'Color', 'Period', 'Contracted', 'Energy_Cost', 'Power_Cost']),
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
    created: function() {
      this.TransferGraph.Update(this.analyzer, this.Aggreg, [this.bill.From, this.bill.To]);
    },
    watch: {
      Aggreg: function() {
        this.TransferGraph.Update(this.analyzer, this.Aggreg, [this.bill.From, this.bill.To]);
      },
      analyzer: function() {
        this.TransferGraph.Update(this.analyzer, this.Aggreg, [this.bill.From, this.bill.To]);
      }
    },
    computed: {
      GraphOptions: function() {
        var Opt = { file: [], axes: { x: {}, y: {}, y2: {} }};
        Opt.interactionModel = Graph.InteractionModels.Fixed(this);
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
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
        Opt.xAxisHeight = 12;
        Opt.axes.x.valueFormatter = (x) => { return Graph.Formatters.X_Hourly(x, this.analyzer.TimeZone,  ((this.TransferGraph.Data != null) && (this.TransferGraph.Data.Aggreg == '1H')) ? false : true); };
        Opt.axes.x.ticker = (min, max, pixels) => { return Graph.Formatters.X_HourlyFull(min, max, pixels, (this.analyzer != null) ? this.analyzer.TimeZone : null); };
        if(this.TransferGraph.Data == null) return Opt;
        Opt.underlayCallback = (ctx, area, dygraph) => { Graph.Callbacks.Underlay_DrawPeriods(ctx, area, dygraph, this.TransferGraph.Data) };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return Graph.Formatters.Y_UnitScaleFull(x, gran, opts, g, '', 2, true); };
        var TimeCol = this.TransferGraph.Data.Headers.indexOf('Time');
        if(this.GraphSeries == 'Energy') {
          var EnergyCol = this.TransferGraph.Data.Headers.indexOf('Active_Energy');
          var CostCol = this.TransferGraph.Data.Headers.indexOf('Energy_Cost');
          Opt.labels = ['Time', 'Energy', 'Energy cost'];
          Opt.colors = [this.analyzer.SerieColor('Active_Energy'), '#d9534f'];
          Opt.series = { 'Energy': { axis: 'y', plotter: (e) => { Graph.Plotters.BarOffset(e, (this.Aggreg == '1D') ? 86400000 : (this.Aggreg == '1H') ? 3600000 : 900000); } }, 'Energy cost': { axis: 'y2' } };
          for(var n = 0; n < this.TransferGraph.Data.Rows.length; n++) Opt.file.push([this.TransferGraph.Data.Rows[n][TimeCol], this.TransferGraph.Data.Rows[n][EnergyCol], this.TransferGraph.Data.Rows[n][CostCol]]);
          Opt.axes.y.valueFormatter = (x) => { return Graph.Formatters.Y_UnitScale(x, 'Wh', 2, true);  };
          Opt.axes.y2.valueFormatter = (x) => { return this.Round(x, 2, true) + this.bill.Currency + '/' + this.bill.EnergyUnits; };
        } else {
          var ActiveCol = this.TransferGraph.Data.Headers.indexOf('Active_Power');
          var CostCol = this.TransferGraph.Data.Headers.indexOf('Power_Cost');
          var ContractedCol = this.TransferGraph.Data.Headers.indexOf('Contracted');
          Opt.labels = ['Time', 'Power', 'Contracted', 'Cost'];
          Opt.colors = [this.analyzer.SerieColor('Active_Power'), '#428bca', '#d9534f'];
          Opt.series = { 'Power': { axis: 'y', fillGraph: true, plotter: (e) => { Graph.Plotters.LineOffset(e, (this.Aggreg == '1D') ? 86400000 : (this.Aggreg == '1H') ? 3600000 : 900000); } }, 'Contracted': { axis: 'y', strokeWidth: 1.0 }, 'Cost': { axis: 'y2' } };
          for(var n = 0; n < this.TransferGraph.Data.Rows.length; n++) Opt.file.push([this.TransferGraph.Data.Rows[n][TimeCol], this.TransferGraph.Data.Rows[n][ActiveCol], this.TransferGraph.Data.Rows[n][ContractedCol] * 1000.0, this.TransferGraph.Data.Rows[n][CostCol]]);
          Opt.axes.y.valueFormatter = (x) => { return Graph.Formatters.Y_UnitScale(x, 'W', 2, true); };
          Opt.axes.y2.valueFormatter = (x) => { return this.Round(x, 2, true) + this.bill.Currency + '/' + this.bill.PowerUnits; };
        }
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
    }
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