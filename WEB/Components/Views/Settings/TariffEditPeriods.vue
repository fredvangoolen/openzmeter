<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Periods</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Delete" v-bind:class="{ disabled: Tariff.Template.Periods.length == 1 }"><img :src="require('Imgs/cross.png')" alt="Delete period"></button>
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
              <color-picker :selected="props.rowData.Color" :columns="2" v-on:ColorChanged="ColorChanged(props.rowIndex, $event)" :colors="['#fffff0', '#ffecd6', '#ffd8bc', '#ffc4a2', '#ffae88', '#ff986c', '#ff7f50', '#f0ffff', '#d4f2f1', '#b7e5e2', '#98d8d3', '#78ccc5', '#55bfb8', '#20b2aa']"></color-picker>
            </template>
            <template slot="power" slot-scope="props">
              <option-picker :selected="props.rowData.PowerMode" v-on:OptionChanged="PowerChanged(props.rowIndex, $event)" :options="['NONE', 'ESP_3.x', 'ESP_6.x']"></option-picker>
            </template>
            <template slot="reactive" slot-scope="props">
              <option-picker :selected="props.rowData.ReactiveMode" v-on:OptionChanged="ReactiveChanged(props.rowIndex, $event)" :options="['NONE', 'ESP']"></option-picker>
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
  import OptionPicker from 'Components/OptionPicker.vue';
  export default {
    data() {
      return { Show: false, Tariff: {}, Columns: [
          {name: 'Name', title: 'Name'},
          {name: '__slot:power', title: 'Power mode'},
          {name: '__slot:reactive', title: 'Reactive mode'},
          {name: '__slot:color', title: 'Color'}
        ] };
    },
    methods: {
      ColorChanged: function(index, color) {
        this.Tariff.Template.Periods[index].Color = color;
      },
      PowerChanged: function(index, option) {
        this.Tariff.Template.Periods[index].PowerMode = option;
      },
      ReactiveChanged: function(index, option) {
        this.Tariff.Template.Periods[index].ReactiveMode = option;
      },
      Format: function(val) {
        return (val === null) ? 'NONE' : val; 
      },
      FillTable: function() {
          var Rows = [];
          for(var i = 0; i < this.Tariff.Template.Periods.length; i++) Rows.push( { Name: 'P' + (i + 1), PowerMode: this.Tariff.Template.Periods[i].PowerMode, ReactiveMode: this.Tariff.Template.Periods[i].ReactiveMode, Color: this.Tariff.Template.Periods[i].Color });
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
        this.Tariff.Template.Periods.splice(-1, 1);
        this.FillTable();
      },
      Add: function() {
        this.Tariff.Template.Periods.push({PowerMode: 'NONE', ReactiveMode: 'NONE', Color: '#fffff0'});
        this.FillTable();
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Main.Load(this.Tariff);
      }
    },
    components: { Vuetable, ColorPicker, OptionPicker }
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