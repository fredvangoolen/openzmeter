<template>
  <div v-if="analyzer!=null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <span class="header-text">Alarms</span>
            <div class="pull-right input-group input-group-xs" style="width:200px">
              <span class="input-group-btn">
                <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Add user"><img :src="require('Imgs/resultset_previous.png')"></button>
              </span>
              <input type="text" class="form-control" placeholder="Filter..." v-model="Filter">
              <span class="input-group-btn">
                <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Add user"><img :src="require('Imgs/resultset_next.png')"></button>
              </span>
            </div>
          </div>
          <div class="panel-body" style="min-height: 100px">
            <spinner v-if="ShowSpinner"></spinner>
            <vuetable ref="table" :fields="Columns" :data="Alarms" :api-mode="false" class="table small table-striped">
              <template slot="state" slot-scope="props">
                <img v-if="props.rowData.Triggered==false" :src="require('Imgs/alarm_release.png')">
                <img v-if="props.rowData.Triggered==true" :src="require('Imgs/alarm_trigger.png')">
              </template>
            </vuetable>
          </div>
        </div>
      </div>
    </div>
  </div>
  <div v-else-if="analyzer==null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  export default {
    props: { analyzer: Object },
    data: function() {
      return { ShowSpinner: true, Filter: '', Alarms: [],
      Columns: [
          {name: '__slot:state', title: ''},
          {name: 'AlarmTime', title: 'Date', callback: this.FormatDateTime },
          {name: 'Name', title: 'Name' }
        ] };
    },
    created: function () {
      if(this.analyzer != null) this.LoadAlarms();
    },
    watch: {
      Filter: function() {
        this.LoadAlarms();
      },
      analyzer: function() {
        this.LoadAlarms();
      }
    },
    methods: {
      FormatDateTime: function(val) { return this.analyzer.FormatDateTime(val); },
      LoadAlarms: function () {
        this.ShowSpinner = true;
        this.$http.post('getAlarms', { Limit: 10, Offset: 0, Filter: this.Filter, Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.Alarms = result.data;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      Delete: function(row) {

      }
    },
    components: { Vuetable }
  }
</script>

<style scoped>
  .table >>> th:nth-child(1) {
    width: 30px;
  }
  .table >>> th:nth-child(2), .table >>> td:nth-child(2) {
    width: 160px;
    text-align: center;
  }
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
