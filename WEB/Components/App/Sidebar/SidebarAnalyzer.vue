<template>
  <div>
  <div class="sidenav-header">
    <div>{{ analyzerName }}</div>
    <div class="btn-group pull-right">
      <button type="button" class="btn btn-default btn-xs" v-bind:class="{disabled: !hasAnalyzers}" @click="showDialog = true"><img src="Imgs/magnifier.png"></button>
      <button type="button" class="btn btn-default btn-xs disabled"><img src="Imgs/exclamation.png"></button>
    </div>
    <div class="clearfix"></div>
    <table>
      <tr><td>Disk:</td><td><progress-bar :value="disk"></progress-bar></td></tr>
      <tr><td>Battery:</td><td><progress-bar :value="battery"></progress-bar></td></tr>
      <tr><td>Usage:</td><td>{{ usage }}</td></tr>              
      <tr><td>Uptime:</td><td>{{ uptime }}</td></tr>
    </table>
  </div>
    <analyzer-dialog v-if="showDialog" @close="showDialog = false" :analyzers="AvaliableAnalyzers"></analyzer-dialog>
  </div> 
</template> 

<script>
  import ProgressBar from 'Components/ProgressBar.vue';
  import AnalyzerDialog from './Analyzer/Analyzer.vue';
  export default {
    data() {
      return { showDialog: false, AvaliableAnalyzers: [], hasAnalyzers: false, SelAnalyzer: null, analyzerName: 'test', disk: 10, battery: 10, usage: 'poco', uptime: 'mucho'};
    },
    mounted: function() {
      this.GetAnalyzers();
    },
    methods: {
      GetAnalyzers() {
        $.ajax({ url: "getAnalyzers", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) { this.AvaliableAnalyzers = result; this.CheckSelected(); }.bind(this)
        });
      },
      ShowAnalyzers() {
        this.showDialog = true;
      },
      CheckSelected() {
        if(this.AvaliableAnalyzers.length === 0) {
          this.ShowAnalyzers();
          this.hasAnalyzers = false;
        } else {
          this.hasAnalyzers = true;
          for(var i = 0; i < this.AvaliableAnalyzers.length; i++) {
            if(this.AvaliableAnalyzers[i].ID === localStorage.getItem("SelAnalyzer")) this.SelAnalyzer = this.AvaliableAnalyzers[i];
          }
          if(this.SelAnalyzer === null) this.ShowAnalyzers();
          this.$root.SelectedAnalyzer = this.SelAnalyzer;  
          this.$root.$emit('AnalyzerChanged');      
        }
      }
    },
    components: { ProgressBar, AnalyzerDialog }
  }
</script>

<style>
  .sidenav-header .progress {
    height: 12px;
    background-color: #474544;
    margin-bottom: 2px;
  }
  .sidenav-header .progress > div {
    background-color: #337ab7;
  }
  .sidenav-header td {
    text-align: right;
  }
  .sidenav-header > table {
    width: 100%;
    font-size: 11px;
    line-height: 12px;
    margin-top: 4px;
    margin-bottom: 4px;
  }
  .sidenav-header > table tr td:nth-child(2) {
    width: 100%;
    padding: 0px 0px 0px 10px;
    text-align: center;
  }
</style>