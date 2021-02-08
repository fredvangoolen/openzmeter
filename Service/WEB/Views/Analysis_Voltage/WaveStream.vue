<template>
  <dlg>
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Waveform stream</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" title="Close dialog" v-if="Abort==null" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
    </template>
    <div class="row" v-if="Abort==null">
      <div class="col-md-6" >
        <label class="control-label">File name</label>
        <div class="input-group">
          <input type="text" class="form-control" v-model="File">
          <span class="input-group-addon">.CSV</span>
        </div>
      </div>
      <div class="col-md-6">
        <label class="control-label">Waves</label>
        <option-picker :options="[{name: 'Voltage', value: 'Voltage' }, {name: 'Current', value: 'CURRENT'}, {name: 'Voltage and Current', value: 'Voltage_Current'}]" v-model="DownloadSamples" :big="true"></option-picker>
      </div>
      <div class="col-xs-12" style="margin-top: 5px">
        <button type="button" class="btn btn-default btn-sm" style="width:100%" title="Close dialog" @click="StartStream">Start</button>
      </div>
    </div>
    <div class="row" v-if="Abort!=null">
      <div class="col-md-12">
        <div class="alert alert-info text-center" style="margin: 5px 0px; padding: 5px" v-if="Info!=null">Downloaded {{ Info.Samples }} samples ({{ Round(Info.Time, 2) }} seconds)</div>
        <div class="alert alert-error text-center" style="margin: 5px 0px; padding: 5px" v-if="Error!=null">{{ Error }}</div>
      </div>
      <div class="col-md-12">
        <button type="button" class="btn btn-default btn-sm" style="width:100%" title="Terminate streaming" @click="StopStream">Stop</button>
      </div>
    </div>
  </dlg>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object },
    data: function() {
      return { File: 'Waveform', DownloadSamples: 'Voltage', Error: null, Info: null, Abort: null, Data: [] };
    },
    methods: {
      StartStream: function() {
        this.Abort = new AbortController();
        const jsonDecoder = new TransformStream({
          start: (controller) => {
            controller.buf = '';
          },
          transform: (chunk, controller) => {
            for(var p = 0; p < chunk.length; p++) {
              if((controller.buf.length == 0) && (chunk[p] != '{')) continue;
              controller.buf += chunk[p];
              if(chunk[p] == '}') {
                controller.enqueue(JSON.parse(controller.buf));
                controller.buf = '';
              }
            }
          }
        });
        const eventStream = new WritableStream({
          start: (controller) => {
            this.Info = { Samples: 0, Time: 0.0 };
            if(this.DownloadSamples == 'Voltage') {
              this.Data = [['Sample'].concat(oZm.Series['Voltage'].Names(this.analyzer))];
            } else if(this.DownloadSamples == 'CURRENT') {
              this.Data = [['Sample'].concat(oZm.Series['CURRENT'].Names(this.analyzer))];
            } else {
              this.Data = [['Sample'].concat(oZm.Series['Voltage'].Names(this.analyzer)).concat(oZm.Series['CURRENT'].Names(this.analyzer))];
            }
          },
          write: (message, controller) => {
            for(var n = 0; n < message.Samples; n++) {
              var Row = [this.Info.Samples];
              if(message.Voltage_Samples != null) {
                for(var ch = 0; ch < message.Voltage_Samples.length; ch++) Row.push(message.Voltage_Samples[ch][n]);
              }
              if(message.Current_Samples != null) {
                for(var ch = 0; ch < message.Current_Samples.length; ch++) Row.push(message.Current_Samples[ch][n]);
              }
              this.Data.push(Row);
              this.Info.Samples++;
            }
            this.Info.Time = this.Info.Samples / this.analyzer.SampleRate;
          },
          close: (controller) => {
            this.Abort = null;
            this.Save();
          },
          abort: (reason) => {
            this.Abort = null;
          }
        });
        fetch('getWaveStream', { method: 'POST', body: JSON.stringify({ Analyzer: this.analyzer.Analyzer, Series: (this.DownloadSamples == 'Voltage') ? ['Voltage_Samples'] : (this.DownloadSamples == 'Current') ? ['Current_Samples'] : ['Voltage_Samples', 'CURRENT_SAMPLES']}), headers: { 'Content-Type': 'application/json' }, credentials: 'same-origin', signal: this.Abort.signal }).then(
          (response) => {
            response.body.pipeThrough(new TextDecoderStream()).pipeThrough(jsonDecoder).pipeTo(eventStream);
          },
          (error) => {
            this.Info  = null;
            this.Error = "Download failed, not enought memory, disk space or connection speed is too slow";
            setTimeout(() => { this.Error = null; }, 5000);
          }
        );
      },
      StopStream: function() {
        this.Abort.abort();
        this.Save();
      },
      FormatText: function() {
        var processRow = function (row) {
          var finalVal = '';
          for(var j = 0; j < row.length; j++) {
            var innerValue = row[j] === null ? '' : row[j].toString();
            if(row[j] instanceof Date) innerValue = row[j].toLocaleString();
            var result = innerValue.replace(/"/g, '""');
            if(result.search(/("|;|\n)/g) >= 0) result = '"' + result + '"';
            if(j > 0) finalVal += ';';
            finalVal += result;
          }
          return finalVal + '\r\n';
        };
        var csvFile = '';
        for(var i in this.Data) csvFile += processRow(this.Data[i]);
        return csvFile;
      },
      Save: function() {
        var csvFile = this.FormatText();
        var blob = new Blob([csvFile], {type: 'text/csv;charset=utf-8;'});
        if(navigator.msSaveBlob) { // IE 10+
          navigator.msSaveBlob(blob, this.File + '.csv');
        } else {
          var link = document.createElement("a");
          if(link.download !== undefined) {
            var url = URL.createObjectURL(blob);
            link.setAttribute("href", url);
            link.setAttribute("download", this.File + '.csv');
            link.style.visibility = 'hidden';
            document.body.appendChild(link);
            link.click();
            document.body.removeChild(link);
          }
        }
      },
    }
  }
</script>