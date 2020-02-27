<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Last events</div>
      <div class="header-buttons clearfix">
        <toolbar-pager></toolbar-pager>
        <div class="btn-group">
          <button class="btn btn-default btn-xs btn-danger" type="button" @click="DeleteSelected">Delete selection</button>
          <button-export file="Events" :rows="[]"></button-export>
        </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="events==null"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" :data="Events" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-xs btn-success" @click="Edit(props.rowData)" :disable="props.rowData.Finished==false">Add notes</button>
            <button type="button" class="btn btn-xs btn-default" @click="RMS(props.rowData)" :disable="props.rowData.Finished==false">RMS</button>
            <button type="button" class="btn btn-xs btn-default" @click="RAW(props.rowData)" :disable="props.rowData.Finished==false">Wave</button>
            <button type="button" class="btn btn-danger btn-xs"  @click="Delete(props.rowIndex)" :disable="props.rowData.Finished==false">Delete</button>
          </div>
        </template>
      </vuetable>
    </div>
    <!--event-dialog ></event-dialog>
    <event-notes ref="eventNotes"></event-notes-->
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  import EventDialog from './EventDialog.vue';
  import EventNotes from './EventNotes.vue';
  export default {
    props: { analyzer: Object, events: Object },
    data: function() {
      return { Columns: [
                {name: '__checkbox'},
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
    computed: {
      Events: function() {
        var Events = [];
        if(this.events == null) return [];
        for(var x = 0; x < this.events.StartTime.length; x++) {
          var Event = { StartTime: this.events.StartTime[x], 
                        Date: this.analyzer.FormatDateTime(this.events.StartTime[x]), 
                        Type: this.events.Type[x], 
                        Finished: this.events.Finished[x], 
                        Reference: this.events.Reference[x], 
                        PeakVoltage: this.events.Peak[x], 
                        ExitVoltage: this.events.Change[x], 
                        SampleDuration: this.events.Duration[x], 
                        SampleRate: this.events.SampleRate[x], 
                        Notes: this.events.Notes[x],
                        Duration: (this.events.Duration == 0) ? 'RUNNNING' : oZm.Round2Plain(this.events.Duration[x] / this.events.SampleRate[x])
          };
          var Max = 0;
          for(var i = 0; i < this.events.Reference[x].length; i++) {
            if(Math.abs(this.events.Reference[x][i] - this.events.Peak[x][i]) > Math.abs(Max)) Max = this.events.Peak[x][i] - this.events.Reference[x][i];
          }
              if(Event.Type === 'RVC') {
                Event.Diff = 0;
                for(var i = 0; i < this.events.Reference[x].length; i++) {
                  if(Math.abs(this.events.Reference[x][i] - this.events.Change[x][i]) > Math.abs(Event.Diff)) Event.Diff = this.events.Change[x][i] - this.events.Reference[x][i];
                }
                Event.Change = oZm.Round2Plain(Event.Diff) + '<br>(' + oZm.Round2Plain(Max) + ' Max)';
                Event.Percent = oZm.Round2Plain(Event.Diff / this.events.Reference[x][0] * 100.0);
              } else {
                Event.Diff = Max;
                Event.Change = oZm.Round2Plain(Max);
                Event.Percent = oZm.Round2Plain(Max / this.events.Reference[x][0] * 100.0);
              }
              Events.push(Event);
            }
            return Events;
      }
    },
    methods: {
RMS: function(evt) {
        this.$refs.event.Show(evt, 'RMS');
      },
      RAW: function(evt) {
        this.$refs.event.Show(evt, 'RAW');
      },
      DeleteSelected: function(evt) {
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
    components: { Vuetable, EventDialog, EventNotes }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1) {
      width: 30px;
    }    
    .table >>> th:nth-child(2) {
      width: 160px;
    }
    .table >>> th:nth-child(3) {
      width: 60px;
      text-align: center;
    }
    .table >>> td:nth-child(3) {
      text-align: center;
    }
    .table >>> th:nth-child(4), .table >>> th:nth-child(5), .table >>> th:nth-child(6) {
      width: 100px;
      text-align: right;
    }
    .table >>> td:nth-child(4), .table >>> td:nth-child(5), .table >>> td:nth-child(6) {
      text-align: right;
    }
    .table >>> th:nth-child(8) {
      width: 200px;
    }
  }
</style>
