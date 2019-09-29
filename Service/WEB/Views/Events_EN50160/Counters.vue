<template>
  <div class="panel-body">
    <spinner v-if="ShowSpinner"></spinner>
    <div class="row">
      <div class="col col-lg-3 col-md-6 col-xs-12">
        <div class="panel panel-other-light text-center">
          <div class="panel-body">
            <div class="float-right" v-if="DipsWarning"><img :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Limit exceded"></div>
            <span class="huge-small">Voltage swell or dips</span><br/>Less than 1 min and -15%/+10% nominal voltage
            <progress-bar :value="Dips" :text="DipsText"></progress-bar>
            <div>10 to 1000 allowed per year</div>
          </div>
        </div>
      </div>
      <div class="col col-lg-3 col-md-6 col-xs-12">
        <div class="panel panel-other-light text-center">
          <div class="panel-body">
            <div class="float-right" v-if="ShortWarning"><img :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Limit exceded"></div>
            <span class="huge-small">Short interruptions</span><br/>Less than 3 min and under 1% nominal voltage
            <progress-bar :value="Short" :text="ShortText"></progress-bar>
            <div>10 to 100 allowed per year</div>
          </div>
        </div>
      </div>                  
      <div class="col col-lg-3 col-md-6 col-xs-12">
        <div class="panel panel-other-light text-center">
          <div class="panel-body">
            <div class="float-right" v-if="LongWarning"><img :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Limit exceded"></div>
            <span class="huge-small">Long interruptions</span><br/>More than 3 min and under 1% nominal voltage
            <progress-bar :value="Long" :text="LongText"></progress-bar>
            <div>10 to 50 allowed per year</div>
          </div>
        </div>
      </div>
      <div class="col col-lg-3 col-md-6 col-xs-12">
        <div class="panel panel-other-light text-center">
          <div class="panel-body">
            <div class="float-right" v-if="HarmonicsWarning"><img :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Limit exceded"></div>
            <span class="huge-small">Harmonics</span><br/>Harmonics in valid ranges as detailed in EN50160
            <progress-bar :value="Harmonics" :text="HarmonicsText"></progress-bar>
            <div class="float-left">90%</div><div class="float-right">100%</div>
            <div class="clearfix"></div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Spinner from 'Components/Spinner.vue';
  import ProgressBar from 'Components/ProgressBar.vue';
  export default {
    data: function() {
      return { Dips: 0, DipsText: '', DipsWarning: true, Short: 0, ShortText: '', ShortWarning: true, Long: 0, LongText: '', LongWarning: true, Harmonics: 0, HarmonicsText: '', HarmonicsWarning: true, ShowSpinner: true, Warning: true };
    },
    created: function() {
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);  
    },
    mounted: function() {
      this.SetAnalyzer();
    },
    methods: {
      SetData(data) {
        this.Dips = data.Dip * 100 / 1000;
        this.DipsText = data.Dip + ' events';
        this.DipsWarning = (data.Dip > 1000);
        this.Short = data.Short * 100 / 100;
        this.ShortText = data.Short + ' events';
        this.ShortWarning = (data.Short > 100);
        this.Long = data.Long * 100 / 50;
        this.LongText = data.Long + ' events';
        this.LongWarning = (data.Long > 100);  
        var In = (data.HarmonicsIN / data.HarmonicsSamples) * 100.0;
        this.Harmonics = (In - 90) * 10;
        this.HarmonicsText = oZm.Round2Plain(In) + '% of time in range';
        this.HarmonicsWarning = ((data.HarmonicsIN / data.HarmonicsSamples) < 0.95);
        this.ShowSpinner = false;
      },
      SetAnalyzer() {
        this.ShowSpinner = true;
      }
    },
    components: { Spinner, ProgressBar }
  }
</script>

<style scoped>
  .text-center {
    font-size: 12px;
  }
  .panel .progress {
    height: 20px;
    background-color: #FFFFFF;
    border:1px solid #BBB;
    margin-bottom: 0px;
  }
  .panel .progress >>> div {
    background-color: #337ab7;
  }
  .panel-other-light {
    border: 1px solid #ccc;
    background: #eee;    
  }
</style>