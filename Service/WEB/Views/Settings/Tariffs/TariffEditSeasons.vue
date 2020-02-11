<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Seasons</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="New season"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Go back"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px;">Define seasons of the year</div>
      <div class="row">
        <div class="col-xs-12">
        <spinner v-if="ShowSpinner"></spinner>
        <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped">
          <template slot="actions" slot-scope="props">
            <div class="btn-group" style="width:100%">
              <button class="btn btn-xs btn-default" @click="Duplicate(props.rowIndex)">Duplicate</button>
              <button class="btn btn-xs btn-default" @click="Edit(props.rowIndex)">Edit</button>
              <button class="btn btn-xs btn-danger" @click="Delete(props.rowIndex)">Delete</button>
            </div>
          </template>
        </vuetable>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import Spinner from 'Components/Spinner.vue';
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  export default {
    data() {
      return { Show: false, ShowSpinner: true, Tariff: null, 
        Columns: [
          {name: 'Name', title: 'Name' },
          {name: 'StartDate', title: 'Start date', callback: 'DateFormat' },
          {name: 'EndDate', title: 'End date', callback: 'DateFormat' }, 
          {name: '__slot:actions', title: '' }
        ]
      };
    },
    methods: {
      DateFormat: function(val) {
        return moment.tz(val, this.Tariff.TimeZone).format('DD/MM/YYYY');
      },
      FillTable: function() {
        this.$refs.table.setData({ data: this.Tariff.Template.Seasons });  
        this.ShowSpinner = false;
      },
      Load: function(tariff) {
        this.Show = true;
        this.Tariff = tariff;
        this.ShowSpinner=true;
        this.$nextTick(function() {
          this.FillTable();
        });
      },
      Duplicate: function(index) {
        var Row = $.extend(true, {}, this.Tariff.Template.Seasons[index]);
        Row.Name = Row.Name + "_Copy";
        var Row = this.Tariff.Template.Seasons.push(Row);
        this.Show = false;
        this.$parent.$refs.Season.Load(this.Tariff, this.Tariff.Template.Seasons.length - 1);
      },
      Delete: function(index) {
        if(confirm("Delete '"+this.Tariff.Template.Seasons[index].Name +"' season?")){
          this.Tariff.Template.Seasons.splice(index, 1);
          this.FillTable();
        }
      },
      Add: function() {
        this.Tariff.Template.Seasons.push({ StartDate: this.Tariff.StartDate, EndDate: this.Tariff.EndDate, DayPeriods: new Array(24).fill(0), Name: 'Unnamed season' });
        this.Show = false;
        this.$parent.$refs.Season.Load(this.Tariff, this.Tariff.Template.Seasons.length - 1);
      },
      Edit: function(index) {
        this.Show = false;
        this.$parent.$refs.Season.Load(this.Tariff, index);
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
      width: 150px;
    }
    .table >>> td:nth-child(4) {
      text-align: right;
    }
  }
</style>
