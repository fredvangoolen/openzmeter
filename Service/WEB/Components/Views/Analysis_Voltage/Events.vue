<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">From {{ StartDate }} to {{ EndDate }}</div>
      <div class="header-buttons clearfix" ref="Toolbar">
        <toolbar-tools variable="To" ref="tools" :options="['EXPORT']" v-on:ExportClick="ExportClick"></toolbar-tools>
      </div>  
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-xs btn-success" v-on:click="Edit(props.rowData)" :disable="props.rowData.Finished==false">Add notes</button>
            <button type="button" class="btn btn-xs btn-default" v-on:click="RMS(props.rowData)" :disable="props.rowData.Finished==false">RMS</button>
            <button type="button" class="btn btn-xs btn-default" v-on:click="RAW(props.rowData)" :disable="props.rowData.Finished==false">Wave</button>
            <button type="button" class="btn btn-danger btn-xs" v-on:click="Delete(props.rowIndex)" :disable="props.rowData.Finished==false">Delete</button>
          </div>
        </template>
      </vuetable>
    </div>
    <event-dialog ref="event"></event-dialog>
    <event-notes ref="eventNotes"></event-notes>
  </div>  
</template>

<script>
  import Spinner from 'Components/Spinner.vue';
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  import EventDialog from './EventDialog.vue';
  import EventNotes from './EventNotes.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  export default {
    data: function() {
      return { ShowSpinner: true, StartDate: '', EndDate: '',
        Columns: [
          {name: 'Date', title: 'Date'},
          {name: 'Type', title: 'Type'}, 
          {name: 'Duration', title: 'Duration <em>(s)</em>'},
          {name: 'Change', title: 'ΔU <em>(V)</em>'},
          {name: 'Percent', title: 'ΔU <em>(%)</em>'},
          {name: 'Notes', title: 'Notes'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    created: function() {
      this.Events = [];
      this.LastSample = 0;
      this.DateWindow = null;
    },
    methods: {
      Redraw: function() {},
      GetEvents: function(to, period, lastsample) {
        var Analyzer = this.$root.SelectedAnalyzer;
        if(lastsample === 0) {
          this.ShowSpinner = true;
          this.Events = [];
          this.LastSample = 0;
          this.$refs.table.tableData = [];
        }
        $.ajax({ url: 'getEvents', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ To: to, LastSample: this.LastSample, Analyzer: Analyzer.Analyzer, Period: period }),
          success: function(result) {
            this.DateWindow = [result.From, result.To];
            this.EndDate = moment(result.To).format('DD/MM/YYYY - HH:mm:ss');
            this.StartDate = moment(result.From).format('DD/MM/YYYY - HH:mm:ss');
            if((result['StartTime'] === null) || (result['StartTime'].length === 0)) return;
            for(var x = 0; x < result['StartTime'].length; x++) {
              if(result['StartTime'][x] > this.LastSample) this.LastSample = result['StartTime'][x];
              var Event = { StartTime: result.StartTime[x], Date: moment(result.StartTime[x]).format('DD/MM/YYYY - HH:mm:ss'), Type: result.Type[x], Finished: result.Finished[x], Reference: result.Reference[x], PeakVoltage: result.Peak[x], ExitVoltage: result.Change[x], SampleDuration: result.Duration[x], SampleRate: result.SampleRate[x], Notes: result.Notes[x] };
              Event.Duration = (result.Duration === 0) ? 'RUNNNING' : oZm.Round2Plain(result.Duration[x] / result.SampleRate[x]);
              var Max = 0;        
              for(var i = 0; i < result.Reference[x].length; i++) {
                if(Math.abs(result.Reference[x][i] - result.Peak[x][i]) > Math.abs(Max)) Max = result.Peak[x][i] - result.Reference[x][i];
              }
              if(Event.Type === 'RVC') {
                Event.Diff = 0;
                for(var i = 0; i < result.Reference[x].length; i++) {
                  if(Math.abs(result.Reference[x][i] - result.Change[x][i]) > Math.abs(Event.Diff)) Event.Diff = result.Change[x][i] - result.Reference[x][i];
                }
                Event.Change = oZm.Round2Plain(Event.Diff) + '<br>(' + oZm.Round2Plain(Max) + ' Max)';
                Event.Percent = oZm.Round2Plain(Event.Diff / result.Reference[x][0] * 100.0);                
              } else {
                Event.Diff = Max;
                Event.Change = oZm.Round2Plain(Max);
                Event.Percent = oZm.Round2Plain(Max / result.Reference[x][0] * 100.0);
              }       
              this.Events.push(Event);
            }
            this.$refs.table.tableData = this.Events;
          }.bind(this),
          complete: function() {
            this.ShowSpinner = false;
            this.$emit("UpdateComplete");
          }.bind(this),
          error: function(response) {
            this.ShowSpinner = true;
          }.bind(this)
        });
      },
      RMS: function(evt) {
        this.$refs.event.Show(evt, 'RMS');
      },
      RAW: function(evt) {
        this.$refs.event.Show(evt, 'RAW');
      },
      Delete: function(evt) {
        var Analyzer = this.$root.SelectedAnalyzer;
        if(confirm("Delete selected event?")){
          $.ajax({ url: 'delEvent', timeout: 20000, type: 'POST', dataType: 'text', contentType: 'application/json', data: JSON.stringify({ StartTime: this.Events[evt].StartTime, Analyzer: Analyzer.Analyzer, Type: this.Events[evt].Type }),
            success: function(result) {
              this.Events.splice(evt, 1);
              this.$refs.table.setData( { data: this.Events });
            }.bind(this)
          });
        }
      },
      Edit: function(evt) {
        this.$refs.eventNotes.Show(evt);
      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Export = [['Date', 'Type', 'Duration', 'ΔU (V)', 'Notes']];
        for(var i = 0; i < this.Events.length; i++) Export.push([this.Events[i].Date, this.Events[i].Type, this.Events[i].SampleDuration / this.Events[i].SampleRate, this.Events[i].Diff, this.Events[i].Notes]);          
        this.$refs.tools.SaveCVS('Events.csv', Export);
      }
    },
    components: { Spinner, Vuetable, EventDialog, EventNotes, ToolbarTools }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1) {
      width: 160px;
    } 
    .table >>> th:nth-child(2) {
      width: 60px;
      text-align: center;
    }
    .table >>> td:nth-child(2) {
      text-align: center;
    }
    .table >>> th:nth-child(3), .table >>> th:nth-child(4), .table >>> th:nth-child(5) {
      width: 100px;
      text-align: right;
    }
    .table >>> td:nth-child(3), .table >>> td:nth-child(4), .table >>> td:nth-child(5) {
      text-align: right;
    }
    .table >>> th:nth-child(7) {
      width: 200px;
    }
  }
</style>
