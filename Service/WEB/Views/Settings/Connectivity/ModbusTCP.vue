<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">ModbusTCP</h4></div>
      <div class="pull-right btn-group">
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Config.Analyzers.push('ANALYZER_SERIAL')" data-toggle="tooltip" title="Add exported analyzer"><img :src="require('Imgs/plus.png')"></button>
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Save and close" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Back" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
        <toolbar-pager class="pull-right" :pageSize="10" :input="Analyzers" :output.sync="FilterAnalyzers" :filtercols="['id', 'Serial']"></toolbar-pager>
      </div>
    </template>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Config.Enabled"></toggle>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 5px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12">
        <label class="control-label">Port</label>
        <input-spin v-model="Config.Port" :big="true" :decimals="0" :min="1" :max="65535" :increment="1"></input-spin>
      </div>
      <div class="col-xs-12">
        <vuetable :fields="Columns" :api-mode="false" :data="FilterAnalyzers" class="table small table-striped">
          <template slot="value" slot-scope="props">
            <div class="input-group input-group-xs">
              <span class="input-group-btn">
                <button class="btn btn-default" type="button" @click="Config.Analyzers.splice(props.rowData.id, 1)"><img :src="require('Imgs/cross.png')"></button>
              </span>
              <input type="text" class="form-control" v-model="Config.Analyzers[props.rowData.id]">
            </div>
          </template>
        </vuetable>
      </div>
    </div>
  </dlg>
</template>

<script>
  import vuetable from 'vuetable-2';
  export default {
    props: { settings: Object },
    data: function() {
      return { ShowSpinner: false, Config: null, Analyzers: [], FilterAnalyzers: [],
        Columns: [
          { name: '__slot:value', title: 'Analyzer serial' },
          { name: 'id', title: 'Unit Identifier' },
        ]
      };
    },
    watch: {
      'Config.Analyzers': function() {
        this.Analyzers = [];
        for(var n = 0; n < this.Config.Analyzers.length; n++) this.Analyzers.push({id: n, Serial: this.Config.Analyzers[n]});
      }
    },
    created: function() {
      this.Config = Object.assign({ Analyzers: [] }, this.settings);
    },
    methods: {
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post('setModbusTCP', this.Config).then(
          (result) => {
            this.$emit('update:settings', this.Config);
            this.$emit('close');
          },
          (fail) => {
            this.ShowSpinner = false;
          }
        );
      }
    },
    components: { vuetable }
  }
</script>

<style scoped>
  .table >>> th:nth-child(2) {
    width: 100px;
    text-align: center;
  }
  .table >>> td:nth-child(2) {
    text-align: center;
  }
</style>