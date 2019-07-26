<template>
  <div class="row">
    <div class="col-xs-12">
      <label class="control-label">Start on boot</label>
      <toggle></toggle>
    </div>
    
    
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Voltage channel Gain</label>
      <input-spin :min="-10" :max="10" :value="0" :increment="0.001" ref="voltageGain" prepend="%"></input-spin>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Voltage channel Offset</label>
      <input-spin :min="-20" :max="20" :value="0" :increment="0.1" ref="voltageOffset" prepend="V"></input-spin>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Current channel Gain</label>
      <input-spin :min="-10" :max="10" :value="0" :increment="0.001" ref="currentGain" prepend="%"></input-spin>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Current channel Offset</label>
      <input-spin :min="-1" :max="1" :value="0" :increment="0.001" ref="currentOffset" prepend="A"></input-spin>
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
  import Toggle from 'Components/Toggle.vue';
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


