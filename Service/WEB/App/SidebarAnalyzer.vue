<template>
  <div class="this" v-bind:class="{ expanded: expanded }">
    <img class="icon" :src="require('Imgs/disconnect32.png')">
    <div class="btn-group">
      <button type="button" class="btn btn-default btn-xs" v-on:click="ShowEvents" data-toggle="tooltip" :title="(EventsCount == 0) ? 'No events since last login' : EventsCount + ' events since last login'"><img v-bind:class="{blink_img: (EventsCount != 0)}" :src="require('Imgs/exclamation.png')"></button>
      <button type="button" class="btn btn-default btn-xs" @click="ShowDialog=true" data-toggle="tooltip" title="Select and configure avaliable analyzers"><img :src="require('Imgs/magnifier.png')"></button>
    </div>
    <div class="name">{{ (analyzer == null) ? 'None selected' : analyzer.Name }}</div>
    <analyzer-table v-if="ShowDialog" :analyzer="analyzer" @update:analyzer="SetAnalyzer($event)" @close="ShowDialog=false"></analyzer-table>
  </div>
</template>

<script>
  import AnalyzerTable from './Analyzer/Table.vue';
  import moment from 'moment';
  import 'moment-timezone';
  export default {
    props: { analyzer: Object, expanded: Boolean },
    data() {
      return { ShowDialog: false, EventsCount: 0 };
    },
    created: function() {
      if(localStorage.getItem("Analyzer") == null) {
        this.SetAnalyzer(null);
        this.ShowDialog = true;
      } else {
        this.$http.post('getAnalyzer', { Analyzer: localStorage.getItem("Analyzer") }).then(
          (result) => {
            result.data.Analyzer = localStorage.getItem("Analyzer");
            this.SetAnalyzer(result.data);
          },
          (fail) => {
            this.SetAnalyzer(null);
            this.ShowDialog = true;
          }
        );
      }
    },
    methods: {
      SetAnalyzer: function(analyzer) {
        if(analyzer != null) {
          analyzer.PhaseNames = ['R', 'S', 'T', 'N'];
          analyzer.VoltageChannels = (analyzer.Mode == '1PHASE') ? 1 : 3;
          analyzer.CurrentChannels = (analyzer.Mode == '1PHASE') ? 1 : (analyzer.Mode == '3PHASE') ? 3 : 4;
          analyzer.Color = { Voltage: '#d9534f', Current: '#428bca', Active_Power: '#f0ad4e', Reactive_Power: '#DF842A', Active_Energy: '#3d3d8b', Reactive_Energy: '#643d8b', Frequency: '#5cb85c' };
          analyzer.Unit = { Voltage: 'V', Current: 'A', Active_Power: 'W', Active_Energy: 'Wh', Frequency: 'Hz'};
          analyzer.HarmonicsNames = ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th', '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th', '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th', '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th'];
          analyzer.GetSeriesNames = this.GetSeriesNames.bind(this);
          analyzer.FormatAggregation = this.FormatAggregation.bind(this);
          analyzer.FormatTime = this.FormatTime.bind(this);
          analyzer.FormatTimeMS = this.FormatTimeMS.bind(this);
          analyzer.FormatDateTime = this.FormatDateTime.bind(this);
        }
        this.$emit('update:analyzer', analyzer);
//        this.LastSample = 0;
//        this.EventsCount = 0;
//        this.GetEvents();
      },
      GetSeriesNames: function(serie, len) {
        if(len == 1) return [serie];
        var Result = [];
        for(var n = 0; n < len; n++) Result.push(serie + ' (' + this.analyzer.PhaseNames[n] + ')');
        return Result;
      },
      FormatAggregation: function(val) {
        return (val == '1H') ? '1 hour' : (val == '15M') ? '15 minutes' : (val == '10M') ? '10 minutes' : (val == '1M') ? '1 minute' : (val == '3S') ? '3 seconds' : 'unknown';
      },
      FormatTime: function(val) {
        return moment(val).tz(this.analyzer.TimeZone).format('HH:mm:ss');
      },
      FormatTimeMS: function(val) {
        return moment(val).tz(this.analyzer.TimeZone).format('HH:mm:ss.SSS');
      },
      FormatDateTime: function(val, spacer) {
        if(spacer == null) spacer = ' - ';
        return moment(val).tz(this.analyzer.TimeZone).format('hh:mm:ss') + spacer + moment(val).tz(this.analyzer.TimeZone).format('DD/MM/YYYY');
      },








      ShowEvents: function() {
        localStorage.setItem('Period', JSON.stringify({Period: moment().valueOf() - parseInt(localStorage.getItem('LastLogin')), To: moment().valueOf() }));
        this.$router.push({path: '/Events/Disturbances'});
      },
      GetEvents: function() {
        clearInterval(this.Timer);
        var Analyzer = this.$root.SelectedAnalyzer;
        if(Analyzer === null) return;
        $.ajax({ url: 'getEvents', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ To: moment().valueOf(), LastSample: this.LastSample, Analyzer: Analyzer.Analyzer, Period: moment().valueOf() - parseInt(localStorage.getItem('LastLogin')) }),
          success: function(result) {
            if(result.StartTime.length > 0) {
              this.EventsCount += result.StartTime.length;
              this.LastSample = result.StartTime.slice(-1)[0];
            }
          }.bind(this),
          complete: function() {
            this.Timer = setTimeout(this.GetEvents.bind(this), 60000);
          }.bind(this)
        });
      }
    },
    components: { AnalyzerTable }
  }
</script>

<style scoped>
  .this {
    background-color: #474544;
    color: #FAFAFA;
    width: 304px;
    height: 33px;
    display: flex;
    margin-left: 0;
    transition: margin-left 0.2s ease-in-out;
    border-bottom: 1px solid #333;
    border-top: 1px solid #333;
  }
  .this.expanded {
    margin-left: -48px;
    transition: margin-left 0.2s ease-in-out;
  }
  .icon {
    height: 24px;
    width: 24px;
    margin: 4px 12px;
  }
  .name {
    width: 192px;
    padding-left: 2px;
    line-height: 24px;
    padding-top: 4px;
  }
  .btn-group {
    margin-top: 4px;
    margin-left: -2px;
  }
  .blink_img {
    animation: blinker_img 1s linear infinite;
  }
  .btn-group > .btn {
    background: #444;
    border-color: #333;
    padding: 6px 6px 7px 6px;
    margin-top: -5px;
    border-radius: 0px;
    margin-left: -1px;
  }
  .btn-group > .btn:hover {
    background: #555;
    border-color: #333;
  }
  @keyframes blinker_img {
    20% { opacity: 0.2; }
  }
</style>
