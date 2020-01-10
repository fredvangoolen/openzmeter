<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-frequency">
          <div class="panel-heading">
            <div class="huge header-text">Frequency</div>
            <div class="header-buttons clearfix" ref="Toolbar">
              <toolbar-aggregation storage="Aggregation" :options="['3S', '1M', '10M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <div class="btn-group">
                <button-export :rows="ExportData" file="Frequency"></button-export>
                <button-expand></button-expand>
                <!--toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools-->
              </div>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" :analyzer="analyzer" :dataNow="NowData" :dataGraph="GraphData" :height.sync="Height"></counters>
              <frequency class="col-md-6 col-lg-8 col-xl-9" :analyzer="analyzer" :data="GraphData" :height="Height"></frequency>
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
  import Frequency from './Frequency.vue';
  import Counters from './Counters.vue';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { Aggreg: null, ShowSpinner: true, NowData: null, GraphData: null, NowTimer: null, GraphTimer: null, Range: null, Height: 300 };
      },
    watch: {
      analyzer: function() { this.UpdateNowValues(); this.UpdateGraphValues(); },
      Aggreg: function() {  this.UpdateGraphValues(); },
      Range: function() { this.UpdateGraphValues(); }
    },
    computed: {
      ExportData: function() {
        if(this.GraphData == null) return [];
        var Rows = [['Time', 'Frequency']];
        for(var n in this.GraphData.Sampletime) Rows.push([this.GraphData.Sampletime[n], this.GraphData.Frequency[n]]);
        return Rows;
      }
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
      UpdateNowValues: function() {
        if(this.NowTimer != null) clearTimeout(this.NowTimer);
        if(this.analyzer == null) return;
        this.analyzer.GetSeriesLast('3S', ['FREQUENCY'],
          (working) => {},
          (result) => { this.NowData = result; },
          (timeout) => { this.NowTimer = setTimeout( () => { this.UpdateNowValues(); }, timeout); }
        );
      },
      UpdateGraphValues: function() {
        if(this.GraphTimer != null) clearTimeout(this.GraphTimer);
        if((this.Aggreg == null) || (this.Range == null) || (this.analyzer == null)) return;
        this.analyzer.GetSeriesRange(this.Aggreg, this.Range, this.GraphData, ['FREQUENCY'],
          (working) => { this.ShowSpinner = working; },
          (result) => { this.GraphData = Object.freeze(result); },
          (timeout) => { if(timeout != null) this.GraphTimer = setTimeout( () => { this.UpdateGraphValues(); }, timeout); }
        );
//        if(this.GraphData == null) this.GraphData = { LastSample: 0 };
//        if(this.GraphData.LastSample == 0) this.ShowSpinner = true;
//        this.$http.post('getSeries', { Aggreg: this.Aggreg, Series: ['FREQUENCY'], To: Window.To, Period: Window.Period, LastSample: 0, Analyzer: this.analyzer.Analyzer }).then(
//          (result) => {
//            this.GraphData = oZm.MergeSeriesResult(result.data, this.GraphData);
//            this.ShowSpinner = false;
//            if((result.data['Sampletime'] === null) || (result.data['Sampletime'].length === 0)) return;
//            this.GraphData = oZm.GraphDataAppend(result.data, 'Frequency', this.GraphData);
//            lastsample = oZm.LastSampletime(result, lastsample);
//            this.$refs.graph.SetOptions( { dateWindow: [result.From, result.To], file: this.Data, valueRange: oZm.Series['FREQUENCY'].Range(this.Data, [true]) } );
//            this.UpdateCounters(result['Aggregation_Source'], result.From, result.To);
//          }
//        );
//          complete: function() {
//            this.ShowSpinner = false;
//            this.$refs.period.UpdateCompleted(lastsample);
//          }.bind(this),
//          error: function(response) {
//            if(response.status === 400) this.$refs.aggregation.AggregationFail();
//            this.ShowSpinner = true;
//          }.bind(this)
//        });
      }
    },
    components: { Frequency, Counters }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
