<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      From {{ StartDate }} to {{ EndDate }}
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-xs btn-success" v-on:click="Edit(props.rowData)" :disable="props.rowData.Finished==false">Edit</button>
            <button type="button" class="btn btn-xs btn-default" v-on:click="RMS(props.rowData)" :disable="props.rowData.Finished==false">RMS</button>
            <button type="button" class="btn btn-xs btn-default" v-on:click="RAW(props.rowData)" :disable="props.rowData.Finished==false">Wave</button>
            <button type="button" class="btn btn-danger btn-xs" v-on:click="Delete(props.rowIndex)" :disable="props.rowData.Finished==false">Delete</button>
          </div>
        </template>
      </vuetable>
    </div>
    <event-dialog ref="event"></event-dialog>
  </div>  
</template>

<script>
  import Spinner from 'Components/Spinner.vue';
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  import EventDialog from './EventDialog.vue';
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
    },
    methods: {
      Redraw: function() {},
      GetEvents: function(to, period, lastsample) {
        var Analyzer = this.$root.SelectedAnalyzer;
        if(lastsample === 0) {
          this.ShowSpinner = true;
          this.Events = [];
          this.LastSample = 0;
        }
        $.ajax({ url: 'getEvents', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ To: to, LastSample: this.LastSample, Analyzer: Analyzer.ID, Period: period }),
          success: function(result) {
            this.EndDate = moment(result.To).format('DD/MM/YYYY - HH:mm:ss');
            this.StartDate = moment(result.From).format('DD/MM/YYYY - HH:mm:ss');
            if((result['StartTime'] === null) || (result['StartTime'].length === 0)) return;
            for(var x = 0; x < result['StartTime'].length; x++) {
              if(result['StartTime'][x] > this.LastSample) this.LastSample = result['StartTime'][x];
              var Event = { StartTime: result.StartTime[x], Date: moment(result.StartTime[x]).format('DD/MM/YYYY - HH:mm:ss'), Type: result.Type[x], Finished: result.Finished[x], Reference: result.Reference[x], PeakVoltage: result.Peak[x], ExitVoltage: result.Change[x], SampleDuration: result.Duration[x] };
              Event.Duration = (result.Duration === 0) ? 'RUNNNING' : oZm.Round2Plain(result.Duration[x] / result.SampleRate[x]);
              var Max = 0;        
              for(var i = 0; i < result.Reference[x].length; i++) {
                if(Math.abs(result.Reference[x][i] - result.Peak[x][i]) > Math.abs(Max)) Max = result.Peak[x][i] - result.Reference[x][i];
              }
             if(Event.Type === 'RVC') {
                var Diff = 0;
                for(var i = 0; i < result.Reference[x].length; i++) {
                  if(Math.abs(result.Reference[x][i] - result.Change[x][i]) > Math.abs(Diff)) Diff = result.Change[x][i] - result.Reference[x][i];
                }
                Event.Change = oZm.Round2Plain(Diff) + '<br>(' + oZm.Round2Plain(Max) + ' Max)';
                Event.Percent = oZm.Round2Plain(Diff / result.Reference[x][0] * 100.0);                
              } else {
                Event.Change = oZm.Round2Plain(Max);
                Event.Percent = oZm.Round2Plain(Max / result.Reference[x][0] * 100.0);
              }       
              this.Events.push(Event);
            }
            this.$refs.table.setData( { data: this.Events });
          }.bind(this),
          complete: function() {
            this.ShowSpinner = false;
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
        $.ajax({ url: 'delEvent', timeout: 20000, type: 'POST', dataType: 'text', contentType: 'application/json', data: JSON.stringify({ StartTime: this.Events[evt].StartTime, Analyzer: Analyzer.ID, Type: this.Events[evt].Type }),
          success: function(result) {
            this.Events.splice(evt, 1);
            this.$refs.table.setData( { data: this.Events });
          }.bind(this)
        });
      },
      Edit: function(evt) {}
    },
    components: { Spinner, Vuetable, EventDialog }
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
    width: 170px;
  } 
</style>
