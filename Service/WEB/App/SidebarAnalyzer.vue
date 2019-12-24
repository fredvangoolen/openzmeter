<template>
  <div class="this" v-bind:class="{ expanded: expanded}">
    <img class="icon" :src="require('Imgs/disconnect32.png')">
    <div class="name">{{ (analyzer == null) ? 'None selected' : analyzer.Name }}</div>
    <div class="btn-group">
      <button type="button" class="btn btn-default btn-xs" v-on:click="ShowEvents" data-toggle="tooltip" :title="(EventsCount == 0) ? 'No events since last login' : EventsCount + ' events since last login'"><img v-bind:class="{blink_img: (EventsCount != 0)}" :src="require('Imgs/exclamation.png')"></button>
      <button type="button" class="btn btn-default btn-xs" v-on:click="View = 'Table'" data-toggle="tooltip" title="Select and configure avaliable analyzers"><img :src="require('Imgs/magnifier.png')"></button>
    </div>
    <analyzer-table v-if="View == 'Table'" :analyzer="analyzer"></analyzer-table>
    <analyzer-configure v-if="View == 'Configure'" :value="Analyzer"></analyzer-configure>
    <analyzer-rates v-if="View == 'Rates'" :value="Analyzer"></analyzer-rates>
    <analyzer-rate v-if="View == 'Rate'" :value="Rate" :tariffs="Tariffs" :analyzer="Analyzer"></analyzer-rate>
    <analyzer-maintenance v-if="View == 'Maintenance'" ref="Maintenance"></analyzer-maintenance>
    <analyzer-alarms v-if="View == 'Alarms'"></analyzer-alarms>
    <analyzer-alarm v-if="View == 'Alarm'" ref="Alarm"></analyzer-alarm>
  </div> 
</template> 

<script>
  import ProgressBar from 'Components/ProgressBar.vue';
  import AnalyzerTable from './Analyzer/Table.vue';
  import AnalyzerConfigure from './Analyzer/Configure.vue';
  import AnalyzerRates from './Analyzer/Rates.vue';
  import AnalyzerRate from './Analyzer/Rate.vue';
  import AnalyzerMaintenance from './Analyzer/Maintenance.vue';
  import AnalyzerAlarms from './Analyzer/Alarms.vue';
  import AnalyzerAlarm from './Analyzer/Alarm.vue';
  import moment from 'moment';
  export default {
    props: { analyzer: { type: Object }, expanded: Boolean },
    data() {
      return { View: null, SelAnalyzer: null, EventsCount: 0, Analyzer: null, Rate: null,  Tariffs: null };
    },
    created: function() {
      this.LastSample = 0;
      this.Timer = null;
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
      if(localStorage.getItem("SelAnalyzer") === undefined) {
        this.View = 'Table';
      } else {
        $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: localStorage.getItem("SelAnalyzer") }), 
          success: (result) => { 
            this.$parent.$parent.Analyzer = result;
            this.$root.SelectedAnalyzer = result;
              this.$root.$emit('AnalyzerChanged', result);
            },
            error: () => {
              this.$root.$emit('AnalyzerChanged', null);
              this.View = 'Table';
            }
          });
        }

  
  },
    beforeDestroy: function () {
      clearInterval(this.Timer);
    },
    methods: {
      SetAnalyzer: function(analyzer) {
        this.SelAnalyzer = analyzer;
        this.LastSample = 0;
        this.EventsCount = 0;
        this.GetEvents();
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
    components: { ProgressBar, AnalyzerTable, AnalyzerConfigure, AnalyzerRates, AnalyzerRate, AnalyzerMaintenance, AnalyzerAlarms, AnalyzerAlarm }
  }
</script>

<style scoped>
  .this {
    background-color: #474544;
    color: #FAFAFA;
    width: 304px;
    height: 32px;
    display: flex;
    margin-left: 0;
    transition: margin-left 0.2s ease-in-out;
    border-bottom: 1px solid #333;
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
    padding-left: 5px;
    line-height: 24px;
    padding-top: 4px;
  }
  .btn-group {
    margin-top: 4px;
  }
  .blink_img {
    animation: blinker_img 1s linear infinite;
  }
  @keyframes blinker_img {  
    20% { opacity: 0.2; }
  }
</style>
