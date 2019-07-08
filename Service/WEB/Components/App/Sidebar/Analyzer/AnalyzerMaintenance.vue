<template>
  <div class="modal-content" v-bind:class="{ hide: !Show }">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Maintenance</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </div>
    <div class="modal-body">
      <input type="file" name="name" style="display: none;" ref="filename" v-on:change="FileSet" accept="application/gzip, .gz"/>
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Only stored samples and events are affected by this next actions.<hr style="margin-top: 5px; margin-bottom: 5px;"/>Analyzer data takes currently <strong>{{ DBSize }}</strong></div>
      <div class="row">
        <div class="col-xs-12 col-sm-6"><label class="control-label">From</label><date-picker ref="StartDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" v-on:DateChanged="SetStartDate"></date-picker></div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">To</label><date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" v-on:DateChanged="SetEndDate"></date-picker></div>
      </div>
      <hr style="margin-top: 10px; margin-bottom: 10px;"/>
      <div class="row" style="margin-top:5px; margin-bottom: 5px">
        <div class="col-xs-6"><button type="button" class="btn btn-default btn-block btn-xs" @click="BackupRange">Backup range</button></div>
        <div class="col-xs-6"><button type="button" class="btn btn-default btn-block btn-xs" @click="BackupAll">Backup all</button></div>
      </div>
      <hr style="margin-top: 10px; margin-bottom: 10px;"/>
      <div class="row" style="margin-top:5px; margin-bottom: 5px">
        <div class="col-xs-6"><button type="button" class="btn btn-default btn-block btn-danger btn-xs" @click="ClearRange">Clear range</button></div>
        <div class="col-xs-6"><button type="button" class="btn btn-default btn-block btn-danger btn-xs" @click="ClearAll">Clear all</button></div>
      </div>
      <hr style="margin-top: 10px; margin-bottom: 10px;"/>
      <div class="row" style="margin-top:5px; margin-bottom: 5px">
        <div class="col-xs-12"><button type="button" class="btn btn-default btn-block btn-xs" @click="Restore">Restore backup</button></div>
      </div>
    </div>
  </div>
</template>

<script>
  import moment from 'JS/moment.js';
  import DatePicker from 'Components/DatePicker.vue';
  import Spinner from 'Components/Spinner.vue';
  import * as oZm from 'JS/openZmeter.js';
  import Oboe from 'JS/oboe.js';
  import StreamSaver from 'JS/streamsaver.js';
  import Pako from 'JS/pako.js';
  export default {
    data() {
      return { Show: false, DBSize: '', StartDate: null, EndDate: null };
    },
    methods: {
      Load: function(analyzer, id) {
        this.Analyzer = analyzer;
        this.Show = true;
        this.DBSize = oZm.GetBestUnitsBin(analyzer.Size);
        this.StartDate = moment().startOf('year');
        this.EndDate = moment().endOf('year');
        this.$nextTick(function() {
          this.$refs.StartDate.SetDate(this.StartDate);
          this.$refs.EndDate.SetDate(this.EndDate);
        });
      },
      SetStartDate: function(date) {
        this.StartDate = date.startOf('day').valueOf();
      },
      SetEndDate: function(date) {
        this.EndDate = date.endOf('day').valueOf();
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Table.Load(this.Analyzer);
      },
      FileSet: function(evt) {
        if(evt.target.files.length !== 1) return;
        var Reader = new FileReader();
        var Inflator = new Pako.Inflate();
        var File = evt.target.files[0];
        var Offset = 0;
        var DecompresedSize = 0;
        var onDeflatedData = function(data) {
          DecompresedSize += data.length;
        };
        var onLoadHandler = function(evt) {
          if(evt.target.error !== null) {
            console.log("ERROR LOADING FILE");
            return;
          }            
          Offset += evt.target.result.byteLength;
          Inflator.push(evt.target.result, (Offset >= File.size) ? Pako.Z_FINISH: Pako.Z_SYNC_FLUSH);
          if(Offset < File.size) {
            Reader.readAsArrayBuffer(File.slice(Offset, Offset + 65536));
          } else {
            console.log(DecompresedSize);
          }
        };
        Reader.onload = onLoadHandler;
        Inflator.onData = onDeflatedData;
        Reader.readAsArrayBuffer(File.slice(0, 65536));
      },
      BackupRange: function() {
        setTimeout(this.GenerateData.bind(this), 100);
      },
      GenerateData: function(fs) {
        for(var n= 0; n < 10000; n++) {
          this.Backup.Voltage.push([230.00000, 230.12234234, 223.23234234]);
        }
        console.log(this.Backup.Voltage.length);
        setTimeout(this.GenerateData.bind(this), 100);

      },
      ClearRange: function() {},
      Restore: function() {
        this.$refs.filename.click();
      },
      BackupAll: function() {},
      ClearAll: function() {}
    },
    created: function() {
      this.Backup = {Voltage: []};
    },
    beforeDestroy: function() {
      $(this.$refs.Base).off('onSetSelectValue', this.SelectBase.bind(this));
      $(this.$refs.Base).off('onUnsetSelectValue', this.DeselectBase.bind(this));
      $(this.$refs.Base).bsSuggest("destroy");
    },
    components: { DatePicker, Spinner }
  }
</script>

<style scoped>

</style>