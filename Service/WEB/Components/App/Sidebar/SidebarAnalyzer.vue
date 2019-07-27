<template>
  <div>
    <div class="sidenav-header">
      <div>{{ ($root.SelectedAnalyzer == null) ? 'None selected' : $root.SelectedAnalyzer.Name }}</div>
      <div class="btn-group pull-right">
        <button type="button" class="btn btn-default btn-xs" @click="ShowDialog"><img :src="require('Imgs/magnifier.png')"></button>
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
      return { SelAnalyzer: null };
    },
    mounted: function() {
      this.CheckAnalyzer();
    },
    methods: {
      CheckAnalyzer: function() {
        if(localStorage.getItem("SelAnalyzer") === undefined) {
          this.$refs.Dialog.Show();
        } else {
          $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: localStorage.getItem("SelAnalyzer") }), 
            success: function(result) { 
              this.$root.SelectedAnalyzer = result;  
              this.$root.$emit('AnalyzerChanged', result);
            }.bind(this),
            error: function() {
              this.$refs.Dialog.Show();
            }.bind(this)
          });
        }
      },
      ShowDialog() {
        this.$refs.Dialog.Show(this.AvaliableAnalyzers);
      },
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