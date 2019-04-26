<template>
  <div ref="panel" class="panel panel-other">
    <div class="panel-heading">{{ BillMonth(bill.Sampletime) }}
      <div class="float-right" v-html="TotalBill(bill)"></div>
    </div>
    <div class="panel-body">
      <table v-for="(period, idx) in bill.Periods">
        <tr class="period-name">
          <td>P{{ idx + 1}}</td>
          <td class="total" colspan="2" v-html="TotalPeriod(period)"></td>
        </tr>
        <tr>
          <td>Energy consumed ({{ Round2(period.Energy.Consumed / 1000.0) }}kWh x {{ Round6(period.Energy.Cost * 1000) }} {{ currency }})</td>
          <td class="total">{{ Round2(period.Energy.Consumed * period.Energy.Cost)}} {{ currency }}</td>
        </tr>
        <tr v-if="period.Energy.Other != ''">
          <td>{{ period.Energy.Other }}</td>
          <td class="total">{{ Round2(period.Energy.OtherCost) }} {{ currency }}</td>
        </tr>
        <tr>
          <td>Power contracted ({{ Round2(period.Power.Contracted / 1000.0) }}kW x {{ Round6(period.Power.Cost * 1000) }} {{ currency }} x {{ period.Power.Days }} days)</td>
          <td class="total">{{ Round2(period.Power.Contracted * period.Power.Cost * period.Power.Days) }} {{ currency }}</td>
        </tr>
        <tr v-if="period.Power.Other != ''">
          <td>{{ period.Power.Other }}</td>
          <td class="total">{{ Round2(period.Power.OtherCost) }} {{ currency }}</td>
        </tr>
      </table>
      <table v-if="bill.Taxes.length > 0">
        <tr class="period-name">
          <td>Other concepts and taxes</td>
          <td class="total" v-html="TotalTaxes()"></td>
        </tr>
        <tr v-for="tax in bill.Taxes">
          <td>{{ tax.Name }}</td>
          <td class="total">{{ Round2(tax.Total) }} {{ currency }}</td></tr>
      </table>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'JS/moment.js';
  export default {
    props: ['bill', 'currency'],
    data: function() {
      return { ShowSpinner: true, Bills: [] };
    },
    methods: {
      Round2: oZm.Round2Plain,
      Round6: oZm.Round6Plain,
      TotalBill: function(bill) {
        return oZm.Round2(bill.Total) + '<em> ' + this.currency + '</em>' ;
      },
      TotalPeriod: function(period) {
        return oZm.Round2(period.Energy.Cost * period.Energy.Consumed + period.Energy.OtherCosts + period.Power.Cost * period.Power.Contracted * period.Power.Days + period.Power.OtherCosts) + '<em> ' + this.currency + '</em>' ;
      },
      TotalTaxes: function() {
        var Total = 0.0;
        for(var i = 0; i < this.bill.Taxes.length; i++) Total += this.bill.Taxes[i].Total;
        return oZm.Round2(Total) + '<em> ' + this.currency + '</em>';
      },
      BillMonth: function(val) {
        var MonthsNames = moment.months();
        return MonthsNames[moment(val).month()];
      }
    }
  }
</script>

<style scoped>
  table {
    width: 100%;
  }
  .panel-heading {
    font-weight: bold;
    font-size: 16px;      
  }
  .period-name {
    font-weight: bold;
    font-size: 14px;
    border-bottom: 1px solid #bbb;
  }
  .total {
    text-align: right;
  }
</style>