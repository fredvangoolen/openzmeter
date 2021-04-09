<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Remote access</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" :class="{ disabled: ShowSpinner }" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', null)"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" :class="{ active: (View=='Main') }" @click="View='Main'">Main</button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: (View=='MQTT') }" @click="View='MQTT'">MQTT</button>
      </div>
    </template>
    <div class="row" v-if="View=='Main'">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable remote access</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Analyzer.Sync.Enabled"></toggle>
        <div class="help">Enable remote access throw openZmeter network</div>
      </div>
      <div class="col-xs-12"><hr style="margin: 5px 0px 5px 0px"/></div>
      <div class="col-xs-12">
        <label class="control-label">Remote URL</label>
        <input type="text" class="form-control" v-model="Analyzer.Sync.Server">
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">User name</label>
        <input type="text" class="form-control" v-model="Analyzer.Sync.User">
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Password</label>
        <password v-model="Analyzer.Sync.Pass"></password>
      </div>
    </div>
    <div class="row" v-if="View=='MQTT'">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable publish</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Analyzer.MQTT.Enabled"></toggle>
        <div class="help">Publish analyzer data to MQTT broker. Only selected data is exported.</div>
      </div>
      <div class="col-xs-12"><hr style="margin: 5px 0px 5px 0px"/></div>
      <div class="col-xs-8">
        <label class="control-label" style="line-height: 22px">Broker</label>
        <input type="text" class="form-control" v-model="Analyzer.MQTT.Server">
      </div>
      <div class="col-xs-4">
        <label class="control-label" style="line-height: 22px">Prefix</label>
        <input type="text" class="form-control" v-model="Analyzer.MQTT.Prefix">
      </div>
      <div class="col-xs-6">
        <label class="control-label" style="line-height: 22px">User</label>
        <input type="text" class="form-control" v-model="Analyzer.MQTT.User">
      </div>
      <div class="col-xs-6">
        <label class="control-label" style="line-height: 22px">Password</label>
        <password v-model="Analyzer.MQTT.Pass"></password>
      </div>
      <vuetable ref="table" :fields="Columns" :api-mode="false" :data="Rows" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <input type="checkbox" v-model="Analyzer.MQTT[props.rowData.Export]"/>
          </div>
        </template>
      </vuetable>
    </div>
  </dlg>
</template>

<script>
  import vuetable from 'vuetable-2';
  export default {
    props: { analyzer: Object },
    data: function() {
      return { ShowSpinner: false, Analyzer: null, View: 'Main',
               Columns: [
                { name: 'Source', title: 'Source' },
                { name: 'Topic', title: 'Topic', callback: this.TopicName },
                { name: '__slot:actions', title: 'Enabled' }
              ],
              Rows: [
                { Source: '200 miliseconds aggregation', Topic: '200MS', Export: 'Export200MS' },
                { Source: '3 seconds aggregation', Topic: '3S', Export: 'Export3S' },
                { Source: '1 minute aggregation', Topic: '1M', Export: 'Export1M' },
                { Source: '10 minutes aggregation', Topic: '10M', Export: 'Export10M' },
                { Source: '15 minutes aggregation', Topic: '15M', Export: 'Export15M' },
                { Source: '1 hour aggregation', Topic: '1H', Export: 'Export1H' },
                { Source: 'Alarms', Topic: 'Alarms', Export: 'ExportAlarms' },
                { Source: 'Config', Topic: 'Config', Export: 'ExportConfig' }
              ]
      };
    },
    created: function() {
      this.Analyzer = Object.assign({}, this.analyzer);
    },
    methods: {
      TopicName: function(val) {
        return (this.Analyzer.MQTT.Prefix != "") ? this.Analyzer.MQTT.Prefix + "/" + this.Analyzer.Analyzer + "/" + val : this.Analyzer.Analyzer + "/" + val;
      },
      Save: function() {
        this.ShowSpinner = true;
        this.Post('setAnalyzer', this.Analyzer).then(
          () => { this.$emit('close', this.Analyzer); },
          (fail) => {
            this.ShowSpinner = false;
            if(fail != null) this.$toast.open({type:'error', message: "Failed to save analyzer config<br>" + fail.message, duration: 3000});
          }
        );
      }
    },
    components: { vuetable }
  }
</script>

<style scoped>
  .help {
    font-size: 80%;
  }
  .table >>> td:nth-child(3) {
    text-align: center;
  }
  .table >>> th:nth-child(3) {
    width: 60px;
  }
</style>