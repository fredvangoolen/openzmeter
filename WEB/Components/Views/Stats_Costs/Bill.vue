<template>
  <div ref="panel" class="panel panel-other">
    <div class="panel-heading">{{ bill.Sampletime }}
      <div class="float-right">0.00€</div>
    </div>
    <div class="panel-body">
      <table v-for="(period, idx) in bill.Periods">
        <tr class="period-name"><td>P{{ idx + 1}}</td><td class="total"colspan="2">0.00€</td></tr>
        <tr><td>Energy consumed (430kWh x 0.10€)</td><td class="total">10.00€</td></tr>
        <tr><td>Reactive penalties</td><td class="total">0.00€</td></tr>
        <tr><td>Power contracted (430kWh x 0.10€ x 30 days)</td><td class="total">10.00€</td></tr>
        <tr><td>Excess penalties (10 times)</td><td class="total">10.00€</td></tr>
      </table>
      <table>
        <tr class="period-name"><td>Other concepts and taxes</td><td class="total">10.00€</td></tr>
        <tr v-for="tax in bill.Taxes"><td>{{ tax.Name }}</td><td class="total">10.00€</td></tr>
      </table>
    </div>
  </div>
</template>

<script>
  export default {
    props: ['bill'],
    data: function() {
      return { ShowSpinner: true, Bills: [] };
    },
    methods: {
      
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