<template>
  <q-page>
    <q-card>
      <q-toolbar class="bg-indigo-9 text-white q-pr-none">
        <q-toolbar-title>Energy costs</q-toolbar-title>
        <toolbar-select v-model="Group" name="Rate" :options="[{name: 'Contracted', value: 0}, {name: 'Group 1', value: 1}, {name: 'Group 2', value: 2}, {name: 'Group 3', value: 3}, {name: 'Group 4', value: 4}, {name: 'Group 5', value: 5}, {name: 'Group 6', value: 6}, {name: 'Group 7', value: 7}, {name: 'Group 8', value: 8}, {name: 'Group 9', value: 9}, {name: 'Group 10', value: 10}]" storage="Stats_Heatmap.Group" />
        <q-separator dark vertical />
        <toolbar-year storage="Stats_Heatmap.Year" v-model="Year" aggreg="15M" />
        <q-separator dark vertical />
        <toolbar-export file="Cost" :rows="Export" />
      </q-toolbar>
    </q-card>
    <div class="row">
      <div class="col-lg-12">
          <div class="panel-body">
            <spinner v-if="Bills==null"></spinner>
            <summary-graph :data="Bills"></summary-graph>
          </div>

      </div>
      <div class="col-lg-12" v-if="Bills!=null&&Bills.length==0">
        <div class="alert alert-warning text-center" style="margin-bottom: 5px; padding: 5px">No bills in selected year. No one generated yet or no rate is assigned for required period.</div>
      </div>
      <div class="col-lg-12" v-if="Bills!=null&&Bills.length>0">
        <detail-tabs :tabs="BillNames" v-model="Active"></detail-tabs>
        <div v-for="(bill, idx) in Bills" :key="idx">
          <bill :bill="bill" :group="Group" v-if="BillNames[idx]==Active"></bill>
        </div>
      </div>
    </div>
  </q-page>
</template>

<script>
  import SummaryGraph from './SummaryGraph.vue';
  import Bill from './Bill.vue';
//  import moment from 'moment';
  export default {
    data: function() {
      return { Year: null, Bills: null, Active: null, Group: 0 };
    },
    computed: {
      BillNames: function() {
        var Names = [];
        for(var n in this.Bills) Names.push(moment(this.Bills[n].To).tz(this.Analyzer.TimeZone).format('DD-MMM'));
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
        var To   = moment().tz(this.Analyzer.TimeZone).year(this.Year).endOf('year').valueOf();
        var From = moment().tz(this.Analyzer.TimeZone).year(this.Year).startOf('year').valueOf();
        this.Post('getBills', { From: From, To: To, Analyzer: this.Analyzer.Analyzer, Group: this.Group }).then(
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