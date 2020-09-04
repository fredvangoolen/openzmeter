<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <span class="header-text">ITIC Curve (CBEMA)</span>
            <div class="header-buttons">
              <toolbar-years :analyzer="analyzer" v-model="Year" storage="StatYear" aggreg="1H"></toolbar-years>
              <div class="btn-group">
                <button-export file="Events" :rows="Export"></button-export>
              </div>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="TransferEvents.Data==null"></spinner>
            <curve :data="TransferEvents.Data" :analyzer="analyzer"></curve>
          </div>
        </div>
      </div>
    </div>
    <div class="row">
      <div class="col-xs-12 col-md-6">
        <by-date :data="TransferEvents.Data" :analyzer="analyzer"></by-date>
      </div>
      <div class="col-xs-12 col-md-6">
        <by-type :data="TransferEvents.Data" :analyzer="analyzer"></by-type>
      </div>
      <div class="col-xs-12 col-md-6">
        <by-rms :data="TransferEvents.Data" :analyzer="analyzer"></by-rms>
      </div>
      <div class="col-xs-12 col-md-6">
        <by-duration :data="TransferEvents.Data" :analyzer="analyzer"></by-duration>
      </div>
    </div>
  </div>
</template>

<script>
  import Curve from './Curve.vue';
  import ByDate from './ByDate.vue';
  import ByType from './ByType.vue';
  import ByRms from './ByRMS.vue';
  import ByDuration from './ByDuration.vue';
  import Graph from 'Components/Graph.vue';
  import moment from 'moment';
  export default {
    props: { analyzer: Object },
    data: function() {
      return { Year: null, TransferEvents: this.EventsTransfer(this) };
    },
    created: function() {
      this.TransferEvents.SingleShot = true;
    },
    watch: {
      analyzer: function() { this.TransferEvents.Update(this.analyzer, [moment().tz(this.analyzer.TimeZone).year(val).endOf('year').valueOf(), moment().tz(this.analyzer.TimeZone).year(val).startOf('year').valueOf()]); },
      Year: function(val) { this.TransferEvents.Update(this.analyzer, [moment().tz(this.analyzer.TimeZone).year(val).endOf('year').valueOf(), moment().tz(this.analyzer.TimeZone).year(val).startOf('year').valueOf()]); }
    },
    beforeDestroy: function() {
      this.TransferEvents.Destroy();
    },
    methods: {
      Export: function() {
        if(this.Data == null) return [];
        var Data =  [['Start', 'Type', 'Duration', 'Notes']];
        for(var i in this.Data.StartTime) Data.push([this.Data.StartTime[i], this.Data.Type[i], this.Round(this.Data.Duration[i] / this.Data.SampleRate[i], 3), this.Data.Notes[i]]);
        return Object.seal(Data);
      }
    },
    components: { Curve, ByDate, ByType, ByRms, ByDuration }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>