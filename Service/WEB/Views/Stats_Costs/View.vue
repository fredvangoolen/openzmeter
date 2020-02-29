<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <span class="header-text">Energy costs</span>
            <div class="header-buttons clearfix">
              <toolbar-years :analyzer="analyzer" v-model="Year" aggreg="1H" storage="StatsYear"></toolbar-years>
              <div class="btn-group">
                <button-export file="Cost" :rows="Export"></button-export>
              </div>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="Bills==null"></spinner>
            <summary-graph :data="Bills" :analyzer="analyzer"></summary-graph>
            <div class="alert alert-warning text-center" style="margin-bottom: 5px; padding: 5px" v-if="Bills!=null&&Bills.length==0">No bills in selected year. No one generated yet or no rate is assigned for required period.</div>
            <bill v-for="(bill, idx) in Bills" :bill="bill" :currency="Currency" :key="idx"></bill>
          </div>
        </div>
      </div>
    </div>
  </div>
  <div v-else-if="analyzer == null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>
  </div>
</template>

<script>
  import SummaryGraph from './SummaryGraph.vue';
  import Bill from './Bill.vue';
  import moment from 'moment';
  export default {
    props: { analyzer: Object },
    data: function() {
      return { Year: null, Bills: null, Currency: '€' };
    },
    watch: {
      Year: function(val) {
        this.Bills = null;
        var To   = moment().tz(this.analyzer.TimeZone).year(val).endOf('year').valueOf();
        var From = moment().tz(this.analyzer.TimeZone).year(val).startOf('year').valueOf();
        this.$http.post('getBills', { From: From, To: To, Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.Bills = result.data.filter(function(value) { return value.Complete === true; });
          },
          (fail) => {}
        );
      }
    },
    computed: {
      Export: function() {
        var Data = [];
//        for(var i = 0; i < this.$refs.bills.length; i++) {
//          Data = this.$refs.bills[i].GetData(Data);
//          Data.push([]);
//          Data.push([]);
//        }
//        this.$refs.tools.SaveCVS('Cost.csv', Data);
        return Data;
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
