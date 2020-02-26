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
            <spinner v-if="Data==null"></spinner>
            <curve :data="Data" :analyzer="analyzer"></curve>
          </div>
        </div>
      </div>
    </div>
    <div class="row">
      <div class="col-xs-12 col-md-6">
        <by-date :data="Data" :analyzer="analyzer"></by-date>
      </div>
      <div class="col-xs-12 col-md-6">
        <by-type :data="Data" :analyzer="analyzer"></by-type>
      </div>
      <div class="col-xs-12 col-md-6">
        <by-rms :data="Data" :analyzer="analyzer"></by-rms>
      </div>
      <div class="col-xs-12 col-md-6">
        <by-duration :data="Data" :analyzer="analyzer"></by-duration>
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
      return { Year: null, Data: null };
    },
    watch: {
      Year: function(val) {
        this.Data = null;
        var To   = moment().tz(this.analyzer.TimeZone).year(val).endOf('year').valueOf();
        var From = moment().tz(this.analyzer.TimeZone).year(val).startOf('year').valueOf();
        this.$http.post('getEvents', { From: From, To: To, Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.Data = Object.seal(result.data);
          },
          (fail) => {}
        );
      }
    },
    computed: {
      Export: function() {
        if(this.Data == null) return [];
        var Data =  [['Start', 'Type', 'Duration', 'Notes']];
        for(var i in this.Data.StartTime) Data.push([this.Data.StartTime[i], this.Data.Type[i], this.Data.Duration[i] / this.Data.SampleRate[i], this.Data.Notes[i]]);
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