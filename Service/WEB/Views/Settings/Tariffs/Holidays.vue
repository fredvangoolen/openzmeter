<template>
  <div>
    <dlg v-if="View==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Holidays</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="New holiday"></button>
          <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Go back"></button>
        </div>
      </template>
      <div class="row">
        <div class="col-xs-12">
          <vuetable ref="table" :fields="Columns" :api-mode="false" :data="tariff.Template.Holidays" class="table small table-striped">
            <template slot="actions" slot-scope="props">
              <div class="btn-group" style="width:100%">
                <button class="btn btn-xs btn-default" @click="Edit(props.rowIndex, $event)">Edit</button>
                <button class="btn btn-xs btn-danger" @click="Delete(props.rowIndex, $event)">Delete</button>
              </div>
            </template>
          </vuetable>
        </div>
      </div>
    </dlg>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import Holiday from './Holiday.vue';
  export default {
    props: { tariff: Object },
    data() {
      return { View: null,
        Columns: [
          {name: 'Name', title: 'Name'},
          {name: 'StartDate', title: 'Start date', callback: 'DateFormat'},
          {name: 'EndDate', title: 'End date', callback: 'DateFormat'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    methods: {
      DateFormat: function(val) {
        return moment.tz(val, this.Tariff.TimeZone).format('DD/MM/YYYY');
      },
      FillTable: function() {
        this.$refs.table.setData({ data: this.Tariff.Template.Holidays });
      },
      Load: function(tariff) {
        this.Show = true;
        this.Tariff = tariff;
        this.ShowSpinner = true;
        this.$nextTick(function() {
          this.FillTable();
        });
      },
      Delete: function(index) {
        if(confirm("Delete '"+this.Tariff.Template.Holidays[index].Name +"' holidays?")){
          this.Tariff.Template.Holidays.splice(index, 1);
          this.FillTable();
        }
      },
      Add: function() {
        this.Tariff.Template.Holidays.push({ StartDate: this.Tariff.StartDate, EndDate: this.Tariff.EndDate, Name: 'Unnamed holiday' });
        this.Show = false;
        this.$parent.$refs.Holiday.Load(this.Tariff, this.Tariff.Template.Holidays.length - 1);
      },
      Edit: function(index) {
        this.Show = false;
        this.$parent.$refs.Holiday.Load(this.Tariff, index);
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Main.Load(this.Tariff);
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
  .table >>> th:nth-child(2), .table >>> th:nth-child(3)  {
    width: 100px;
    text-align: center;
  }
  .table >>> td:nth-child(2), .table >>> td:nth-child(3) {
    text-align: center;
  }
  .table >>> th:nth-child(4) {
    width: 100px;
  }
  .table >>> td:nth-child(4) {
    text-align: right;
  }
}
</style>
