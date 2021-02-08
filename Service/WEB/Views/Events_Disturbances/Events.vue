<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Last events</div>
      <div class="header-buttons clearfix">
        <toolbar-pager :pageSize="pagesize" :input="Events" :output.sync="FilterEvents" :filtercols="['Date', 'Type', 'Notes']"></toolbar-pager>
        <div class="btn-group">
          <button class="btn btn-default btn-xs btn-danger" type="button" @click="DeleteSelected">Delete selection</button>
          <button-export file="Events" :rows="Export"></button-export>
        </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="(events==null)||ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" :data="FilterEvents" track-by="StartTime" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-xs btn-success" @click="Edit(props.rowData)">Add notes</button>
            <button type="button" class="btn btn-xs btn-default" :class="{ disabled: !props.rowData.AvaliableRAW }" @click="RMS(props.rowData)">RMS</button>
            <button type="button" class="btn btn-xs btn-default" :class="{ disabled: !props.rowData.AvaliableRAW }" @click="RAW(props.rowData)">Wave</button>
            <button type="button" class="btn btn-danger btn-xs"  @click="DeleteEvent(props.rowData)">Delete</button>
          </div>
        </template>
      </vuetable>
    </div>
    <event-dialog v-if="(View=='RMS')||(View=='RAW')" :mode="View" :analyzer="analyzer" :event="Event" @close="View=null"></event-dialog>
    <event-notes v-if="View=='Notes'" :analyzer="analyzer" :event="Event" @close="UpdateEvent"></event-notes>
    <event-delete :analyzer="analyzer" :event="Delete" v-if="Delete!=null" @close="UpdateEvent"></event-delete>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  import EventDialog from './EventDialog.vue';
  import EventNotes from './EventNotes.vue';
  import EventDelete from './EventDelete.vue';
  export default {
    props: { analyzer: Object, events: Object, pagesize: Number },
    data: function() {
      return { Event: null, View: null, Events: [], FilterEvents: [], ShowSpinner: false, Delete: null,
               Columns: [
                 { name: '__checkbox'},
                 { name: 'StartTime', title: 'Date', callback: this.FormatDate },
                 { name: 'Type', title: 'Type' },
                 { name: '', title: 'Duration <em>(s)</em>', callback: this.DurationFormat },
                 { name: '', title: 'ΔU <em>(V)</em>', callback: this.ChangeFormat },
                 { name: '', title: 'ΔU <em>(%)</em>', callback: this.PercentFormat },
                 { name: 'Notes', title: 'Notes' },
                 { name: '__slot:actions', title: '' }
               ]
      };
    },
    updated: function() {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    watch: {
      events: function() {
        var Events = [];
        if((this.events == null) || (this.analyzer == null)) return;
        var StartTimeCol  = this.events.Headers.indexOf("StartTime");
        var TypeCol       = this.events.Headers.indexOf("Type");
        var ReferenceCol  = this.events.Headers.indexOf("Reference");
        var PeakCol       = this.events.Headers.indexOf("Peak");
        var ChangeCol     = this.events.Headers.indexOf("Change");
        var SampleRateCol = this.events.Headers.indexOf("SampleRate");
        var DurationCol   = this.events.Headers.indexOf("Duration");
        var NotesCol      = this.events.Headers.indexOf("Notes");
        var RawCol        = this.events.Headers.indexOf("AvaliableRAW");
        var RmsCol        = this.events.Headers.indexOf("AvaliableRMS");
        for(var x = 0; x < this.events.Rows.length; x++) {
          Events.push({ StartTime: this.events.Rows[x][StartTimeCol], Type: this.events.Rows[x][TypeCol],
                        Reference: this.events.Rows[x][ReferenceCol], Peak: this.events.Rows[x][PeakCol], Change: this.events.Rows[x][ChangeCol],
                        SampleRate: this.events.Rows[x][SampleRateCol], Duration: this.events.Rows[x][DurationCol], Notes: this.events.Rows[x][NotesCol],
                        AvaliableRAW: this.events.Rows[x][RawCol], AvaliableRMS: this.events.Rows[x][RmsCol]
          });
        }
        this.Events = Events;
      }
    },
    methods: {
      FormatDate: function(val) {
        return this.analyzer.FormatDateTime(val);
      },
      DurationFormat: function(val) {
        return (val.Duration == 0) ? 'RUNNNING' : this.Round(val.Duration / val.SampleRate, 2, false);
      },
      ChangeFormat: function(val) {
        var Max = 0;
        for(var i = 0; i < val.Reference.length; i++) Max = (Math.abs(val.Reference[i] - val.Peak[i]) > Math.abs(Max)) ? val.Peak[i] - val.Reference[i] : Max;
        if(val.Type == 'RVC') {
          var Diff = 0;
          for(var i = 0; i < val.Reference.length; i++) Diff = (Math.abs(val.Reference[i] - val.Change[i]) > Math.abs(Diff)) ? val.Change[i] - val.Reference[i] : Diff;
          return this.Round(Diff, 2, false) + '<br>(' + this.Round(Max, 2, false) + ' Max)';
        } else {
          return this.Round(Max, 2, false);
        }
      },
      PercentFormat: function(val) {
        var Max = 0;
        for(var i = 0; i < val.Reference.length; i++) Max = (Math.abs(val.Reference[i] - val.Peak[i]) > Math.abs(Max)) ? val.Peak[i] - val.Reference[i] : Max;
        if(val.Type == 'RVC') {
          var Diff = 0;
          for(var i = 0; i < val.Reference.length; i++) Diff = (Math.abs(val.Reference[i] - val.Change[i]) > Math.abs(Diff)) ? val.Change[i] - val.Reference[i] : Diff;
          return this.Round(Diff / val.Reference[0] * 100.0, 2, false);
        } else {
          return this.Round(Max / val.Reference[0] * 100.0, 2, false);
        }
      },
      Edit: function(evt) {
        this.Event = evt;
        this.View = 'Notes';
      },
      UpdateEvent: function(update) {
        this.View = null;
        this.Delete = null;
        if(update) this.$emit('eventschanged');
      },
      RMS: function(evt) {
        this.ShowSpinner = true;
        this.$http.post('getEvent', { StartTime: evt.StartTime, Analyzer: this.analyzer.Analyzer, Samples: 'RMS' }).then(
          (result) => {
            this.ShowSpinner = false;
            this.Event = Object.freeze(result.body);
            this.View = 'RMS';
          },
          (fail) => {
            this.ShowSpinner = false;
          }
        );
      },
      RAW: function(evt) {
        this.ShowSpinner = true;
        this.$http.post('getEvent', { StartTime: evt.StartTime, Analyzer: this.analyzer.Analyzer, Samples: 'RAW' }).then(
          (result) => {
            this.ShowSpinner = false;
            this.Event = Object.freeze(result.body);
            this.View = 'RAW';
          },
          (fail) => {
            this.ShowSpinner = false;
          }
        );
      },
      DeleteSelected: function() {
        var Selection = this.$refs.table.selectedTo;
        this.Delete = [];
        for(var n = 0; n < Selection.length; n++) this.Delete.push(Selection[n]);
      },
      DeleteEvent: function(evt) {
        this.Delete = [evt.StartTime];
      },
      Export: function() {
        var Export = [['Date', 'Type', 'Duration', 'ΔU (V)', 'Notes']];
        for(var i = 0; i < this.Events.length; i++) Export.push([this.Events[i].StartTime, this.Events[i].Type, this.Round(this.Events[i].SampleDuration / this.Events[i].SampleRate, 3), this.Round(this.Events[i].Diff, 3), this.Events[i].Notes]);
        return Object.seal(Export);
      }
    },
    components: { Vuetable, EventDialog, EventNotes, EventDelete }
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
