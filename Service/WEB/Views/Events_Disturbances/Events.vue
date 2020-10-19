<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Last events</div>
      <div class="header-buttons clearfix">
        <toolbar-pager :pageSize="10" :input="Events" :output.sync="FilterEvents" :filtercols="['Date', 'Type', 'Notes']"></toolbar-pager>
        <div class="btn-group">
          <button class="btn btn-default btn-xs btn-danger" type="button" @click="DeleteSelected">Delete selection</button>
          <button-export file="Events" :rows="Export"></button-export>
        </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="(events==null)||ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" :data="FilterEvents" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-xs btn-success" @click="Edit(props.rowData)">Add notes</button>
            <button type="button" class="btn btn-xs btn-default" @click="RMS(props.rowData)">RMS</button>
            <button type="button" class="btn btn-xs btn-default" @click="RAW(props.rowData)">Wave</button>
            <button type="button" class="btn btn-danger btn-xs"  @click="Delete(props.rowData)">Delete</button>
          </div>
        </template>
      </vuetable>
    </div>
    <event-dialog v-if="(View=='RMS')||(View=='RAW')" :mode="View" :analyzer="analyzer" :event="Event" @close="View=null"></event-dialog>
    <event-notes v-if="View=='Notes'" :analyzer="analyzer" :event="Event" @close="UpdateEvent"></event-notes>
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
      return { Event: null, View: null, Events: [], FilterEvents: [], ShowSpinner: false,
               Columns: [
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
    updated: function() {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    watch: {
      events: function() {
        var Events = [];
        if(this.events == null) return [];
        var IDCol         = this.events.Headers.indexOf("EventID");
        var StartTimeCol  = this.events.Headers.indexOf("Time");
        var SampleRateCol = this.events.Headers.indexOf("SampleRate");
        var DurationCol   = this.events.Headers.indexOf("Duration");
        var TypeCol       = this.events.Headers.indexOf("Type");
        var ReferenceCol  = this.events.Headers.indexOf("Reference");
        var PeakCol       = this.events.Headers.indexOf("Peak");
        var ChangeCol     = this.events.Headers.indexOf("Change");
        var NotesCol      = this.events.Headers.indexOf("Notes");
        for(var x = 0; x < this.events.Rows.length; x++) {
          var Event = { id: this.events.Rows[x][IDCol],
                        StartTime: this.events.Rows[x][StartTimeCol],
                        Date: this.analyzer.FormatDateTime(this.events.Rows[x][StartTimeCol]),
                        Type: this.events.Rows[x][TypeCol],
                        Reference: this.events.Rows[x][ReferenceCol],
                        PeakVoltage: this.events.Rows[x][PeakCol],
                        ExitVoltage: this.events.Rows[x][ChangeCol],
                        SampleDuration: this.events.Rows[x][DurationCol],
                        SampleRate: this.events.Rows[x][SampleRateCol],
                        Notes: this.events.Rows[x][NotesCol],
                        Duration: (this.events.Rows[x][DurationCol] == 0) ? 'RUNNNING' : this.Round(this.events.Rows[x][DurationCol] / this.events.Rows[x][SampleRateCol], 2, false)
          };
          var Max = 0;
          for(var i = 0; i < this.events.Rows[x][ReferenceCol].length; i++) {
            if(Math.abs(this.events.Rows[x][ReferenceCol][i] - this.events.Rows[x][PeakCol][i]) > Math.abs(Max)) Max = this.events.Rows[x][PeakCol][i] - this.events.Rows[x][ReferenceCol][i];
          }
          if(Event.Type === 'RVC') {
            Event.Diff = 0;
            for(var i = 0; i < this.events.Rows[x][ReferenceCol].length; i++) {
              if(Math.abs(this.events.Reference[x][i] - this.events.Rows[x][ChangeCol][i]) > Math.abs(Event.Diff)) Event.Diff = this.events.Rows[x][ChangeCol][i] - this.events.Rows[x][ReferenceCol][i];
            }
            Event.Change = this.Round(Event.Diff, 2, false) + '<br>(' + this.Round(Max, 2, false) + ' Max)';
            Event.Percent = this.Round(Event.Diff / this.events.Rows[x][ReferenceCol][0] * 100.0, 2, false);
          } else {
            Event.Diff = Max;
            Event.Change = this.Round(Max, 2, false);
            Event.Percent = this.Round(Max / this.events.Rows[x][ReferenceCol][0] * 100.0, 2, false);
          }
          Events.push(Event);
        }
        this.Events = Events;
      }
    },
    methods: {
      Edit: function(evt) {
        this.Event = evt;
        this.View = 'Notes';
      },
      UpdateEvent: function(update) {
        this.View = null;
        if(update) this.$emit('eventschanged');
      },
      RMS: function(evt) {
        this.ShowSpinner = true;
        this.$http.post('getEvent', { EventID: evt.id, Analyzer: this.analyzer.Analyzer, Samples: 'RMS' }).then(
          (result) => {
            this.ShowSpinner = false;
            this.Event = Object.freeze(result.body);
            this.View = 'RMS';
          },
          (fail) => {
            this.ShowSpinner = false;
            alert("Event samples are not avaliable now. Maybe event is still running or deleted.");
          }
        );
      },
      CyclicHistogram: function(evt){
        this.ShowSpinner = true;
        this.$http.post('getCyclicHistogram', { StartTime: evt.StartTime, Analyzer: this.analyzer.Analyzer, Type: evt.Type }).then(
          (result) => {
            this.ShowSpinner = false;
            this.$router.push({name: 'CyclicHistogram' , params: {id: evt.StartTime}});
          },
          (fail) => {
            this.ShowSpinner = false;
            alert("Event samples are not avaliable now. Maybe event is still running or deleted.");
          }
        );

        this.Event = evt;
        this.View = 'About';
        return;
      },
      RAW: function(evt) {
        this.ShowSpinner = true;
        this.$http.post('getEvent', { EventID: evt.id, Analyzer: this.analyzer.Analyzer, Samples: 'RAW' }).then(
          (result) => {
            this.ShowSpinner = false;
            this.Event = Object.freeze(result.body);
            this.View = 'RAW';
          },
          (fail) => {
            this.ShowSpinner = false;
            alert("Event samples are not avaliable now. Maybe event is still running or deleted.");
          }
        );
      },
      DeleteSelected: function(evt) {
        var Selection = this.$refs.table.selectedTo;
        if(confirm('Delete ' + Selection.length + ' event(s)?') == false) return;
        this.ShowSpinner = true;
        for(var n = 0; n < Selection.length; n++) {
          this.$http.post('delEvent', { EventID: Selection[n], Analyzer: this.analyzer.Analyzer });
          this.$emit('eventschanged');
        }
        this.ShowSpinner = false;
      },
      Delete: function(evt) {
        if(confirm("Delete selected event?")) {
          this.$http.post('delEvent', { EventID: evt.id, Analyzer: this.analyzer.Analyzer }).then(
            (result, n) => { this.$emit('eventschanged'); }
          );
        }
      },
      Export: function() {
        var Export = [['Date', 'Type', 'Duration', 'ΔU (V)', 'Notes']];
        for(var i = 0; i < this.Events.length; i++) Export.push([this.Events[i].StartTime, this.Events[i].Type, this.Round(this.Events[i].SampleDuration / this.Events[i].SampleRate, 3), this.Round(this.Events[i].Diff, 3), this.Events[i].Notes]);
        return Object.seal(Export);
      }
    },
    components: { Vuetable, EventDialog, EventNotes }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
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
