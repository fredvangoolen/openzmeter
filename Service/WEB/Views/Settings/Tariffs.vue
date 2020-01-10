<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="huge-small pull-left">Tariffs</div>
      <button type="button" class="pull-right btn btn-default btn-xs" data-toggle="tooltip" title="Add tariff" @click="Add"><img :src="require('Imgs/plus.png')"> Add</button>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" :data="Tariffs" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" >
            <button class="btn btn-xs btn-default" @click="Edit(props.rowData, $event)">Edit</button>
            <button class="btn btn-xs btn-danger" @click="Delete(props.rowData, $event)">Delete</button>
          </div>
        </template>
      </vuetable>
    </div>
    <tariffs-edit ref="EditDialog"></tariffs-edit>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import moment from 'moment';
  import TariffsEdit from './TariffsEdit.vue';
  import Spinner from 'Components/Spinner.vue';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data: function() {
      return {ShowSpinner: true, Tariffs: [],
        Columns: [
          { name: 'TimeZone', title: 'Time zone' },
          { name: 'Name', title: 'Name' },
          { name: 'StartDate', title: 'Start date' },
          { name: 'EndDate', title: 'End date' },
          { name: '__slot:actions', title: '' }
        ]
      };
    },
    methods: {
      GetTariffs: function() {
        this.$http.post('getTariffs', { }).then(
          (result) => {
            if(result.status == 200) {
              for(var n in result.data) {
                result.data[n].StartDate = moment.tz(result.data[n].StartDate, result.data[n].TimeZone).format('DD/MM/YYYY');
                result.data[n].EndDate   = moment.tz(result.data[n].EndDate,   result.data[n].TimeZone).format('DD/MM/YYYY');
              }
              this.Tariffs = result.data;
            }
            this.ShowSpinner = false;
          }
        );
      },
      Edit: function(tariff, evt) {
        $(evt.target).blur();
        this.ShowSpinner = true;
        $.ajax({ url: "getTariff", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ TariffID: tariff.TariffID }),
          success: (result) => {
            this.$refs.EditDialog.Show(result);
          },
          complete: () => {
            this.ShowSpinner = false;
          }
        });
      },
      Add: function() {
        this.$refs.EditDialog.Show( {
          TimeZone: moment.tz.guess(),
          StartDate: moment().startOf('year').valueOf(),
          EndDate: moment().endOf('year').valueOf(),
          Name: 'No name',
          Template: {
            Description: 'Add tariff description here',
            Seasons: [],
            Periods: [ { ReactiveMode: 'NONE', PowerMode: 'NONE', Color: '#fffff0' } ],
            Holidays: [],
            WeekendPeriod: null,
            HolidayPeriod: null
          }
        });
      },
      Delete: function(tariff, evt) {
        if(confirm("Delete '" + tariff.Name + "' tariff?")){
          $(evt.target).blur();
          $.ajax({ url: "delTariff", timeout: 20000, type: "POST", dataType: "text", contentType: 'application/json', data: JSON.stringify({ TariffID: tariff.TariffID }),
            success: function(result) {
              this.GetTariffs();
            }.bind(this)
          });
        }
      }
    },
    mounted: function() {
      this.ShowSpinner = true;
      this.GetTariffs();
    },
    updated: function(){
        oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    components: { Spinner, Vuetable, TariffsEdit }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1) {
      width: 60px;
      text-align: center;
    }
    .table >>> td:nth-child(1) {
      text-align: center;
    }
    .table >>> th:nth-child(3), .table >>> th:nth-child(4)  {
      width: 150px;
      text-align: center;
    }
    .table >>> td:nth-child(3), .table >>> td:nth-child(4) {
      text-align: center;
    }
    .table >>> th:nth-child(5) {
      width: 100px;
    }
    .table >>> td:nth-child(5) {
      text-align: right;
    }
  }
</style>