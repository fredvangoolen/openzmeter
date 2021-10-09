<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">ModbusTCP</h4></div>
      <div class="pull-right btn-group">
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" title="Save and close" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
          <button type="button" class="btn btn-default btn-xs" title="Back" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
      </div>
    </template>
    <div class="row">
      <div class="col-xs-6">
        <label class="control-label" style="line-height: 22px">Enable</label>
        <q-toggle v-model="Config.Enabled" />
      </div>
      <div class="col-xs-6" style="display: flex">
        <label class="control-label">Port</label>
        <input-spin v-model="Config.Port" :decimals="0" :min="1" :max="65535" :increment="1" style="width: 100%; margin-left: 10px; margin-top: 1px"></input-spin>
      </div>
      <div class="col-xs-12">
        <detail-tabs :tabs="['Analyzers', 'Mapping']" v-model="ActiveTab" :light="true"></detail-tabs>
        <div key="Analyzers" class="panel-tab panel panel-dialog" style="margin-bottom: 0px" v-if="ActiveTab=='Analyzers'">
          <div class="panel-heading">
            <div class="pull-right btn-group">
              <button type="button" class="btn btn-default btn-xs" @click="Config.Analyzers.push('ANALYZER_SERIAL')" title="Add exported analyzer"><img :src="require('Imgs/plus.png')"></button>
            </div>
            <toolbar-pager class="pull-right" :pageSize="10" :input="Analyzers" :output.sync="FilterAnalyzers" :filtercols="['id', 'Serial']"></toolbar-pager>
            <div class="clearfix"></div>
          </div>
          <div class="panel-body">
            <vuetable :fields="ColumnsAnalyzers" :api-mode="false" :data="FilterAnalyzers" class="analyzers table small table-striped">
              <template slot="value" slot-scope="props">
                <div class="input-group input-group-xs">
                  <span class="input-group-btn">
                    <button class="btn btn-default" type="button" @click="Config.Analyzers.splice(props.rowData.id, 1)"><img :src="require('Imgs/cross.png')"></button>
                  </span>
                  <input-text v-model="Config.Analyzers[props.rowData.id]"></input-text>
                </div>
              </template>
            </vuetable>
          </div>
        </div>
        <div key="Mapping" class="panel panel-dialog panel-tab" v-if="ActiveTab=='Mapping'">
          <div class="panel-heading">
            <div class="pull-left btn-group">
              <option-picker v-model="Mode" :options="[ {name: '1PHASE', value: '1PHASE'}, {name: '3PHASE', value: '3PHASE'}, {name: '3PHASE+N', value: '3PHASE+N'} ]" ></option-picker>
            </div>
            <toolbar-pager class="pull-right" :pageSize="10" :input="Mappings" :output.sync="FilterMappings" :filtercols="['Address', 'Type', 'Description', 'Units']"></toolbar-pager>
            <div class="clearfix"></div>
          </div>
          <div class="panel-body">
            <vuetable :fields="ColumnsMappings" :api-mode="false" :data="FilterMappings" class="mappings table small table-striped"></vuetable>
          </div>
        </div>
      </div>
    </div>
  </dlg>
</template>

