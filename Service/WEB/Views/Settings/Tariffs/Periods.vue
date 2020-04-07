<template>
  <dlg>
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Periods</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Delete" :disabled="Tariff.Template.Periods.length == 1"><img :src="require('Imgs/cross.png')" alt="Delete period"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="New period"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', Tariff)"><img :src="require('Imgs/arrow_undo.png')" alt="Go back"></button>
      </div>
    </template>
    <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px;">Define the maximum periods of days and their pricing model</div>
    <div class="row">
      <div class="col-xs-12">
        <vuetable ref="table" :fields="Columns" :api-mode="false" :data="Tariff.Template.Periods" class="table small table-striped">
          <template slot="color" slot-scope="props">
            <color-picker v-model="props.rowData.Color" :columns="2" :colors="['#fffff0', '#ffecd6', '#ffd8bc', '#ffc4a2', '#ffae88', '#ff986c', '#ff7f50', '#f0ffff', '#d4f2f1', '#b7e5e2', '#98d8d3', '#78ccc5', '#55bfb8', '#20b2aa']"></color-picker>
          </template>
          <template slot="power" slot-scope="props">
            <option-picker v-model="props.rowData.PowerMode" :options="[ {name: 'NONE', value: 'NONE'}, {name: 'ESP_3.x', value: 'ESP_3.x'}, {name: 'ESP_6.x (P1)', value: 'ESP_6.x (P1)'}, {name: 'ESP_6.x (P2)', value: 'ESP_6.x (P2)'}, {name: 'ESP_6.x (P3, P4, P5)', value: 'ESP_6.x (P3, P4, P5)'}, {name:'ESP_6.x (P6)', value:'ESP_6.x (P6)'}]"></option-picker>
          </template>
          <template slot="reactive" slot-scope="props">
            <option-picker v-model="props.rowData.ReactiveMode" :options="[ {name: 'NONE', value: 'NONE'}, {name: 'ESP', value: 'ESP'} ]"></option-picker>
          </template>
        </vuetable>
      </div>
    </div>
  </dlg>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import Spinner from 'Components/Spinner.vue';
  import * as oZm from 'JS/openZmeter.js';
  import ColorPicker from 'Components/ColorPicker.vue';
  import OptionPicker from 'Components/OptionPicker.vue';
  export default {
    props: { tariff: Object },
    data() {
      return {
        Columns: [
          {name: 'Name', title: 'Name' },
          {name: '__slot:power', title: 'Power mode', },
          {name: '__slot:reactive', title: 'Reactive mode'},
          {name: '__slot:color', title: 'Colour'}
        ]
      };
    },
    created: function() {
      this.Tariff = Object.assign({}, this.tariff);
      for(var i in this.Tariff.Template.Periods) this.Tariff.Template.Periods[i].Name = 'P' + (parseInt(i) + 1);
    },
    methods: {
      Delete: function() {
        if(confirm("Delete last period?")) this.Tariff.Template.Periods.splice(-1, 1);
      },
      Add: function() {
        this.Tariff.Template.Periods.push({PowerMode: 'NONE', ReactiveMode: 'NONE', Color: '#fffff0', Name: 'P' + (this.tariff.Template.Periods.length + 1)});
      }
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    components: { Vuetable }
  }
</script>

<style scoped>
@media screen and (min-width: 600px) {
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
}
</style>