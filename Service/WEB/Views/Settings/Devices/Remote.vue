<template>
  <dlg :loading="Loading">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Device &raquo; {{ Settings.Serial }}</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', false)"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
      <toolbar-pager class="pull-right" :pageSize="10" :input="Settings.Analyzers" :output.sync="FilterAnalyzers" :filtercols="['User', 'Analyzer']"></toolbar-pager>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null">{{ Error }}</div>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enabled</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Settings.Enabled"></toggle>
        <div class="help">While enabled authorized user can connect to the service. Each new shared analyzer will be added to the analyzer list on first connection. Users an permissions on this machine will be applied.</div>
        <hr style="margin: 5px 0px 5px 0px"/>
      </div>
      <div class="col-xs-12">
        <div class="input-group input-group-xs">
          <span class="input-group-addon">Analyzer</span>
          <input type="text" class="form-control" v-model="NewAnalyzer">
          <span class="input-group-btn"><button type="button" class="btn btn-default" @click="Add" :disabled="!CanAdd"><img :src="require('Imgs/plus.png')"></button></span>
        </div>
      </div>
    </div>
    <vuetable ref="table" :fields="Columns" :api-mode="false" :data="FilterAnalyzers" class="table small table-striped">
      <template slot="actions" slot-scope="props">
        <div class="btn-group">
          <button type="button" class="btn btn-danger btn-xs" @click="Delete(props.rowData.Analyzer)">Delete</button>
        </div>
      </template>
      <template slot="user" slot-scope="props">
        <div class="input-group input-group-xs">
          <input type="text" class="form-control rounded" v-model="props.rowData.User">
        </div>
      </template>
    </vuetable>
  </dlg>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  export default {
    props: { settings: Object },
    data() {
      return { Loading: false, Settings: null, Error: null, NewAnalyzer: '', FilterAnalyzers: [],
        Columns: [
          { name: 'Analyzer', title: 'Analyzer' },
          { name: '__slot:user', title: 'Owner' },
          { name: 'Size', title: 'Size', callback: this.BestUnitsBin },
          { name: '__slot:actions', title: ''}
        ]
      };
    },
    computed: {
      CanAdd: function() {
        return (this.NewAnalyzer.match("^[0-9A-F]{24}(_CAL|_CH1|_CH2|_CH3|_CH4)?(_1P|_3P|_3PN)$") != null);
      }
    },
    updated: function() {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    created: function() {
      this.Settings = Object.assign({}, this.settings);
    },
    methods: {
      Add: function() {
        for(var n = 0; n < this.Settings.Analyzers.length; n++) {
          if(this.Settings.Analyzers[n].Analyzer == this.NewAnalyzer) {
            this.NewAnalyzer = '';
            return;
          }
        }
        this.Settings.Analyzers.push({ User: 'admin', Analyzer: this.NewAnalyzer, Size: 0 });
        this.NewUser = '';
        this.NewAnalyzer = '';
      },
      Delete: function(row) {
        if(confirm("Delete '" + row + "' analyzer?") == false) return;
        for(var n = 0; n < this.Settings.Analyzers.length; n++) {
          if(this.Settings.Analyzers[n].Analyzer != row) continue;
          this.Settings.Analyzers.splice(n, 1);
          return;
        }
      },
      Save: function () {
        this.Loading = true;
        this.$http.post('setDevice', this.Settings).then(
          (result) => { this.$emit('close', true); },
          (fail) => {
            this.Loading = false;
            this.Error = 'Failed to save device settings!';
            setTimeout(() => { this.Error = null; }, 1000);
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
  .rounded {
    border-radius: 4px !important;
  }
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(2) {
      width: 120px;
    }
    .table >>> th:nth-child(3) {
      width: 100px;
      text-align: right;
    }
    .table >>> td:nth-child(3) {
      text-align: right;
    }
    .table >>> th:nth-child(4) {
      width: 62px;
    }
    .table >>> td:nth-child(4) {
      text-align: right;
    }
  }

</style>