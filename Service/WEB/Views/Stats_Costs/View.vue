<template>
  <div v-if="analyzer != null">
    <z-row>
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <span class="header-text">Energy costs</span>
            <div class="header-buttons clearfix">
              <z-button-group>
                <option-picker v-model="Group" :options="[{name: 'Contracted', value: 0}, {name: 'Compare group 1', value: 1}, {name: 'Compare group 2', value: 2}, {name: 'Compare group 3', value: 3}, {name: 'Compare group 4', value: 4}, {name: 'Compare group 5', value: 5}, {name: 'Compare group 6', value: 6}, {name: 'Compare group 7', value: 7}, {name: 'Compare group 8', value: 8}, {name: 'Compare group 9', value: 9}, {name: 'Compare group 10', value: 10}]"></option-picker>
              </z-button-group>
              <toolbar-years :analyzer="analyzer" v-model="Year" aggreg="1H" storage="StatsYear"></toolbar-years>
              <z-button-group>
                <button-export file="Cost" :rows="Export"></button-export>
              </z-button-group>
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
          <bill :analyzer="analyzer" :bill="bill" :group="Group" v-if="BillNames[idx]==Active"></bill>
        </div>
      </div>
    </z-row>
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
      return { Year: null, Bills: null, Active: null, Group: 0 };
    },
    computed: {
      BillNames: function() {
        var Names = [];
        for(var n in this.Bills) Names.push(moment(this.Bills[n].To).tz(this.analyzer.TimeZone).format('DD-MMM'));
        return Names;
      }
    },
    watch: {
      analyzer: function() {
        this.GetYear();
      },
      Group: function() {
        this.GetYear();
      },
      Year: function(val) {
        this.GetYear();
      }
    },
    methods: {
      GetYear: function() {
        this.Bills = null;
        if((this.analyzer == null) || (this.Group == null) || (this.Year == null)) return;
        var To   = moment().tz(this.analyzer.TimeZone).year(this.Year).endOf('year').valueOf();
        var From = moment().tz(this.analyzer.TimeZone).year(this.Year).startOf('year').valueOf();
        this.Post('getBills', { From: From, To: To, Analyzer: this.analyzer.Analyzer, Group: this.Group }).then(
          (result) => { this.Bills = result; },
          (fail) => { }
        );
      },
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