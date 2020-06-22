<template>
  <dlg>
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Maintenance</h4></div>
      <div class="pull-right btn-group" v-if="View == 'Main'">
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', null)"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <input type="file" name="name" style="display: none;" ref="filename" @change="FileSet" accept="application/gzip, .gz"/>
    <div v-if="View == 'Main'">
      <div class="row">
        <div class="col-xs-12">
          <div class="alert-info alert alert text-center" style="margin-bottom: 5px; padding: 5px">
            Analyzer data takes currently <strong>{{ GetBestUnitsBin(TotalSize) }}</strong>
          </div>
        </div>
        <div class="col-xs-12">
          <label class="control-label">Action</label>
          <div style="display: flex">
            <option-picker :options="[{name: 'Clear all', value: 'Clear all'}, {name: 'Clear range', value: 'Clear range'}]" v-model="Action"></option-picker>
            <button type="button" class="btn btn-danger btn-xs" @click="Execute">Execute</button>
          </div>
        </div>
        <div class="col-xs-12">
          <div class="alert-warning alert alert text-center help" style="margin: 5px 0px; padding: 5px">
            <div v-if="Action=='Clear all'">Delete all data in selected aggregation or storage group. This action recover disk space.</div>
            <div v-if="Action=='Clear range'">Delete data in selected aggregation or storage group between a date range. This action will not recover disk space, but mark deleted rows for reusage</div>
          </div>
        </div>
        <div class="col-xs-12 col-sm-6" v-if="Action=='Clear range'">
          <label class="control-label">From</label>
          <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="From"></date-picker>
        </div>
        <div class="col-xs-12 col-sm-6" v-if="Action=='Clear range'">
          <label class="control-label">To</label>
          <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="To"></date-picker>
        </div>
        <div class="col-xs-12">
          <vuetable ref="table" :fields="Columns" :api-mode="false" :data="Tables" class="table small table-striped"></vuetable>
        </div>
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
  import vuetable from 'vuetable-2';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object },
    data() {
      return { View: 'Main', Action: 'Clear all', From: 0, To: 0,
        Columns: [
          { name: '__checkbox' },
          { name: 'id', title: 'Name' },
          { name: 'Size', title: 'Size' }
        ]
      };
    },
    computed: {
      TotalSize: function() {
        var TotalSize = 0;
        for(var n = 0; n < this.analyzer.DatabaseSize.length; n++) TotalSize += this.analyzer.DatabaseSize[n].Size;
        return TotalSize;
      },
      Tables: function() {
        var Tables = [];
        for(var n = 0; n < this.analyzer.DatabaseSize.length; n++) Tables.push({ id: this.analyzer.DatabaseSize[n].Table, Size: this.GetBestUnitsBin(this.analyzer.DatabaseSize[n].Size)});
        return Tables.sort((a, b) => { return (a.id == b.id) ? 0 : (a.id > b.id) ? 1 : -1; });
      }
    },
    methods: {
      Execute: function() {
        var Selection = this.$refs.table.selectedTo;
        for(var n = 0; n < Selection.length; n++) {
          if(this.Action == 'Clear table') {
            this.$http.post('delSeries', { Analyzer: this.analyzer.Analyzer, Aggreg: this.$refs.table.selectedTo }).then();
          } else if(this.Action == 'Clear range') {

          }
        }
      },

//      Show: function (analyzer) {

//        this.DBSize = oZm.GetBestUnitsBin(analyzer.Size);
//        this.StartDate = moment.tz(this.Analyzer.TimeZone).startOf('year').valueOf();
//        this.EndDate = moment.tz(this.Analyzer.TimeZone).endOf('year').valueOf();
//        this.View = 'Main';
//        this.$refs.Dialog.Show();
//        this.$nextTick(function () {
//          this.$refs.StartDate.SetDate(this.StartDate);
//          this.$refs.EndDate.SetDate(this.EndDate);
//        });
//      },
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
    components: { vuetable }
  }
</script>

<style scoped>
  .table >>> th:nth-child(1) {
    width: 20px;
  }
  .table >>> th:nth-child(3) {
    width: 94px;
    text-align: right;
  }
  .table >>> td:nth-child(3) {
    text-align: right;
  }
  .help {
    font-size: 85%;
  }


  div .progress {
    height: 12px;
    background-color: #FFFFFF;
    border:1px solid #BBB;
    margin-bottom: 0px;
  }
</style>