<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Maintenance</h4></div>
      <div class="pull-right btn-group" v-if="!ShowSpinner">
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', null)"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <input type="file" name="name" style="display: none;" ref="filename" @change="FileSet" accept="application/gzip, .gz"/>
    <div class="alert-info alert alert text-center" style="margin-bottom: 5px; padding: 5px">
      Analyzer data takes currently <strong>{{ BestUnitsBin(TotalSize) }}</strong>
    </div>
    <div class="row">
      <div class="col-xs-12">
        <detail-tabs :tabs="['200MS','3S', '1M', '10M', '15M', '1H', 'Events', 'Alarms', 'Settings']" v-model="ActiveTab"></detail-tabs>
        <table v-if="ActiveTab=='200MS'">
          <tr>
            <td class="first-row">Store 200 miliseconds data. Table persistence is limited to an hour.</td>
            <td class="first-row text-right action">{{ BestUnitsBin(TableSize('Aggreg_200MS')) }}</td>
          </tr>
          <tr class="line">
            <td class="help">Delete all data in this table. All used space is reclaimed.</td>
            <td class="action"><button type="button" class="btn btn-danger btn-xs" @click="Clear('200MS', true)">Execute</button></td>
          </tr>
          <tr class="line">
            <td class="help">Download all table data to a file.</td>
            <td class="action"><button type="button" class="btn btn-default btn-xs" @click="Download('200MS', true)">Execute</button></td>
          </tr>
        </table>
        <table v-if="ActiveTab=='3S'">
          <tr>
            <td class="first-row">Store 3 seconds data. Table persistence is limited only by storage.</td>
            <td class="first-row text-right action">{{ BestUnitsBin(TableSize('Aggreg_3S')) }}</td>
          </tr>
          <tr class="line">
            <td class="help">Delete all data in this table. All used space is reclaimed.</td>
            <td class="action"><button type="button" class="btn btn-danger btn-xs" @click="Clear('3S', true)">Execute</button></td>
          </tr>
          <tr class="line">
            <td class="help">Clear selected range of table.
              <div style="display:flex">
                <label>From:</label><date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="From" style="flex-grow:2"></date-picker>
                <label>To:</label><date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="To" style="flex-grow:2"></date-picker>
              </div>
            </td>
            <td class="action"><button type="button" class="btn btn-danger btn-xs" @click="Clear('3S', false)">Execute</button></td>
          </tr>
          <tr class="line">
            <td class="help">Download all table data to a file.</td>
            <td class="action"><button type="button" class="btn btn-default btn-xs" @click="Download('3S', true)">Execute</button></td>
          </tr>
          <tr class="line">
            <td class="help">Download selected range of table data to a file.
              <div style="display:flex">
                <label>From:</label><date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="From" style="flex-grow:2"></date-picker>
                <label>To:</label><date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="To" style="flex-grow:2"></date-picker>
              </div>
            </td>
            <td class="action"><button type="button" class="btn btn-default btn-xs" @click="Download('3S', false)">Execute</button></td>
          </tr>
          <tr class="line">
            <td class="help">Restore data from file. Stored rows in overlapped period are replaced with new ones.</td>
            <td class="action"><button type="button" class="btn btn-default btn-xs" @click="alert('NOT IMPLEMENTED!')">Execute</button></td>
          </tr>
        </table>
        <table v-if="ActiveTab=='1M'">
          <tr>
            <td class="first-row">Store 1 minute data. Table persistence is limited only by storage but advanced parameters persist only for a day.</td>
            <td class="first-row text-right action">{{ BestUnitsBin(TableSize('Aggreg_1M')) }}</td>
          </tr>
          <tr class="line">
            <td class="help">Delete all data in this table. All used space is reclaimed.</td>
            <td class="action"><button type="button" class="btn btn-danger btn-xs" @click="Clear('1M', true)">Execute</button></td>
          </tr>
          <tr class="line">
            <td class="help">Clear selected range of table.
              <div style="display:flex">
                <label>From:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="From" style="flex-grow:2"></date-picker>
                <label>To:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="To" style="flex-grow:2"></date-picker>
              </div>
            </td>
            <td class="action"><button type="button" class="btn btn-danger btn-xs" @click="Clear('1M', false)">Execute</button></td>
          </tr>
          <tr class="line">
            <td class="help">Download all table data to a file.</td>
            <td class="action"><button type="button" class="btn btn-default btn-xs" @click="Download('1M', true)">Execute</button></td>
          </tr>
          <tr class="line">
            <td class="help">Download selected range of table data to a file.
              <div style="display:flex">
                <label>From:</label><date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="From" style="flex-grow:2"></date-picker>
                <label>To:</label><date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="To" style="flex-grow:2"></date-picker>
              </div>
            </td>
            <td class="action"><button type="button" class="btn btn-default btn-xs" @click="Download('1M', false)">Execute</button></td>
          </tr>
          <tr class="line">
            <td class="help">Restore data from file. Stored rows in overlapped period are replaced with new ones.</td>
            <td class="action"><button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button></td>
          </tr>
        </table>


        <table v-if="ActiveTab=='10M'">
          <tr>
            <td class="first-row">
              Store 10 minute data. Table persistence is limited only by storage.
            </td>
            <td class="first-row text-right action">
              {{ BestUnitsBin(TableSize('Aggreg_10M')) }}
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Delete all data in this table. All used space is reclaimed.
            </td>
            <td class="action">
              <button type="button" class="btn btn-danger btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Download all table data to a file.
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Download selected range of table data to a file.
              <div style="display:flex">
                <label>From:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="From" style="flex-grow:2"></date-picker>
                <label>To:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="To" style="flex-grow:2"></date-picker>
              </div>
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Restore data from file. Stored rows in overlapped period are replaced with new ones.
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
        </table>
        <table v-if="ActiveTab=='15M'">
          <tr>
            <td class="first-row">
              Store 15 minute data. Table persistence is limited only by storage.
            </td>
            <td class="first-row text-right action">
              {{ BestUnitsBin(TableSize('Aggreg_15M')) }}
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Delete all data in this table. All used space is reclaimed.
            </td>
            <td class="action">
              <button type="button" class="btn btn-danger btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Download all table data to a file.
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Download selected range of table data to a file.
              <div style="display:flex">
                <label>From:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="From" style="flex-grow:2"></date-picker>
                <label>To:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="To" style="flex-grow:2"></date-picker>
              </div>
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Restore data from file. Stored rows in overlapped period are replaced with new ones.
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
        </table>
        <table v-if="ActiveTab=='1H'">
          <tr>
            <td class="first-row">
              Store 1 hour data. Table persistence is limited only by storage.
            </td>
            <td class="first-row text-right action">
              {{ BestUnitsBin(TableSize('Aggreg_1H')) }}
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Delete all data in this table. All used space is reclaimed.
            </td>
            <td class="action">
              <button type="button" class="btn btn-danger btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Download all table data to a file.
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Download selected range of table data to a file.
              <div style="display:flex">
                <label>From:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="From" style="flex-grow:2"></date-picker>
                <label>To:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="To" style="flex-grow:2"></date-picker>
              </div>
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Restore data from file. Stored rows in overlapped period are replaced with new ones.
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
        </table>
        <table v-if="ActiveTab=='Events'">
          <tr>
            <td class="first-row">
              Store event list, RMS and waveforms.
            </td>
            <td class="first-row text-right action">
              {{ BestUnitsBin(TableSize('Events')) }}
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Delete all data in this table. All used space is reclaimed.
            </td>
            <td class="action">
              <button type="button" class="btn btn-danger btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Download all table data to a file.
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Download selected range of table data to a file.
              <div style="display:flex">
                <label>From:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="From" style="flex-grow:2"></date-picker>
                <label>To:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="To" style="flex-grow:2"></date-picker>
              </div>
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Restore data from file. Stored rows in overlapped period are replaced with new ones.
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
        </table>
        <table v-if="ActiveTab=='Alarms'">
          <tr>
            <td class="first-row">
              Store alarm list.
            </td>
            <td class="first-row text-right action">
              {{ BestUnitsBin(TableSize('Alarms')) }}
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Delete all data in this table. All used space is reclaimed.
            </td>
            <td class="action">
              <button type="button" class="btn btn-danger btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Download all table data to a file.
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Download selected range of table data to a file.
              <div style="display:flex">
                <label>From:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="From" style="flex-grow:2"></date-picker>
                <label>To:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="To" style="flex-grow:2"></date-picker>
              </div>
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Restore data from file. Stored rows in overlapped period are replaced with new ones.
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
        </table>
        <table v-if="ActiveTab=='Settings'">
          <tr>
            <td class="first-row">
              Store configured alarms and rates.
            </td>
            <td class="first-row text-right action">
              {{ BestUnitsBin(TableSize('Settings')) }}
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Delete all data in this table. All used space is reclaimed.
            </td>
            <td class="action">
              <button type="button" class="btn btn-danger btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Download all table data to a file.
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Download selected range of table data to a file.
              <div style="display:flex">
                <label>From:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="From" style="flex-grow:2"></date-picker>
                <label>To:</label>
                <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-model="To" style="flex-grow:2"></date-picker>
              </div>
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
          <tr class="line">
            <td class="help">
              Restore data from file. Stored rows in overlapped period are replaced with new ones.
            </td>
            <td class="action">
              <button type="button" class="btn btn-default btn-xs" @click="Execute">Execute</button>
            </td>
          </tr>
        </table>
      </div>
    </div>
  </dlg>
