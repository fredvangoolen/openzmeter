<template>
  <div>
    <div ref="Spinner" class="spinnerdiv" v-bind:class="{ 'hide': !ShowSpinner }"></div>
    <div>
      <cubism-row v-for="row in Series" :key="row" :serie="row" :first="row === Series[0]"></cubism-row>  
    </div>
  </div>
</template>

<script>
  import CubismRow from './CubismRow.vue';
  import * as oZm from '../JS/openZmeter.js';
  export default {
    props: {
      Series: { type: Array, required: true }
    },
    data() {
      return { 
        LastSample: 0,
        ShowSpinner: true,
        Data: {}
      };
    },
    methods: {
      GetSeries() {
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '1M', Series: this.Series, From: 0, Period: 86400000,LastSample: this.LastSample, Analyzer: 'VIRTUAL_1PHASE' }), 
          success: function(result) {
            this.ShowSpinner = false;
            this.$emit('AggregationSet', result['Aggregation_Source']);
            if(result['Sampletime'] === null) return;
            if(result['Sampletime'].length === 0) return;
            for(var i = 0; i < this.Series.length; i++) {
              if(this.Series[i] === 'ACTIVE_POWER') this.Data[this.Series[i]] = oZm.GraphDataAppend(result, 'Active_Power', this.Data[this.Series[i]]);
              if(this.Series[i] === 'REACTIVE_POWER') this.Data[this.Series[i]] = oZm.GraphDataAppend(result, 'Reactive_Power', this.Data[this.Series[i]]);
            }
            this.$emit('SeriesUpdate', this.Data, result['From'], result['To'], 0);
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
          }.bind(this),
          complete: function() {
            if(this.UpdateIndicatorsTimer !== null) clearTimeout(this.UpdateIndicatorsTimer);
            this.UpdateIndicatorsTimer = setTimeout(this.GetSeries, 60000);
          }.bind(this), 
          error: function() {
            this.ShowSpinner = true;
          }.bind(this)
        });
      }
    },
    mounted: function() {
      for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = [];
      this.GetSeries();
    },
    components: { CubismRow }
  }
</script>

<style>

</style>