<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <div class="huge header-text">Active Energy</div>
            <div class="header-buttons clearfix" ref="ToolbarActive">
              <toolbar-aggregation storage="AggregationEnergy" :options="['15M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <div class="btn-group">

              </div>
              <!--toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClickActive" v-on:MaximizeClick="MaximizeActiveClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools-->
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="ACTIVE_ENERGY" :analyzer="analyzer" :data15M="Data15M" :data1H="Data1H" :dataGraph="GraphData" :height.sync="ActiveHeight"></counters>
              <energy class="col-md-6 col-lg-8 col-xl-9" serie="ACTIVE_ENERGY" :data="GraphData" :height="ActiveHeight"></energy>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <div class="panel panel-reactive-energy">
          <div class="panel-heading">
            <div class="header-text">Fund. Reactive Energy</div>
            <div class="pull-right btn-group" ref="ToolbarReactive">

              <!--toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClickReactive" v-on:MaximizeClick="MaximizeReactiveClick" v-on:MinimizeClick="MinimizeClick" ref="reactiveTools"></toolbar-tools-->
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="REACTIVE_ENERGY" :analyzer="analyzer" :data15M="Data15M" :data1H="Data1H" :dataGraph="GraphData" :height.sync="ReactiveHeight"></counters>
              <energy class="col-md-6 col-lg-8 col-xl-9" serie="REACTIVE_ENERGY" :data="GraphData" :height="ReactiveHeight"></energy>
            </div>
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
  import Energy from './Energy.vue';
  import Counters from './Counters.vue';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { Aggreg: null, ShowSpinner: true, Data15M: null, Data1H: null, GraphData: null, NowTimer: null, GraphTimer: null, Range: null, ActiveHeight: 300, ReactiveHeight: 300 };
    },
    watch: {
      analyzer: function() { this.UpdateNowValues(); this.UpdateGraphValues(); },
      Aggreg: function() {  this.UpdateGraphValues(); },
      Range: function() { this.UpdateGraphValues(); }
    },
    computed: {
      ExportData: function() {
        return [];
      }
    },
    created: function() {
      this.UpdateNowValues();
      this.UpdateGraphValues();
    },
    beforeDestroy: function() {
      if(this.NowTimer != null) clearTimeout(this.NowTimer);
      if(this.GraphTimer != null) clearTimeout(this.GraphTimer);
    },
    methods: {
      UpdateNowValues: function() {
        if(this.NowTimer != null) clearTimeout(this.NowTimer);
        if(this.analyzer == null) return;
        this.analyzer.GetSeriesLast('1H', ['ACTIVE_POWER', 'REACTIVE_POWER'],
          (working) => {},
          (result) => { this.Data1H = result; },
          (timeout) => {
            this.analyzer.GetSeriesLast('15M', ['ACTIVE_POWER', 'REACTIVE_POWER'],
              (working) => {},
              (result) => { this.Data15M = result; },
              (timeout) => { this.NowTimer = setTimeout(() => { this.UpdateNowValues(); }, 10000); }
            );
          }
        );
      },
      UpdateGraphValues: function() {
        if(this.GraphTimer != null) clearTimeout(this.GraphTimer);
        if((this.Aggreg == null) || (this.Range == null) || (this.analyzer == null)) return;
        this.analyzer.GetPricesRange(this.Aggreg, this.Range, this.GraphData,
          (working) => { this.ShowSpinner = working; },
          (result) => { this.GraphData = Object.freeze(result); },
          (timeout) => { if(timeout != null) this.GraphTimer = setTimeout( () => { this.UpdateGraphValues(); }, timeout); }
        );

//        if(lastsample === 0) {
//          this.ActiveData = [];
//          this.ReactiveData = [];
//          this.ShowSpinner = true;
//        }
//        var Analyzer = this.$root.SelectedAnalyzer;
//        $.ajax({ url: 'getPrices', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Aggreg: aggreg, To: to, LastSample: lastsample, Analyzer: Analyzer.Analyzer, Period: period }),
//          success: function(result) {
//            if((result['Sampletime'] === null) || (result['Sampletime'].length === 0)) return;
//            if(result['Aggregation_Source'] === '15M') {
//              for(var n = 0; n < result['Active_Power'].length; n++) result['Active_Power'][n] = result['Active_Power'][n] / 4.0;
//              for(var n = 0; n < result['Reactive_Power'].length; n++) result['Reactive_Power'][n] = result['Reactive_Power'][n] / 4.0;
//            }
//            this.ActiveData = oZm.GraphDataAppend(result, 'Active_Power', this.ActiveData);
//            this.ReactiveData = oZm.GraphDataAppend(result, 'Reactive_Power', this.ReactiveData);
//            lastsample = oZm.LastSampletime(result, lastsample);
//            this.Bills = result["Bills"];
//            //Group periods
//            var LastItem = (this.Periods.length > 0) ? this.Periods.slice(-1)[0] : null;
//            for(var i = 0; i < result.Sampletime.length; i++) {
//              var Period = { Name: result.Period_Name[i], From: result.Sampletime[i] - ((this.Aggreg === '15M') ? 899999 : 3599999), To: result.Sampletime[i], Color: result.Period_Color[i] };
//              if(LastItem === null) {
//                this.Periods.push(Period);
//                LastItem = Period;
//              } else if(Period.From === (LastItem.To + 1)) {
//                if(Period.Name === LastItem.Name) {
//                  LastItem.To = Period.To;
//                  this.Periods.pop();
//                  this.Periods.push(LastItem);
//                } else {
//                  this.Periods.push(Period);
//                  LastItem = Period;
//                }
//              }
//              this.Periods = this.Periods.filter(period => (period.To > result.from) || (period.From < result.to) ).sort(function(a, b) { if(a.To < b.To) return -1; if(a.To > b.To) return 1; return 0; });
//            }
//            this.$refs.graphActive.SetOptions( { dateWindow: [result.From, result.To], file: this.ActiveData, valueRange: oZm.Series['ACTIVE_ENERGY'].Range(this.ActiveData, [true]) } );
//            this.$refs.graphReactive.SetOptions( { dateWindow: [result.From, result.To], file: this.ReactiveData, valueRange: oZm.Series['REACTIVE_ENERGY'].Range(this.ReactiveData, [true]) } );
//            this.$refs.activeCounters.SetStats([result.From, result.To], this.ActiveData, result["Aggregation_Source"]);
//            this.$refs.reactiveCounters.SetStats([result.From, result.To], this.ReactiveData, result["Aggregation_Source"]);
//          }.bind(this),
//          complete: function() {
//            this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
//            this.ShowSpinner = false;
//            this.$refs.period.UpdateCompleted(lastsample);
//          }.bind(this),
//          error: function(response) {
//            if(response.status === 400) this.$refs.aggregation.AggregationFail();
//            this.ShowSpinner = true;
//          }.bind(this)
//        });
      }
    },
    components: { Energy, Counters }
  }
</script>

<style scoped>
  .header-text {
    font-size: 20px;
    line-height: 24px;
  }
</style>