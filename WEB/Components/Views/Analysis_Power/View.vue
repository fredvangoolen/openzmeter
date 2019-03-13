<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><active-power ref="active" v-on:ZoomEvt="ZoomEvt" v-on:SetAutoUpdate="SetAutoUpdate" v-on:AggregationChanged="AggregationChanged" v-on:PeriodChange="PeriodChange" v-on:ExportClick="ExportClick"></active-power></div>
      <div class="col-lg-12"><reactive-power ref="reactive" v-on:ZoomEvt="ZoomEvt"></reactive-power></div>
      <div class="col-lg-12"><details-power></details-power></div>
    </div>
  </div>
</template>

<script>
  import ActivePower from './ActivePower.vue';
  import ReactivePower from './ReactivePower.vue';
  import DetailsPower from './DetailsPower.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data () {
      return { };
    },
    created: function() {
      this.ActiveData = [];
      this.ReactiveDate = [];
      this.LastSample = 0;
      this.Timer = null;
      this.NowTimer = null;
      this.Aggreg = null;
      this.Period = null;
      this.DateWindow = [0, 0];
      this.AutoUpdate = null;
      this.To = null;
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      window.dispatchEvent(new Event('resize'));
      this.UpdateGraph();
      this.NowValue();
    },
    beforeDestroy: function() {
      clearInterval(this.Timer);
      clearInterval(this.NowTimer);
    },
    methods: {
      SetAnalyzer: function() {
        this.LastSample = 0;
        this.UpdateGraph();
        this.NowValue();
      },
      SetAutoUpdate: function(autoUpdate, to) {
        this.AutoUpdate = autoUpdate;
        this.To = to;    
        clearInterval(this.Timer);
        this.LastSample = 0;
        if(autoUpdate === false) this.$refs.active.$refs.tools.WriteTo(this.DateWindow[1]);
        if((autoUpdate === true) || (this.Data === [])) this.UpdateGraph();
      },
      AggregationChanged: function(aggreg) {
        this.Aggreg = aggreg;
        this.LastSample = 0;
        this.UpdateGraph();
      },
      PeriodChange: function(time) {
        this.Period = time;
        this.LastSample = 0;
        this.UpdateGraph();
      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        this.$refs.tools.SaveCVS('Frequency.csv', [['Time', 'Frequency']].concat(this.Data));
      },
      NowValue: function() {
      },
      UpdateGraph: function() {
        if((this.$root.SelectedAnalyzer === null) || (this.Aggreg === null) || (this.Period === null) || (this.AutoUpdate === null) || (this.To === null)) return;
        if(this.LastSample === 0) {
          this.ActiveData = [];
          this.ReactiveDate = [];
        }
        this.$refs.active.$refs.tools.BeginUpdate();
        $.ajax({ url: 'getPrices', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Aggreg: this.Aggreg, To: this.To, LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.ID, Period: this.Period }), 
          success: function(result) {
            if(result['Sampletime'] === null) return;
            if(result['Sampletime'].length === 0) return;
            this.ActiveData = oZm.GraphDataAppend(result, 'Active_Power', this.ActiveData);
            this.ActiveData = oZm.GraphDataAppend(result, 'Active_Power', this.ActiveData);
            this.DateWindow = [result['From'], result['To']];
            this.To = this.$refs.tools.WriteTo(result['To']);
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
//            this.$refs.graph.SetOptions( { dateWindow: this.DateWindow, file: this.Data, valueRange: oZm.Series['FREQUENCY'].Range(this.Data, [true]) } );
//            this.UpdateCounters(result['Aggregation_Source']);
  //          this.ShowSpinner = false;
          }.bind(this),
          complete: function() {
            this.$refs.tools.EndUpdate();
            if(this.Timer !== null) clearTimeout(this.Timer);
            if(this.AutoUpdate === true) this.Timer = setTimeout(this.UpdateGraph.bind(this), 5000);
          }.bind(this),
          error: function(response) {
            if(response.status === 400) this.$refs.aggregation.AggregationFail();
//            this.ShowSpinner = true;
          }.bind(this)
        });
      },
      ZoomEvt: function(minDate, maxDate) {
        clearInterval(this.Timer);
        this.AutoUpdate = false;
        this.To = this.$refs.active.$refs.tools.WriteTo(maxDate, true);
        this.$refs.active.$refs.period.SetPeriod(maxDate - minDate);
        this.Period = maxDate - minDate;
        this.LastSample = 0;
        this.UpdateGraph();
      }
    },
    components: { ActivePower, ReactivePower, DetailsPower }
  }
</script>