<script>
  export default {
    props: { settings: Object },
    data: function() {
      return { ShowSpinner: false, ActiveTab: null, Config: null, Analyzers: [], FilterAnalyzers: [], FilterMappings: [], Mode: '1PHASE',
        Harmonics: ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th', '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th', '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th', '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th'],
        ColumnsAnalyzers: [
          { name: '__slot:value', title: 'Analyzer serial' },
          { name: 'id', title: 'Unit Identifier' },
        ],
        ColumnsMappings: [
          { name: 'Register', title: 'Register', callback: this.FormatAddress },
          { name: 'Type', title: 'Data Type' },
          { name: 'Description', title: 'Description' },
          { name: 'Units', title: 'Units' },
        ]
      };
    },
    computed: {
      Mappings: function() {
        var Map = [];
        var Address = 0;
        var VoltageChannels = (this.Mode == '1PHASE') ? 1 : 3;
        var CurrentChannels = (this.Mode == '1PHASE') ? 1 : (this.Mode == '3PHASE') ? 3 : 4;
        Map.push({Register: Address, Type: 'STRING[32]', Description: 'Analyzer serial number', Units: ''});   Address += 16;
        Map.push({Register: Address, Type: 'UINT32', Description: 'Timestamp', Units: 's'});  Address += 2;
        Map.push({Register: Address, Type: 'UINT64', Description: 'Timestamp', Units: 'ms'}); Address += 4;
        Map.push({Register: Address, Type: 'FLOAT', Description: 'Frequency', Units: 'Hz'}); Address += 2;
        if(VoltageChannels > 1) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Unbalance', Units: '%'}); Address += 2; }
        for(var ch = 0; ch < VoltageChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Voltage CH' + (ch + 1), Units: 'V'}); Address += 2; }
        for(var ch = 0; ch < CurrentChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Current CH' + (ch + 1), Units: 'A'}); Address += 2; }
        for(var ch = 0; ch < VoltageChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Active power CH' + (ch + 1), Units: 'W'}); Address += 2; }
        for(var ch = 0; ch < VoltageChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Reactive power CH' + (ch + 1), Units: 'var'}); Address += 2; }
        for(var ch = 0; ch < VoltageChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Active energy CH' + (ch + 1), Units: 'Wh'}); Address += 2; }
        for(var ch = 0; ch < VoltageChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Reactive energy CH' + (ch + 1), Units: 'varh'}); Address += 2; }
        for(var ch = 0; ch < VoltageChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Apparent power CH' + (ch + 1), Units: 'va'}); Address += 2; }
        for(var ch = 0; ch < VoltageChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Power factor CH' + (ch + 1), Units: ''}); Address += 2; }
        for(var ch = 0; ch < VoltageChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Voltage THD CH' + (ch + 1), Units: '%'}); Address += 2; }
        for(var ch = 0; ch < CurrentChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Current THD CH' + (ch + 1), Units: '%'}); Address += 2; }
        if(VoltageChannels > 1) {
          for(var ch = 0; ch < VoltageChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Voltage Phase CH' + (ch + 1), Units: '°'}); Address += 2; }
        }
        for(var ch = 0; ch < VoltageChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: 'Current Phase (Phi) CH' + (ch + 1), Units: '°'}); Address += 2; }
        for(var h = 0; h < 50; h++) {
          for(var ch = 0; ch < VoltageChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: this.Harmonics[h] + ' Voltage Harmonic CH' + (ch + 1), Units: 'V'}); Address += 2; }
        }
        for(var h = 0; h < 50; h++) {
          for(var ch = 0; ch < CurrentChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: this.Harmonics[h] + ' Current Harmonic CH' + (ch + 1), Units: 'A'}); Address += 2; }
        }
        for(var h = 0; h < 50; h++) {
          for(var ch = 0; ch < VoltageChannels; ch++) { Map.push({Register: Address, Type: 'FLOAT', Description: this.Harmonics[h] + ' Power Harmonic CH' + (ch + 1), Units: 'W'}); Address += 2; }
        }
        return Map;
      }
    },
    watch: {
      'Config.Analyzers': function() {
        this.Analyzers = [];
        for(var n = 0; n < this.Config.Analyzers.length; n++) this.Analyzers.push({id: n, Serial: this.Config.Analyzers[n]});
      }
    },
    created: function() {
      this.Config = Object.assign({ }, this.settings);
      this.Analyzers = [];
      for(var n = 0; n < this.Config.Analyzers.length; n++) this.Analyzers.push({id: n, Serial: this.Config.Analyzers[n]});
    },
    methods: {
      FormatAddress: function(val) {
        return val.toString() + ' (0x' + ('000' + val.toString(16).toUpperCase()).slice(-4) + ')';
      },
      Save: function() {
        this.ShowSpinner = true;
        this.Post('setModbusTCP', this.Config).then(
          (result) => {
            this.$emit('update:settings', this.Config);
            this.$emit('close');
          },
          (fail) => {
            this.ShowSpinner = false;
          }
        );
      }
    }
  }
</script>

<style scoped>
  .panel-tab {
    margin-bottom: 0px;
    border-top-left-radius: 0px;
  }
  .analyzers.table >>> th:nth-child(2) {
    width: 100px;
    text-align: center;
  }
  .analyzers.table >>> td:nth-child(2) {
    text-align: center;
  }
  .mappings.table >>> th:nth-child(1) {
    width: 100px;
  }
  .mappings.table >>> th:nth-child(2) {
    width: 70px;
    text-align: center;
  }
  .mappings.table >>> td:nth-child(2) {
    text-align: center;
  }
  .mappings.table >>> th:nth-child(4) {
    width: 40px;
    text-align: center;
  }
  .mappings.table >>> td:nth-child(4) {
    text-align: center;
  }
</style>