</template>

<script>
  import vuetable from 'vuetable-2';
  import * as oZm from 'JS/openZmeter.js';
  import streamSaver from 'streamsaver'
  export default {
    props: { analyzer: Object },
    data() {
      return { ShowSpinner: false, Action: 'Clear all', From: 0, To: 0, Storage: [], ActiveTab: null,
      };
    },
    watch: {
      ActiveTab: function(tab) {
        if((tab != '3S') && (tab != '1M') && (tab != '10M') && (tab != '15M') && (tab != '1H')) return;
        this.ShowSpinner = true;
        this.$http.post('getSeriesRange', { Analyzer: this.analyzer.Analyzer, Aggreg: tab }).then(
          (result) => { this.ShowSpinner = false; this.From = result.body.From; this.To = result.body.To; },
          (fail) => { this.ShowSpinner = false; }
        );
      },
    },
    created: function() {
      this.GetStorage();
    },
    computed: {
      TotalSize: function() {
        var TotalSize = 0;
        for(var n = 0; n < this.Storage.length; n++) TotalSize += this.Storage[n].Size;
        return TotalSize;
      }
    },
    methods: {
      TableSize: function(name) {
        for(var n = 0; n < this.Storage.length; n++) {
          if(this.Storage[n].Table == name) return this.Storage[n].Size;
        }
        return 0;
      },
      GetStorage: function() {
        this.Rate = null;
        this.ShowSpinner = true;
        this.$http.post('getStatus', { Analyzer: this.analyzer.Analyzer, Series: ['STORAGE'] }).then(
          (result) => {
            this.Storage = result.body.Storage;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      Clear: function(aggreg, all) {
        this.ShowSpinner = true;
        var From = (all) ? 0 : this.From;
        var To   = (all) ? Number.MAX_SAFE_INTEGER : this.To;
        this.$http.post('delSeriesRange', { Analyzer: this.analyzer.Analyzer, From: From, To: To, Aggreg: aggreg}).then(
          (res) => { this.ShowSpinner = false; this.ActiveTab = aggreg; },
          (fail) => { this.ShowSpinner = false; }
        );
      },
      Download: function(aggreg, all) {
        this.ShowSpinner = true;
        var Series = [];
        if(aggreg == '200MS') Series = ['VOLTAGE', 'CURRENT', 'UNBALANCE', 'FREQUENCY', 'FLAG', 'ACTIVE_POWER', 'REACTIVE_POWER', 'POWER_FACTOR', 'VOLTAGE_THD', 'CURRENT_THD', 'PHI'];
        if(aggreg == '3S')    Series = ['VOLTAGE', 'CURRENT', 'FREQUENCY', 'FLAG', 'ACTIVE_POWER', 'REACTIVE_POWER'];
        if(aggreg == '1M')    Series = ['VOLTAGE', 'CURRENT', 'FREQUENCY', 'FLAG', 'POWER_FACTOR', 'UNBALANCE', 'VOLTAGE_THD', 'CURRENT_THD', 'ACTIVE_POWER', 'REACTIVE_POWER', 'PHI', 'VOLTAGE_HARMONICS', 'CURRENT_HARMONICS', 'POWER_HARMONICS'];

        var From = (all) ? 0 : this.From;
        var To   = (all) ? Number.MAX_SAFE_INTEGER : this.To;
        fetch('getSeries', { method: 'POST', body: JSON.stringify({ Analyzer: this.analyzer.Analyzer, From: From, To: To, Aggreg: aggreg, Series: Series}), headers: { 'Content-Type': 'application/json' } }).then(
          (res) => {
            const fileStream = streamSaver.createWriteStream(aggreg + '.json', null);
            const writeStream = fileStream.getWriter();
            const reader = res.body.getReader();
            const pump = () => reader.read().then(({ value, done }) => { return done ? writeStream.close() : writeStream.write(value).then(pump); });
            pump().then(() => { this.ShowSpinner = false; });
          }
        );
      },









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

//        this.DBSize = this.BestUnitsBin(analyzer.Size);
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
//        this.View = 'Save';
        this.Deflator = new Pako.Deflate({level: 9, gzip: true});
        this.DateText = moment.tz(this.StartDate, this.Analyzer.TimeZone).format('DD/MM/YYYY');
        this.Chunks = [];
        this.TotalSize = 0;
        this.SizeText = this.BestUnitsBin(this.TotalSize);
        this.Progress = 0;
        this.Range = this.EndDate - this.StartDate;
        this.Deflator.onEnd = () => {
          FileSaver.saveAs(new Blob(this.Chunks, {type: "application/gzip"}), this.Analyzer.Analyzer + '.gz');
          this.View = 'Main';
        };
        this.Deflator.onData = (chunk) => {
          this.TotalSize += chunk.length;
          this.SizeText = this.BestUnitsBin(this.TotalSize);
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
  table {
    width: 100%;
    border: 1px solid #ccc;
  }
  .first-row {
    background: #eee;
    padding: 2px;
    border: 1px solid #ccc;
  }
  .action {
    width: 80px;
    vertical-align: bottom;
  }
  .action > button {
    width: 100%;
  }
  .line td {
    padding: 4px 4px;
  }
  .line {
    border: 1px solid #ccc;
  }
  table label {
    margin: 2px 2px;
  }

  table .border-bottom {
    border-bottom: 1px solid #ccc;
    background: #eee;
    font-weight: bold;
  }
  table tr td {
    border-spacing: 0px;
    padding: 3px;
  }
  .tbl-size {
    font-size: 12px;
    float: right;
    font-weight: normal;
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