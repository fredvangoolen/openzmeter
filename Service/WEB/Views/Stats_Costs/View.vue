<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <span class="header-text">Energy costs</span>
            <div class="header-buttons clearfix">
              <toolbar-years :analyzer="analyzer" v-model="Year" aggreg="1H" storage="StatsYear"></toolbar-years>
              <div class="btn-group">
                <button-export></button-export>
              </div>
              <toolbar-tools ref="tools" :options="['EXPORT']" v-on:ExportClick="ExportClick"></toolbar-tools>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <summary-graph :value="Bills"></summary-graph>
            <bill v-for="(bill, idx) in Bills" :bill="bill" :currency="Currency" ref="bills" :key="idx"></bill>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import SummaryGraph from './SummaryGraph.vue';
  import Bill from './Bill.vue';
  import moment from 'moment';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { Year: null, ShowSpinner: true, Bills: null, Currency: '€' };
    },
    watch: {
      Year: function(val) {
        this.ShowSpinner = true;
        this.Bills = null;
        var To = moment().year(val).endOf('year').valueOf();
        var Period = To - moment().year(val).startOf('year').valueOf();
        this.$http.post("getPrices", { Period: Period, To: To, Aggreg: '1H', LastSample: 0, Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.ShowSpinner = false;
            this.Bills = result.data.Bills.filter(function(value) { return value.Complete === true; });
          }
        );
      }
    },
    methods: {
      ExportClick: function() {
        var Data = [];
        for(var i = 0; i < this.$refs.bills.length; i++) {
          Data = this.$refs.bills[i].GetData(Data);
          Data.push([]);
          Data.push([]);
        }
        this.$refs.tools.SaveCVS('Cost.csv', Data);
      }
    },
    components: { SummaryGraph, Bill }
  }
</script>

<style scoped>
  .header-text {
    font-size: 20px;
    line-height: 24px;
  }
</style>
