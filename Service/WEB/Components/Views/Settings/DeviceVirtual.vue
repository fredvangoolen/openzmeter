<template>
  <div class="row">
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Nominal Voltage</label>
      <input-spin :min="20" :max="350" :value="230" :increment="10" ref="voltage"></input-spin>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Nominal Frequency</label>
      <input-spin :min="10" :max="100" :value="50" ref="freq" :increment="1"></input-spin>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Mode</label>
      <option-picker :options="['1PHASE', '3PHASE', '3PHASEN']" :selected="'1PHASE'" v-on:OptionChanged="ModeChanged" :big="true"></option-picker>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Samplerate</label>
      <input-spin :min="10000" :max="100000" :value="20000" ref="samplerate" :increment="100"></input-spin>
    </div>
  </div>
</template>

<script>
  import Vue from 'vue';
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import Spinner from 'Components/Spinner.vue';
  import * as oZm from 'JS/openZmeter.js';
  import OptionPicker from 'Components/OptionPicker.vue';
  import InputSpin from 'Components/InputSpin.vue';
  export default {
    data() {
      return {
        Show: false, ShowSpinner: true,
        Columns: [
          {name: 'Online', title: 'Online', callback: 'OnlineStatus'},
          {name: 'Name', title: 'Description'}, 
          {name: 'ID', title: 'Serial'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    methods: {
      ModeChanged: function() {
        
      },
      GetAnalyzers() {
        $.ajax({ url: "getAnalyzers", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) { 
            var TableData = { data: result };
            this.$refs.table.setData(TableData);
            this.ShowSpinner = false;
          }.bind(this)
        });
      },
      OnlineStatus: function(value) {
        return (value === true) ? '&#9635;' : '&#9633;';
      },
      Load: function(analyzers) {
        this.Show = true;
        this.ShowSpinner = true;
        this.$nextTick(function() {
//          this.GetAnalyzers();
        });
      },
      CurrentlySelected: function(id) {
        if(this.$root.SelectedAnalyzer === null) return false;
        if(id.ID === this.$root.SelectedAnalyzer.ID) return true;
        return false;
      },
      Select: function(id) {
        localStorage.setItem("SelAnalyzer", id.ID);
        this.$root.SelectedAnalyzer = id;
        this.$nextTick(function() {
          this.$root.$emit('AnalyzerChanged');
          this.$parent.Close();
        });
      },
      Configure: function(id) {
        this.Show = false;
        this.$parent.$refs.Configure.Load(id.ID); 
      },
      ACL: function(id) {
        alert("ACL not yet implemented"); 
      },
      Tariffs: function(id) { 
        this.Show = false;
        this.$parent.$refs.Tariffs.Load(id.ID); 
      },
      Alerts: function(id) {
        alert("Alerts not yet implemented");
      },
      Maintenance: function(id) {
        alert("Maintenance not yet implemented");
      }
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    components: { Vuetable, Spinner, OptionPicker, InputSpin }
  }
</script>

<style scoped>
@media screen and (min-width: 600px) {
  .table >>> th:nth-child(1) {
    width: 60px;
    text-align: center;
  }
  .table >>> td:nth-child(1) {
    text-align: center;
    font-size: 14px;
  }
  .table >>> th:nth-child(3) {
    width: 180px;
    text-align: center;
  }
  .table >>> td:nth-child(3) {
    text-align: center;
  }
  .table >>> th:nth-child(4) {
    width: 90px;
  }
}
</style>
