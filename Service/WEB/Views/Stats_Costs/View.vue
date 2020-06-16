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
          </div>
        </div>
      </div>
      <div class="col-lg-12" v-if="Bills!=null&&Bills.length==0">
        <div class="alert alert-warning text-center" style="margin-bottom: 5px; padding: 5px">No bills in selected year. No one generated yet or no rate is assigned for required period.</div>
      </div>
      <div class="col-lg-12" v-if="Bills!=null&&Bills.length>0">
        <detail-tabs :tabs="BillNames" v-model="Active"></detail-tabs>
        <div v-for="(bill, idx) in Bills" :key="idx">
          <bill :analyzer="analyzer" :bill="bill" v-if="BillNames[idx]==Active"></bill>
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
      return { Year: null, Bills: null, Active: null };
    },
    computed: {
      BillNames: function() {
        var Names = [];
        for(var n in this.Bills) {
          var Date = moment(this.Bills[n].To).tz(this.analyzer.TimeZone);
          Names.push(Date.format('DD-MMM'));
        }
        return Names;
      }
    },
    watch: {
      Year: function(val) {
        this.Bills = null;
        var To   = moment().tz(this.analyzer.TimeZone).year(val).endOf('year').valueOf();
        var From = moment().tz(this.analyzer.TimeZone).year(val).startOf('year').valueOf();
        this.$http.post('getPrices', { From: From, To: To, Analyzer: this.analyzer.Analyzer, Aggreg: '15M', Precise: true, Series: ['BILLS', 'REACTIVE_POWER', 'ACTIVE_POWER', 'PERIOD_NUMBER', 'PERIOD_POWER_COST', 'PERIOD_ENERGY_COST', 'PERIOD_COLOR', 'PERIOD_POWER_CONTRACTED'] }).then(
          (result) => {
            this.Bills = result.data.Bills;
            var Sample = 0;
            for(var Bill = 0; Bill < this.Bills.length; Bill++) {
              this.Bills[Bill].Sampletime = [];
              this.Bills[Bill].Active_Power = [];
              this.Bills[Bill].Reactive_Power = [];
              this.Bills[Bill].Period_Color = [];
              this.Bills[Bill].Period_Energy_Cost = [];
              this.Bills[Bill].Period_Number = [];
              this.Bills[Bill].Period_Power_Contracted = [];
              this.Bills[Bill].Period_Power_Cost = [];
              for(; Sample < result.data.Sampletime.length; Sample++) {
                if(result.data.Sampletime[Sample] < result.data.Bills[Bill].From) continue;
                if(result.data.Sampletime[Sample] > result.data.Bills[Bill].To) break;
                this.Bills[Bill].Sampletime.push(result.data.Sampletime[Sample]);
                this.Bills[Bill].Active_Power.push(result.data.Active_Power[Sample]);
                this.Bills[Bill].Reactive_Power.push(result.data.Reactive_Power[Sample]);
                this.Bills[Bill].Period_Color.push(result.data.Period_Color[Sample]);
                this.Bills[Bill].Period_Energy_Cost.push(result.data.Period_Energy_Cost[Sample]);
                this.Bills[Bill].Period_Number.push(result.data.Period_Number[Sample]);
                this.Bills[Bill].Period_Power_Contracted.push(result.data.Period_Power_Contracted[Sample]);
                this.Bills[Bill].Period_Power_Cost.push(result.data.Period_Power_Cost[Sample]);
              }
            }
          },
          (fail) => {}
        );
      }
    },
    methods: {
      Export: function() {
        var Data = [];
//        for(var i = 0; i < this.$refs.bills.length; i++) {
//          Data = this.$refs.bills[i].GetData(Data);
//          Data.push([]);
//          Data.push([]);
//        }
//        this.$refs.tools.SaveCVS('Cost.csv', Data);
        return Object.seal(Data);
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
