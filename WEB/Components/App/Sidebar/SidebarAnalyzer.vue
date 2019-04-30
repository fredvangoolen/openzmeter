<template>
  <div>
    <div class="sidenav-header">
      <table>
        <tr>
          <td><img :src="require('Imgs/system_monitor.png')">00:00:00</td>
          <td><img :src="require('Imgs/sd_memory_adapter.png')">90%</td>
      <td><img :src="require('Imgs/ddr_memory.png')">50%</td>
      <td><img :src="require('Imgs/chip.png')">20%</td>
      <td><img :src="require('Imgs/network_wireless.png')">45%</td>
      <td><img :src="require('Imgs/battery.png')">100%</td>
        </tr>
      </table>
      <div>{{ analyzerName }}</div>
      <div class="btn-group pull-right">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{disabled: !hasAnalyzers}" @click="ShowDialog"><img :src="require('Imgs/magnifier.png')"></button>
        <button type="button" class="btn btn-default btn-xs disabled"><img :src="require('Imgs/exclamation.png')"></button>
      </div>
      <div class="clearfix"></div>
      

      
      
      <!--table>
        <tr><td>Disk:</td><td><progress-bar :value="disk"></progress-bar></td></tr>
        <tr><td>Battery:</td><td><progress-bar :value="battery"></progress-bar></td></tr>
        <tr><td>Usage:</td><td>{{ usage }}</td></tr>              
        <tr><td>Uptime:</td><td>{{ uptime }}</td></tr>
      </table-->
    </div>
    <analyzer-dialog ref="Dialog"></analyzer-dialog>
  </div> 
</template> 

<script>
  import ProgressBar from 'Components/ProgressBar.vue';
  import AnalyzerDialog from './Analyzer/Analyzer.vue';
  export default {
    data() {
      return { AvaliableAnalyzers: [], hasAnalyzers: false, SelAnalyzer: null, analyzerName: 'test', disk: 10, battery: 10, usage: 'poco', uptime: 'mucho'};
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
      ShowDialog() {
        this.$refs.Dialog.Show(this.AvaliableAnalyzers);
      },
      CheckSelected() {
        if(this.AvaliableAnalyzers.length === 0) {
          this.ShowDialog();
          this.hasAnalyzers = false;
        } else {
          this.hasAnalyzers = true;
          for(var i = 0; i < this.AvaliableAnalyzers.length; i++) {
            if(this.AvaliableAnalyzers[i].ID === localStorage.getItem("SelAnalyzer")) this.SelAnalyzer = this.AvaliableAnalyzers[i];
          }
          if(this.SelAnalyzer === null) this.ShowDialog();
          this.$root.SelectedAnalyzer = this.SelAnalyzer;  
          this.$root.$emit('AnalyzerChanged');
        }
      }
    },
    components: { ProgressBar, AnalyzerDialog }
  }
</script>

<style scoped>
  .sidenav-header {
    background-color: #474544;
  }
  .sidenav-header td {
    text-align: center;
  }
  .sidenav-header > table {
    width: 100%;
    font-size: 10px;
    line-height: 12px;
    margin-top: 4px;
    margin-bottom: 4px;
  }
  .sidenav-header > table img {
    margin-right: 1px;
  }
  .sidenav-header > div:nth-child(2) {
    width: 186px;
    display: inline-block;
}
</style>