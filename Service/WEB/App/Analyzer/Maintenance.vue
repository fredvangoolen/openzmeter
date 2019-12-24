<template>
  <dlg ref="Dialog">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Maintenance</h4></div>
      <div class="pull-right btn-group" v-if="View == 'Main'">
        <button type="button" class="btn btn-default btn-xs" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <input type="file" name="name" style="display: none;" ref="filename" v-on:change="FileSet" accept="application/gzip, .gz"/>
    <div v-if="View == 'Main'">
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Analyzer data takes currently <strong>{{ DBSize }}</strong></div>
      <div class="row">
        <div class="col-xs-12 col-sm-6"><label class="control-label">From</label><date-picker ref="StartDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: Analyzer.TimeZone }" v-on:change="SetStartDate"></date-picker></div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">To</label><date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: Analyzer.TimeZone }" v-on:change="SetEndDate"></date-picker></div>
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
    <div v-if="View == 'Save'">
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Fetching data, please wait<br>Currently downloading day <strong>{{ DateText }}</strong> - Compressed size: <strong>{{ SizeText }}</strong></div>
      <div class="row">
        <div class="col-xs-12"><progress-bar :value="Progress"></progress-bar></div>
      </div>
    </div>
  </dlg>
</template>

<script>
  import moment from 'moment';
  import DatePicker from 'Components/DatePicker.vue';
  import ProgressBar from 'Components/ProgressBar.vue';
  import * as oZm from 'JS/openZmeter.js';
  import Dlg from 'Components/Dialog.vue';
//  import Oboe from 'JS/oboe.js';
  import FileSaver from 'JS/FileSaver.js';
  import Pako from 'JS/pako.js';
  export default {
    data() {
      return { Analyzer: null, DBSize: '', DateText: '', Progress: 0, View: '', SizeText: '', Range: 0 };
    },
    methods: {
      Show: function (analyzer) {
        this.Analyzer = analyzer;
        this.DBSize = oZm.GetBestUnitsBin(analyzer.Size);
        this.StartDate = moment.tz(this.Analyzer.TimeZone).startOf('year').valueOf();
        this.EndDate = moment.tz(this.Analyzer.TimeZone).endOf('year').valueOf();
        this.View = 'Main';
        this.$refs.Dialog.Show();
        this.$nextTick(function () {
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
      Back: function () {
        this.$refs.Dialog.Close();
        this.$parent.$refs.Dialog.Show();
      },
      FileSet: function (evt) {
        if (evt.target.files.length !== 1)
          return;
        var Reader = new FileReader();
        var Inflator = new Pako.Inflate();
        var File = evt.target.files[0];
        var Offset = 0;
        var DecompresedSize = 0;
        var onDeflatedData = function (data) {
          DecompresedSize += data.length;
        };
        var onLoadHandler = function (evt) {
          if (evt.target.error !== null) {
            console.log("ERROR LOADING FILE");
            return;
          }
          Offset += evt.target.result.byteLength;
          Inflator.push(evt.target.result, (Offset >= File.size) ? Pako.Z_FINISH : Pako.Z_SYNC_FLUSH);
          if (Offset < File.size) {
            Reader.readAsArrayBuffer(File.slice(Offset, Offset + 65536));
          } else {
            console.log(DecompresedSize);
          }
        };
        Reader.onload = onLoadHandler;
        Inflator.onData = onDeflatedData;
        Reader.readAsArrayBuffer(File.slice(0, 65536));
      },
      BackupRange: function () {
        this.View = 'Save';
        this.Deflator = new Pako.Deflate({level: 9, gzip: true});
        this.DateText = moment.tz(this.StartDate, this.Analyzer.TimeZone).format('DD/MM/YYYY');
        this.Chunks = [];
        this.TotalSize = 0;
        this.SizeText = oZm.GetBestUnitsBin(this.TotalSize);
        this.Progress = 0;
        this.Range = this.EndDate - this.StartDate;
        this.Deflator.onEnd = () => {
          FileSaver.saveAs(new Blob(this.Chunks, {type: "application/gzip"}), this.Analyzer.Analyzer + '.gz');
          this.View = 'Main';
        };
        this.Deflator.onData = (chunk) => {
          this.TotalSize += chunk.length;
          this.SizeText = oZm.GetBestUnitsBin(this.TotalSize);
          this.Chunks.push(chunk);
        };
        this.FetchDay();
      },
      FetchDay: function () {
        var DayData = [];
        var Series3S = ["FREQUENCY", "FLAG", "VOLTAGE", "CURRENT", "ACTIVE_POWER", "REACTIVE_POWER"];
        var Series1M = ["FREQUENCY","FLAG", "VOLTAGE", "CURRENT"];
        var Series10M = ["FREQUENCY", "FLAG", "VOLTAGE", "CURRENT"];
        var Series15M = ["ACTIVE_POWER", "REACTIVE_POWER"];
        var Series1H = ["FREQUENCY", "FLAG", "VOLTAGE", "CURRENT", "ACTIVE_POWER", "REACTIVE_POWER", "VOLTAGE_STATS", "FREQUENCY_STATS", "VOLTAGE_THD_STATS", "VOLTAGE_HARMONICS_STATS"];
        var FetchFunc = (aggreg, series) => {
          var ToDate = moment.tz(this.StartDate, this.Analyzer.TimeZone).endOf('day').valueOf();
          $.ajax({url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({Series: series, LastSample: 0, Analyzer: this.Analyzer.Analyzer, Aggreg: aggreg, To: ToDate, Period: 86399999}),
          success: (result) => {
            if(result.Sampletime.length > 0) DayData.push(result);
            if(aggreg == '3S') FetchFunc('1M', Series1M);
            if(aggreg == '1M') FetchFunc('10M', Series10M);
            if(aggreg == '10M') FetchFunc('15M', Series15M);
            if(aggreg == '15M') FetchFunc('1H', Series1H);
            if(aggreg == '1H') {
              this.StartDate = ToDate + 1;
              for(var n = 0; n < DayData.length; n++) this.Deflator.push(JSON.stringify(DayData[n]), false);
              if(ToDate == this.EndDate) this.Deflator.push('', true);
              this.DateText = moment.tz(this.StartDate, this.Analyzer.TimeZone).format('DD/MM/YYYY');
              this.Progress = 100 - Math.round(((this.EndDate - this.StartDate) / this.Range) * 100.0);
              if(ToDate != this.EndDate) this.FetchDay();
            }
          },
          error: () => {
            alert("ERRORRRRRR")
          }
        });
          
          
        };
        FetchFunc('3S', Series3S);
      },
      ClearRange: function () {},
      Restore: function () {
        this.$refs.filename.click();
      },
      BackupAll: function () {},
      ClearAll: function () {}
    },
    created: function () {
      this.Backup = {Voltage: []};
      this.Days = 0;
    },
    components: {Dlg, DatePicker, ProgressBar}
  }
</script>

<style scoped>
  div .progress {
    height: 12px;
    background-color: #FFFFFF;
    border:1px solid #BBB;
    margin-bottom: 0px;
  }
</style>