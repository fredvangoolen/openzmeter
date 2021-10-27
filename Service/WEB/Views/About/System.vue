<template>
  <div>
    <q-toolbar class="bg-grey-3 q-pr-none">
      <div>{{ (LastUpdate==null) ? 'Loading data...' : 'Last update at ' + FormatDate(LastUpdate) }}</div>
      <q-space></q-space>
      <q-btn stretch flat size="sm"><div>Level<br><strong>Warnings</strong></div></q-btn>
      <!--div class="header-buttons">
        <toolbar-options storage="SystemOptions" :options="['Debug', 'Warning', 'Error', 'Info']" v-model="Options"></toolbar-options>
        <toolbar-pager :pageSize="20" :input="Messages" :output.sync="FilterMessages" :filtercols="['Message', 'Date']"></toolbar-pager>
      </div-->
    </q-toolbar>
    <q-separator />
    <div class="panel-heading clearfix">


    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="cards" v-if="Status!=null">
        <div class="panel panel-other card" v-if="Status.CPU!=null">
          <img class="card-img" :src="require('Imgs/chip.png')">
          <div>
            <div class="card-title">CPU</div>
            <div class="card-content"> {{ Round(Status.CPU.Usage, 1) }} %</div>
            <div v-if="Status.CPU.Count!=null">Total of {{ Status.CPU.Count }} cores<span v-if="Status.CPU.Frequency!=null"> at {{ Status.CPU.Frequency }} MHz</span></div>
          </div>
        </div>
        <div class="panel panel-other card" v-if="Status.RAM!=null">
          <img class="card-img" :src="require('Imgs/ddr_memory.png')">
          <div>
            <div class="card-title">RAM</div>
            <div class="card-content"> {{ Round((Status.RAM.Total - (Status.RAM.Free + Status.RAM.Cached + Status.RAM.Buffers)) / Status.RAM.Total * 100.0, 1) }} %</div>
            <div>Total memory {{ Round(Status.RAM.Total / 1024 / 1024, 2) }} MB</div>
          </div>
        </div>
        <div class="panel panel-other card" v-if="Status.Storage!=null">
          <img class="card-img" :src="require('Imgs/storage.png')">
          <div>
            <div class="card-title">Storage</div>
            <div class="card-content"> {{ Round((Status.Storage.Total - Status.Storage.Free) / Status.Storage.Total * 100, 1) }} %</div>
            <div>Total space {{ Round(Status.Storage.Total / 1024 / 1024, 1) }} MB</div>
          </div>
        </div>
        <div class="panel panel-other card" v-if="Status.Temperature!=null">
          <img class="card-img" :src="require('Imgs/temperature_normal.png')">
          <div>
            <div class="card-title">Temperature</div>
            <div class="card-content"> {{ Round(Status.Temperature, 1) }} ºC</div>
          </div>
        </div>
      </div>
      <div class="col-xs-12">
        <label class="control-label">System log messages</label>
        <textarea rows="10" class="form-control" disabled :value="SysMessages"></textarea>
      </div>
      <div class="col-xs-12">
        <vuetable ref="table" :data="FilterMessages" :fields="Columns" :api-mode="false" class="table small table-striped"></vuetable>
      </div>
    </div>
  </div>
</template>

<script>
  import moment from 'moment';
  export default {
    data: function() {
      return { ShowSpinner: true, SysMessages: '', AllMessages: [], Messages: [], FilterMessages: [], Options: {}, LastUpdate: null, Timer: null, Status: {},
              Columns: [
                { name: 'Type', title: 'Type', callback: this.FormatType },
                { name: 'Date', title: 'Date', callback: this.FormatDate },
                { name: 'Message', title: 'Message' }
              ]
            };
    },
    created: function() {
      this.UpdateSystem();
    },
    beforeDestroy: function() {
      if(this.Timer != null) clearTimeout(this.Timer);
    },
    watch: {
      AllMessages: function() { this.Filter(); },
      Options: function() { this.Filter(); }
    },
    methods: {
      FormatDate: function(date) {
        return moment(date).format('DD/MM/YYYY - HH:mm:ss');
      },
      FormatType: function(val) {
        if(val == 'WARNING') return '<span style="color: #f0ad4e">' + val + '</span>';
        if(val == 'INFO') return '<span style="color: #318ce7">' + val + '</span>';
        if(val == 'ERROR') return '<span style="color: #d9534f">' + val + '</span>';
        if(val == 'DEBUG') return '<span style="color: #dddddd">' + val + '</span>';
      },
      Filter: function() {
        this.Messages = [];
        for(var n = 0; n < this.AllMessages.length; n++) {
          if((this.AllMessages[n].Type == 'DEBUG') && (this.Options.Debug == false)) continue;
          if((this.AllMessages[n].Type == 'WARNING') && (this.Options.Warning == false)) continue;
          if((this.AllMessages[n].Type == 'INFO') && (this.Options.Info == false)) continue;
          if((this.AllMessages[n].Type == 'ERROR') && (this.Options.Error == false)) continue;
          this.Messages.push(this.AllMessages[n]);
        }
      },
      UpdateSystem: function() {
        this.Post('getSystem', { Series: ['Status', 'Log', 'SystemLog']}).then(
          (result) => {
            this.AllMessages = [];
            this.LastUpdate = moment();
            for(var n = 1; n < result.Log.Messages.length; n++) this.AllMessages.push({Type: result.Log.Messages[n][0], Date: result.Log.Messages[n][1], Message: result.Log.Messages[n][2]});
            this.Status = result.Status;
            this.SysMessages = result.SystemLog;
            this.ShowSpinner = false;
            this.Timer = setTimeout(this.UpdateSystem, 5000);
          },
          (fail) => {
            this.Timer = setTimeout(this.UpdateSystem, 5000);
            this.Status = null;
            this.ShowSpinner = false;
          }
        );
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
    min-width: 100px;
    max-width: 1000px;
    margin: 3px;
    padding: 10px;
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
    line-height: 16px;
    font-size: 16px;
  }
  .card-content {
    line-height: 28px;
    font-size: 28px;
    font-weight: bold;
  }
  .header-text {
    font-size: 14px;
  }
  .table >>> th:nth-child(1) {
    text-align: center;
    width: 72px;
  }
  .table >>> td:nth-child(1) {
    text-align: center;
  }
  .table >>> th:nth-child(2) {
    text-align: center;
    width: 160px;
  }
  .table >>> td:nth-child(2) {
    text-align: center;
  }
  .main-panel {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
  textarea {
    resize: none;
    font-family: 'Fira Code';
    font-size: 10px;
    white-space: pre;
  }
</style>