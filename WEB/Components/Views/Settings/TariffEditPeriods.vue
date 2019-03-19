<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Periods</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Delete" v-bind:class="{ disabled: Tariff.Details.Periods.length == 1 }"><img :src="require('Imgs/cross.png')" alt="Delete period"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="New period"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Go back"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px;">Define the maximum periods of days and their pricing model</div>
      <div class="row">
        <div class="col-xs-12">
          <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
            <template slot="color" slot-scope="props">
              <color-picker selected="#FFFFFF" :columns="2" :colors="['#fffff0', '#ffecd6', '#ffd8bc', '#ffc4a2', '#ffae88', '#ff986c', '#ff7f50', '#f0ffff', '#d4f2f1', '#b7e5e2', '#98d8d3', '#78ccc5', '#55bfb8', '#20b2aa']"></color-picker>
            </template>
          </vuetable>
        </div>
      </div>
    </div>
  </div> 
</template>

<script>
  import Vuetable from 'vuetable-2';
  import ColorPicker from 'Components/ColorPicker.vue';
  export default {
    data() {
      return { Show: false, Tariff: {}, Columns: [
          {name: 'Name', title: 'Name'},
          {name: 'PowerMode', title: 'Power mode', callback: 'Format'},
          {name: 'ReactiveMode', title: 'Reactive mode', callback: 'Format'},
          {name: '__slot:color', title: 'Color'}
        ] };
    },
    methods: {
      Format: function(val) {
        return (val === null) ? 'NONE' : val; 
      },
      FillTable: function() {
          var Rows = [];
          for(var i = 0; i < this.Tariff.Details.Periods.length; i++) Rows.push( { Name: 'P' + (i + 1), PowerMode: this.Tariff.Details.Periods[i].PowerMode, ReactiveMode: this.Tariff.Details.Periods[i].ReactiveMode });
          this.$refs.table.setData({ data: Rows });        
      },
      Load: function(tariff) {
        this.Tariff = tariff;
        this.Show = true;
        this.$nextTick(function() {
          this.FillTable();
        });
      },
      Delete: function() {
        this.Tariff.Details.Periods.splice(-1, 1);
        this.FillTable();
      },
      Add: function() {
        this.Tariff.Details.Periods.push({PowerMode: null, ReactiveMode: null});
        this.FillTable();
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Main.Load(this.Tariff);
      }
    },
    components: { Vuetable, ColorPicker }
  }
</script>

<style scoped>
  .table >>> th:nth-child(1)  {
    width: 50px;
    text-align: center;
  }
  .table >>> td:nth-child(1) {
    text-align: center;
  }
  .table >>> th:nth-child(4) {
    width: 65px;
  }
</style>