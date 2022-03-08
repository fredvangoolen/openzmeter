<template>
  <dlg :loading="TransferStatus.Data==null">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Information</h4></div>
      <div class="pull-right btn-group">
        <z-button @click="$emit('close')" :image="require('Imgs/arrow_undo.png')" />
      </div>
    </template>
    <template v-slot:footer v-if="(TransferStatus.Data!=null)&&(TransferStatus.Data.Version!=null)">
      <div class="row" style="text-align: center">
        <div class="col-xs-4">Architecture is {{ TransferStatus.Data.Version.Arch }}</div>
        <div class="col-xs-4">Build is {{ TransferStatus.Data.Version.Branch }}_{{ TransferStatus.Data.Version.Build }}</div>
      </div>
    </template>
    <div class="cards" v-if="TransferStatus.Data!=null">
      <div class="panel panel-other card" v-if="TransferStatus.Data.System.CPU!=null">
        <img class="card-img" :src="require('Imgs/chip.png')">
        <div>
          <div class="card-title">CPU</div>
          <div class="card-content"> {{ Round(TransferStatus.Data.System.CPU.Usage, 1) }} %</div>
          <div v-if="TransferStatus.Data.System.CPU.Count!=null">Total of {{ TransferStatus.Data.System.CPU.Count }} cores<span v-if="TransferStatus.Data.System.CPU.Frequency!=null"> at {{ TransferStatus.Data.System.CPU.Frequency }} MHz</span></div>
        </div>
      </div>
      <div class="panel panel-other card" v-if="TransferStatus.Data.System.RAM!=null">
        <img class="card-img" :src="require('Imgs/ddr_memory.png')">
        <div>
          <div class="card-title">RAM</div>
          <div class="card-content"> {{ Round((TransferStatus.Data.System.RAM.Total - (TransferStatus.Data.System.RAM.Free + TransferStatus.Data.System.RAM.Cached + TransferStatus.Data.System.RAM.Buffers)) / TransferStatus.Data.System.RAM.Total * 100.0, 1) }} %</div>
          <div>Total memory {{ Round(TransferStatus.Data.System.RAM.Total / 1024 / 1024, 2) }} MB</div>
        </div>
      </div>
      <div class="panel panel-other card" v-if="TransferStatus.Data.System.Storage!=null">
        <img class="card-img" :src="require('Imgs/storage.png')">
        <div>
          <div class="card-title">Storage</div>
          <div class="card-content"> {{ Round((TransferStatus.Data.System.Storage.Total - TransferStatus.Data.System.Storage.Free) / TransferStatus.Data.System.Storage.Total * 100, 1) }} %</div>
          <div>Total space {{ Round(TransferStatus.Data.System.Storage.Total / 1024 / 1024, 1) }} MB</div>
        </div>
      </div>
      <div class="panel panel-other card" v-if="TransferStatus.Data.System.Temperature!=null">
        <img class="card-img" :src="require('Imgs/temperature_normal.png')">
        <div>
          <div class="card-title">Temperature</div>
          <div class="card-content"> {{ Round(TransferStatus.Data.System.Temperature, 1) }} ºC</div>
        </div>
      </div>
    </div>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label">Log messages</label>
        <textarea rows="10" class="form-control" disabled :value="Messages"></textarea>
      </div>
    </div>
  </dlg>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  export default {
    props: { analyzer: Object },
    data() {
      return { TransferStatus: this.StatusTransfer(['Version', 'Log', 'System']) };
    },
    mounted: function() {
      this.TransferStatus.Update(this.analyzer);
    },
    computed: {
      Messages: function() {
        var Text = '';
        if(this.TransferStatus.Data == null) return Text;
        for(var n = 1; n < this.TransferStatus.Data.Log.Messages.length; n++) {
          Text += this.TransferStatus.Data.Log.Messages[n][0] + ' - ';
          Text += moment(this.TransferStatus.Data.Log.Messages[n][1]).format('DD/MM/YYYY - HH:mm:ss') + ' - ';
          Text += this.TransferStatus.Data.Log.Messages[n][2] + '\n';
        }
        return Text;
      }
    }
  }
</script>

<style scoped>
  .cards {
    display: flex;
    flex-wrap: wrap;
  }
  .card {
    width: 80px;
    margin: 3px;
    padding: 6px;
    display: flex;
    flex-grow: 2;
    font-size: 12px;
  }
  .card-img {
    margin: 6px;
    width: 32px;
    height: 32px;
  }
  .card-title {
    font-weight: bold;
    line-height: 12px;
    font-size: 12px;
  }
  .card-content {
    line-height: 18px;
    font-size: 18px;
    font-weight: bold;
  }
  textarea {
    resize: none;
    font-family: 'Fira Code';
    font-size: 10px;
    white-space: pre;
  }
</style>