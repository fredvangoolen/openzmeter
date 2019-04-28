<template>
  <div class="row">
    <div class="col-xs-12 col-sm-6 col-lg-4">
      <label class="control-label">Name</label>
      <input type="text" class="form-control" id="inputName">
    </div>
    <div class="col-xs-12 col-sm-6 col-lg-4">
      <label class="control-label">Surname</label>
      <input type="text" class="form-control" id="inputSurname">
    </div>
    <div class="col-xs-12 col-sm-6 col-lg-4">
      <label class="control-label">Country</label>
      <input type="text" class="form-control" id="inputIDCard">
    </div>
    <div class="col-xs-12 col-sm-6 col-lg-6">
      <label class="control-label">Company</label>
      <input type="text" class="form-control" id="inputCompany">
    </div>
    <div class="col-xs-12 col-lg-6">
      <label class="control-label">Address</label>
      <input type="text" class="form-control" id="inputAddress">
    </div>
  </div>
</template>

<script>
  import Vue from 'vue';
  import Vuetable from 'vuetable-2';
  import Spinner from 'Components/Spinner.vue';
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
    components: { Vuetable, Spinner }
  }
</script>

<style scoped>
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
</style>
