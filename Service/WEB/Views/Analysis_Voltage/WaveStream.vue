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
          <span class="input-group-addon">{{ (DownloadFormat=='application/json') ? '.json' : '.cbor' }}</span>
        </div>
      </div>
      <div class="col-md-6">
        <label class="control-label">Waves</label>
        <option-picker :options="[{name: 'Voltage', value: 'Voltage' }, {name: 'Current', value: 'CURRENT'}, {name: 'Voltage and Current', value: 'Voltage_Current'}]" v-model="DownloadSamples" :big="true"></option-picker>
      </div>
      <div class="col-md-6">
        <label class="control-label">File format</label>
        <option-picker :options="[{name: 'CBOR', value: 'application/cbor' }, {name: 'JSON', value: 'application/json'}]" v-model="DownloadFormat" :big="true"></option-picker>
      </div>
      <div class="col-md-6">
        <label class="control-label">Length</label>
        <input-spin :min="0" :max="1000" v-model="Length" :big="true" :increment="1" :decimals="1" prepend="Seconds" :list="[{name: 'Unlimited', value: 0}, {name: '10 seconds', value: 10}, {name: '1 minute', value: 60}]"></input-spin>
      </div>
      <div class="col-xs-12" style="margin-top: 5px">
        <button type="button" class="btn btn-default btn-sm" style="width:100%" title="Close dialog" @click="StartStream">Start</button>
      </div>
    </div>
    <div class="row" v-if="Abort!=null">
      <div class="col-md-12">
        <button type="button" class="btn btn-default btn-sm" style="width:100%" title="Terminate streaming" @click="StopStream">Stop</button>
      </div>
    </div>
  </dlg>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import streamSaver from 'streamsaver';
  export default {
    props: { analyzer: Object },
    data: function() {
      return { File: 'Waveform', DownloadSamples: 'Voltage', DownloadFormat: 'application/json', Abort: null, Length: 0 };
    },
    methods: {
      CreateFile: function(reader) {
        var Toast = this.$toast.open({ type:'info', message: "Downloading '" + this.File + "'", dismissible: false, duration: 0});
        var Progress = 0;
        var fileStream = streamSaver.createWriteStream(this.File + ((this.DownloadFormat == 'application/json') ? '.json' : '.cbor'), null);
        var writeStream = fileStream.getWriter();
        var pump = () => reader.read().then(
          ({ value, done }) => {
            if(done) return writeStream.close();
            Progress += value.length;
            Toast.message = "Downloading '" + this.File + "'<br>" + this.BestUnitsBin(Progress) + " saved";
            return writeStream.write(value).then(pump);
          }
        );
        pump().then(
          () => {
            Toast.type = 'success';
            Toast.message = 'Downloaded!'
            Toast.dismissible = true;
            Toast.duration = 3000;
            this.Abort = null;
          },
          () => {
            Toast.type = 'success';
            Toast.message = 'Downloaded!'
            Toast.dismissible = true;
            Toast.duration = 3000;
            this.Abort = null;
            writeStream.close();
          }
        );
      },
      StartStream: function() {
        this.Abort = new AbortController();
        fetch('getWaveStream', { method: 'POST', body: this.CBOR.encode({ Analyzer: this.analyzer.Analyzer, Length: this.Length * this.analyzer.SampleRate, Series: (this.DownloadSamples == 'Voltage') ? ['Voltage_Samples'] : (this.DownloadSamples == 'Current') ? ['Current_Samples'] : ['Voltage_Samples', 'Current_Samples']}), headers: { 'Accept': this.DownloadFormat, 'Content-Type': 'application/cbor' }, signal: this.Abort.signal }).then(
          (res) => { this.CreateFile(res.body.getReader()); }
        );
      },
      StopStream: function() {
        this.Abort.abort();
      }
    }
  }
</script>