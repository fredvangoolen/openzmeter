<template>
  <q-page>
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <span class="header-text">ITIC Curve (CBEMA)</span>
            <div class="header-buttons">
              <toolbar-years v-model="Year" storage="StatYear" aggreg="1H"></toolbar-years>
              <div class="btn-group">
                <button-export file="Events" :rows="Export"></button-export>
              </div>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="TransferEvents.Data==null"></spinner>
            <curve :data="TransferEvents.Data"></curve>
          </div>
        </div>
      </div>
    </div>
    <div class="row">
      <div class="col-xs-12 col-md-6">
        <by-date :data="TransferEvents.Data"></by-date>
      </div>
      <div class="col-xs-12 col-md-6">
        <by-type :data="TransferEvents.Data"></by-type>
      </div>
      <div class="col-xs-12 col-md-6">
        <by-rms :data="TransferEvents.Data"></by-rms>
      </div>
      <div class="col-xs-12 col-md-6">
        <by-duration :data="TransferEvents.Data"></by-duration>
      </div>
    </div>
  </q-page>
</template>

<script>
  import Curve from './Curve.vue';
  import ByDate from './ByDate.vue';
  import ByType from './ByType.vue';
  import ByRms from './ByRMS.vue';
  import ByDuration from './ByDuration.vue';
  import moment from 'moment';
  export default {
    data: function() {
      return { Year: null, TransferEvents: this.EventsTransfer() };
    },
    watch: {
      analyzer: function() { if(this.Year == null) return; this.TransferEvents.Update(this.analyzer, [moment().tz(this.analyzer.TimeZone).year(this.Year).startOf('year').valueOf(), moment().tz(this.analyzer.TimeZone).year(this.Year).endOf('year').valueOf()]); },
      Year: function(val) { if(this.analyzer == null) return ; this.TransferEvents.Update(this.analyzer, [moment().tz(this.analyzer.TimeZone).year(val).startOf('year').valueOf(), moment().tz(this.analyzer.TimeZone).year(val).endOf('year').valueOf()]); }
    },
    beforeDestroy: function() {
      this.TransferEvents.Destroy();
    },
    methods: {
      Export: function() {
        if(this.TransferEvents.Data == null) return [];
        var Data =  [['Start', 'Type', 'Duration', 'Notes']];
        var StartCol = this.TransferEvents.Data.Headers.indexOf('StartTime');
        var TypeCol  = this.TransferEvents.Data.Headers.indexOf('Type');
        var DurationCol = this.TransferEvents.Data.Headers.indexOf('Duration');
        var SamplerateCol = this.TransferEvents.Data.Headers.indexOf('SampleRate');
        var NotesCol = this.TransferEvents.Data.Headers.indexOf('Notes');
        for(var i = 0; i < this.TransferEvents.Data.Rows.length; i++) Data.push([this.analyzer.FormatDateTime(this.TransferEvents.Data.Rows[i][StartCol]), this.TransferEvents.Data.Rows[i][TypeCol], this.Round(this.TransferEvents.Data.Rows[i][DurationCol] / this.TransferEvents.Data.Rows[i][SamplerateCol], 3), this.TransferEvents.Data.Rows[i][NotesCol]]);
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