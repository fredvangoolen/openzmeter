<template>
  <div class="row">
    <div class="col col-lg-3 col-md-6 col-xs-12">
      <div class="panel panel-other-light text-center">
        <div class="panel-body">
          <div class="float-right" v-if="(data[index].Dip > 1000)"><img :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Limit exceded"></div>
          <span class="huge-small">Voltage swell or dips</span><br/>Less than 1 min and -15%/+10% nominal voltage
          <progress-bar :value="data[index].Dip * 100 / 1000" :text="data[index].Dip + ' events'"></progress-bar>
          <div>10 to 1000 allowed per year</div>
        </div>
      </div>
    </div>
    <div class="col col-lg-3 col-md-6 col-xs-12">
      <div class="panel panel-other-light text-center">
        <div class="panel-body">
          <div class="float-right" v-if="(data[index].Short > 100)"><img :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Limit exceded"></div>
          <span class="huge-small">Short interruptions</span><br/>Less than 3 min and under 1% nominal voltage
          <progress-bar :value="data[index].Short * 100 / 100" :text="data[index].Short + ' events'"></progress-bar>
          <div>10 to 100 allowed per year</div>
        </div>
      </div>
    </div>
    <div class="col col-lg-3 col-md-6 col-xs-12">
      <div class="panel panel-other-light text-center">
        <div class="panel-body">
          <div class="float-right" v-if="(data[index].Long > 100)"><img :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Limit exceded"></div>
          <span class="huge-small">Long interruptions</span><br/>More than 3 min and under 1% nominal voltage
          <progress-bar :value="data[index].Long * 100 / 50" :text="data[index].Long + ' events'"></progress-bar>
          <div>10 to 50 allowed per year</div>
        </div>
      </div>
    </div>
    <div class="col col-lg-3 col-md-6 col-xs-12">
      <div class="panel panel-other-light text-center">
        <div class="panel-body">
          <div class="float-right" v-if="((data[index].HarmonicsIN / data[index].HarmonicsSamples) < 0.95)"><img :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Limit exceded"></div>
          <span class="huge-small">Harmonics</span><br/>Harmonics in valid ranges as detailed in EN50160
          <progress-bar :value="((data[index].HarmonicsIN / data[index].HarmonicsSamples) * 100 - 90) * 10" :text="Round((data[index].HarmonicsIN / data[index].HarmonicsSamples) * 100, 2) + '% of time in range'"></progress-bar>
          <div class="float-left">90%</div><div class="float-right">100%</div>
          <div class="clearfix"></div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    props: { data: Array, index: Number },
    data: function() {
      return { };
    }
  }
</script>

<style scoped>
  .panel {
    margin-bottom: 0px;
  }
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