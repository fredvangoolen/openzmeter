<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Seasons</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Add"><img src="Imgs/plus.png" alt="New season"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img src="Imgs/arrow_undo.png" alt="Go back"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px;">Define seasons of the year</div>
      <div class="row">
        <div class="col-xs-12">
        <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
        </vuetable>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  export default {
    data() {
      return { Show: false, Tariff: null, Columns: [] };
    },
    methods: {
      FillTable: function() {
//                  <thead><tr><th>Name</th><th style="width:80px">From</th><th style="width:80px">To</th><th style="width:140px"></th></tr></thead>

    var Rows = [];
          for(var i = 0; i < this.Tariff.Details.Periods.length; i++) Rows.push( { Name: 'P' + (i + 1), PowerMode: this.Tariff.Details.Periods[i].PowerMode, ReactiveMode: this.Tariff.Details.Periods[i].ReactiveMode });
          this.$refs.table.setData({ data: Rows });        
      },
      Load: function(tariff) {
        this.Show = true;
        this.Tariff = tariff;
        this.$nextTick(function() {
          this.FillTable();
        });
      },
//      Delete: function() {
//        this.Tariff.Details.Periods.splice(-1, 1);
//        this.FillTable();
//      },
      Add: function() {
        this.Tariff.Details.Periods.push({PowerMode: null, ReactiveMode: null});
        this.FillTable();
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Main.Load(this.Tariff);
      }
    },
    components: { Vuetable }
  }
</script>