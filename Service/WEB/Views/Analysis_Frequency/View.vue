<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-frequency">
          <div class="panel-heading">
            <div class="huge header-text">Frequency</div>
            <div class="header-buttons clearfix" ref="Toolbar">
              <toolbar-aggregation storage="Aggregation" :options="['3S', '1M', '10M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period variable="Period" v-on:PeriodChanged="UpdateGraph" ref="period"></toolbar-period>
              <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" :dataNow="NowData"></counters>
              <frequency class="col-md-6 col-lg-8 col-xl-9" :data="GraphData"></frequency>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
  <div v-else-if="analyzer == null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>  
  </div>
</template>

<script>
  import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
  import ToolbarPeriod from 'Components/ToolbarPeriod.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Frequency from './Frequency.vue';
  import Counters from './Counters.vue';
  export default {
    props: { analyzer: { type: Object } },
    data() { return { Aggreg: null, ShowSpinner: true, NowData: null, GraphData: null, NowTimer: null, GraphTimer: null }; },
    watch: {
      analyzer: function() { this.UpdateNowValues(); this.UpdateGraph(); },
      Aggreg: function() {  this.UpdateGraphValues(); }
    },
    created: function() {
      this.UpdateNowValues(); 
      this.UpdateGraphValues();
    },
    beforeDestroy: function() {
      if(this.NowTimer != null) clearTimeout(this.NowTimer);
      if(this.GraphTimer != null) clearTimeout(this.GraphTimer);
    },
    methods: {
      MaximizeClick: function() {
        $(window).unbind('resize', this.ResizeHandler);
        this.$refs.tools.Maximize('Energy consumption', this.$refs.graph.Graph, this.$refs.Toolbar);
      },
      MinimizeClick: function() {
        $(window).bind('resize', this.ResizeHandler.bind(this));
      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        this.$refs.tools.SaveCVS('Frequency.csv', [['Time', 'Frequency']].concat(this.Data));
      },
      UpdateNowValues: function() {
        if(this.NowTimer != null) clearTimeout(this.NowTimer);
        if(this.analyzer == null) return;
        this.NowQuery = this.$http.post('getSeries', { Series: ['FREQUENCY'], To: 0, Aggreg: '3S', Period: 5000, LastSample: 0, Analyzer: this.analyzer.Analyzer }).then( 
          (result) => {
            this.NowData = (result.status == 200) ? result.data : null;
            this.NowTimer = setTimeout(() => { this.UpdateNowValues(); }, 2000);
          }
        );
      },
      UpdateGraphValues: function(to, period, lastsample, aggreg) {
        return;
        if(lastsample === 0) {
          this.Data = [];
          this.ShowSpinner = true;
        }
        var Analyzer = this.$root.SelectedAnalyzer;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Aggreg: aggreg, Series: ['FREQUENCY'], To: to, LastSample: lastsample, Analyzer: Analyzer.Analyzer, Period: period }), 
          success: function(result) {
            if((result['Sampletime'] === null) || (result['Sampletime'].length === 0)) return;
            this.Data = oZm.GraphDataAppend(result, 'Frequency', this.Data);
            lastsample = oZm.LastSampletime(result, lastsample);
            this.$refs.graph.SetOptions( { dateWindow: [result.From, result.To], file: this.Data, valueRange: oZm.Series['FREQUENCY'].Range(this.Data, [true]) } );
            this.UpdateCounters(result['Aggregation_Source'], result.From, result.To);
          }.bind(this),
          complete: function() {
            this.ShowSpinner = false;
            this.$refs.period.UpdateCompleted(lastsample);
          }.bind(this),
          error: function(response) {
            if(response.status === 400) this.$refs.aggregation.AggregationFail();
            this.ShowSpinner = true;
          }.bind(this)
        });
      },
      UpdateGraph: function() {

      }
    },
    components: { ToolbarAggregation, ToolbarPeriod, ToolbarTools, Frequency, Counters }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
