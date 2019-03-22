<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Tariffs &raquo; Edit &raquo; Periods</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="row">
        <div class="col-xs-12">
          <table>
            <tr><td></td><td><strong>Contracted power <em>(kW)</em></strong></td><td><strong>Power costs <em>(€/kW/day)</em></strong></td><td><strong>Power costs <em>(€/kWh/day)</em></strong></td></tr>
            <tr v-for="(period, idx) in Tariff.Config.Periods">
                <td><strong>P{{idx + 1}}</strong></td>
                <td><div class="input-group input-group-xs"><input type="text" class="form-control" @change="ItemChanged($event, 'Contracted_Power', idx)" :value="period.Contracted_Power"></div></td>
                <td><div class="input-group input-group-xs"><input type="text" class="form-control" @change="ItemChanged($event, 'Power_Cost', idx)" :value="period.Power_Cost"></div></td>
                <td><div class="input-group input-group-xs"><input type="text" class="form-control" @change="ItemChanged($event, 'Energy_Cost', idx)" :value="period.Energy_Cost"></div></td>
              </tr>
          </table>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    data: function() {
      return { Show: false, Tariff: {} };
    },
    methods: {
      Load: function(tariff) {
        this.Tariff = tariff;
        this.Show = true;
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Tariff.Show = true;
      },
      ItemChanged(value, type, idx) {
        var Value = parseFloat(value.target.value);
        if(isNaN(Value) === true) Value = 0.0;
        this.Tariff.Config.Periods[idx][type] = Value;
        this.Tariff = Object.assign({}, this.Tariff);
      }
    }
  }
</script>

<style scoped>
  table, .input-group {
    width: 100%;
  }
  table tr td {
    padding: 1px 3px;
  }
  input, td {
    text-align: right;
  }
</style>