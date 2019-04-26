<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <span class="header-text">Energy costs</span>
            <div class="header-buttons clearfix">
              <toolbar-years @YearChanged="YearChanged" aggreg="1H"></toolbar-years>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <summary-graph ref="summary"></summary-graph>
            <bill v-for="bill in Bills" :bill="bill" :currency="Currency"></bill>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import ToolbarYears from 'Components/ToolbarYears.vue';
  import Spinner from 'Components/Spinner.vue';
  import SummaryGraph from './SummaryGraph.vue';
  import Bill from './Bill.vue';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data () {
      return { ShowSpinner: true, Bills: [], Currency: '€' };
    },
    methods: {
      YearChanged: function(year) {
        this.ShowSpinner = true;
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Year = moment().year(year);
        $.ajax({ url: "getPrices", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify( { Period: Year.endOf('year').valueOf() - Year.startOf('year').valueOf(), To: Year.endOf('year').valueOf(), Aggreg: '1H', LastSample: 0, Analyzer: Analyzer.ID } ),
          success: function(result) {
            this.Bills = result.Bills;
//            this.Currency = result.Currency;
            this.$refs.summary.SetData(result.Bills);
            this.ShowSpinner = false;
          }.bind(this)
        });
      }
    },
    components: { ToolbarYears, Spinner, SummaryGraph, Bill }
  }
</script>

<style>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
