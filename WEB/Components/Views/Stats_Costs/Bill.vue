<template>
  <div ref="panel" class="panel panel-other">
    <div class="panel-heading">{{ BillMonth(bill.To) }} <span style="font-size:12px">({{ BillRange(bill.From, bill.To) }})</span>
      <div class="float-right" v-html="TotalBill(bill)"></div>
    </div>
    <div class="panel-body">
      <table>
        <tr class="period-name">
          <td colspan="2">Periods</td>
        </tr>
        <tr v-for="(period, idx) in bill.Periods">
          <td class="padding-table">
            <table>
              <tr class="period-name2">
                <td>P{{ idx + 1}}</td>
                <td class="total" colspan="2">{{ Round2(TotalPeriod(period)) }} {{ currency }}</td>
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
          </td>
        </tr>
      </table>
      <table v-if="bill.Taxes.length > 0">
        <tr class="period-name">
          <td>Other concepts and taxes</td>
          <td class="total">{{ Round2(TotalTaxes()) }} {{ currency }}</td>
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
      return { ShowSpinner: true };
    },
    methods: {
      Round2: oZm.Round2Plain,
      Round6: oZm.Round6Plain,
      GetData: function(rows) {
        rows.push([this.BillMonth(this.bill.To), this.BillRange(this.bill.From, this.bill.To), this.bill.Total]);
        rows.push(['Periods']);
        for(var p = 0; p < this.bill.Periods.length; p++) {
          rows.push(['P' + (p + 1), '', this.TotalPeriod(this.bill.Periods[p])]);
          rows.push(['', 'Energy consumed (' + this.Round2(this.bill.Periods[p].Energy.Consumed / 1000.0) + 'kWh x ' + this.Round6(this.bill.Periods[p].Energy.Cost * 1000) + ')', this.bill.Periods[p].Energy.Consumed * this.bill.Periods[p].Energy.Cost]);
          if(this.bill.Periods[p].Energy.Other != '') rows.push(['', this.bill.Periods[p].Energy.Other, this.bill.Periods[p].Energy.OtherCost]);
          rows.push(['', 'Power contracted (' + this.Round2(this.bill.Periods[p].Power.Contracted / 1000.0) + 'kW x ' + this.Round6(this.bill.Periods[p].Power.Cost * 1000) + ' x ' + this.bill.Periods[p].Power.Days + 'days)', this.bill.Periods[p].Power.Contracted * this.bill.Periods[p].Power.Cost * this.bill.Periods[p].Power.Days]);
          if(this.bill.Periods[p].Power.Other != '') rows.push(['', this.bill.Periods[p].Power.Other, this.bill.Periods[p].Power.OtherCost]);
        }
        rows.push(['Other concepts and taxes', '', this.TotalTaxes()]);
        for(var t = 0; t < this.bill.Taxes.length; t++) rows.push(['', this.bill.Taxes[t].Name, this.bill.Taxes[t].Total]);
        return rows;
      },
      TotalBill: function(bill) {
        return oZm.Round2(bill.Total) + '<em> ' + this.currency + '</em>';
      },
      TotalPeriod: function(period) {
        return period.Energy.Cost * period.Energy.Consumed + period.Energy.OtherCosts + period.Power.Cost * period.Power.Contracted * period.Power.Days + period.Power.OtherCosts;
      },
      TotalTaxes: function() {
        var Total = 0.0;
        for(var i = 0; i < this.bill.Taxes.length; i++) Total += this.bill.Taxes[i].Total;
        return Total;
      },
      BillMonth: function(val) {
        var MonthsNames = moment.months();
        return MonthsNames[moment(val).month()];
      },
      BillRange: function(from, to) {
        return moment(from).format('DD/MM/YYYY') + ' - ' + moment(to).format('DD/MM/YYYY');
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
  .period-name2 {
    font-weight: bold;
    font-size: 12px;
    border-bottom: 1px solid #ddd;
  }
  .padding-table {
    padding-left: 20px;
    font-size: 12px;
  }
  .total {
    text-align: right;
  }
</style>