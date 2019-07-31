<template>
  <div>
    <div class="sidenav-header">
      <div>{{ analyzerName }}</div>
      <div class="btn-group pull-right">
        <button type="button" class="btn btn-default btn-xs" :disabled="EventsCount == 0" @click="ShowEvents" data-toggle="tooltip" :title="(EventsCount == 0) ? 'No events since last login' : EventsCount + ' events since last login'"><img v-bind:class="{blink_img: (EventsCount != 0)}" :src="require('Imgs/exclamation.png')"></button>
        <button type="button" class="btn btn-default btn-xs" :disabled="!hasAnalyzers" @click="ShowDialog" data-toggle="tooltip" title="Select and configure avaliable analyzers"><img :src="require('Imgs/magnifier.png')"></button>
      </div>
      <div class="clearfix"></div>
    </div>
    <analyzer-dialog ref="Dialog"></analyzer-dialog>
  </div> 
</template> 

<script>
  import ProgressBar from 'Components/ProgressBar.vue';
  import AnalyzerDialog from './Analyzer/Analyzer.vue';
  import moment from 'JS/moment.js';
  export default {
    data() {
      return { AvaliableAnalyzers: [], hasAnalyzers: false, SelAnalyzer: null, analyzerName: 'None selected', EventsCount: 0 };
    },
    created: function() {
      this.LastSample = 0;
      this.Timer = null;
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      this.GetAnalyzers();
    },
    beforeDestroy: function () {
      clearInterval(this.Timer);
    },
    methods: {
      SetAnalyzer: function() {
        if(this.$root.SelectedAnalyzer === null) {
          this.analyzerName = 'None selected';
        } else {
          this.analyzerName = this.$root.SelectedAnalyzer.Name;
          this.LastSample = 0;
          this.EventsCount = 0;
          this.GetEvents();
        }
      },
      ShowEvents: function() {
        localStorage.setItem('Period', JSON.stringify({Period: moment().valueOf() - parseInt(localStorage.getItem('LastLogin')), To: moment().valueOf() }));
        this.$router.push({path: '/Events/Disturbances'});
      },
      GetEvents: function() {
        clearInterval(this.Timer);
        var Analyzer = this.$root.SelectedAnalyzer;
        if(Analyzer == null) return;
        $.ajax({ url: 'getEvents', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ To: moment().valueOf(), LastSample: this.LastSample, Analyzer: Analyzer.ID, Period: moment().valueOf() - parseInt(localStorage.getItem('LastLogin')) }),
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
      },
      GetAnalyzers() {
        $.ajax({ url: "getAnalyzers", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) { this.AvaliableAnalyzers = result; this.CheckSelected(); }.bind(this)
        });
      },
      ShowDialog() {
        this.$refs.Dialog.Show(this.AvaliableAnalyzers);
      },
      CheckSelected() {
        if(this.AvaliableAnalyzers.length === 0) {
          this.ShowDialog();
          this.hasAnalyzers = false;
        } else {
          this.hasAnalyzers = true;
          for(var i = 0; i < this.AvaliableAnalyzers.length; i++) {
            if(this.AvaliableAnalyzers[i].ID === localStorage.getItem("SelAnalyzer")) this.SelAnalyzer = this.AvaliableAnalyzers[i];
          }
          if(this.SelAnalyzer === null) this.ShowDialog();
          this.$root.SelectedAnalyzer = this.SelAnalyzer;  
          this.$root.$emit('AnalyzerChanged');
        }
      }
    },
    components: { ProgressBar, AnalyzerDialog }
  }
</script>

<style scoped>
  .sidenav-header {
    background-color: #474544;
  }
  .sidenav-header td {
    text-align: center;
  }
  .sidenav-header > table {
    width: 100%;
    font-size: 10px;
    line-height: 12px;
    margin-top: 4px;
    margin-bottom: 4px;
  }
  .sidenav-header > table img {
    margin-right: 1px;
  }
  .sidenav-header > div:first-child {
    width: 186px;
    display: inline-block;
  }
  .blink_img {
    animation: blinker_img 1s linear infinite;
  }
  @keyframes blinker_img {  
    20% { opacity: 0.2; }
  }
</style>