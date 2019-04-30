<template>
  <div>
    <div class="sidenav-header">
      <div>{{ analyzerName }}</div>
      <div class="btn-group pull-right">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{disabled: !hasAnalyzers}" @click="ShowDialog"><img :src="require('Imgs/magnifier.png')"></button>
        <button type="button" class="btn btn-default btn-xs disabled"><img :src="require('Imgs/exclamation.png')"></button>
      </div>
      <div class="clearfix"></div>
    </div>
    <analyzer-dialog ref="Dialog"></analyzer-dialog>
  </div> 
</template> 

<script>
  import ProgressBar from 'Components/ProgressBar.vue';
  import AnalyzerDialog from './Analyzer/Analyzer.vue';
  import moment from 'JS/moment.js';
  export default {
    data() {
      return { AvaliableAnalyzers: [], hasAnalyzers: false, SelAnalyzer: null, analyzerName: 'test'};
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
  .sidenav-header > div:first-child {
    width: 186px;
    display: inline-block;
}
</style>