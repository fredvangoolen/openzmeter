<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <div class="huge header-text">Active Energy</div>
            <portal to="fs-toolbar" :disabled="!ExpandedActive" class="header-buttons">
              <toolbar-aggregation storage="AggregationEnergy" :options="['15M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <div class="btn-group">
                <button-export file="Energy" :rows="Export"></button-export>
                <button-expand v-model="ExpandedActive"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="ACTIVE_ENERGY" :analyzer="analyzer" :data15M="Data15M" :data1H="Data1H" :dataGraph="DataGraph" :height.sync="ActiveHeight"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedActive" class="panel-body">
                  <energy serie="Active_Energy" :analyzer="analyzer" :data="DataGraph" :height="ExpandedActive?DialogHeight:ActiveHeight"></energy>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <div class="panel panel-reactive-energy">
          <div class="panel-heading">
            <div class="header-text">Fund. Reactive Energy</div>
            <portal to="fs-toolbar" :disabled="!ExpandedReactive" class="header-buttons">
              <div class="btn-group">
                <button-export file="Energy" :rows="Export"></button-export>
                <button-expand v-model="ExpandedReactive"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="REACTIVE_ENERGY" :analyzer="analyzer" :data15M="Data15M" :data1H="Data1H" :dataGraph="DataGraph" :height.sync="ReactiveHeight"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedReactive" class="panel-body">
                  <energy serie="Reactive_Energy" :analyzer="analyzer" :data="DataGraph" :height="ExpandedReactive?DialogHeight:ReactiveHeight"></energy>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
    <dlg-fs v-if="ExpandedActive" title="Active Energy" :height.sync="DialogHeight"></dlg-fs>
    <dlg-fs v-if="ExpandedReactive" title="Reactive Energy" :height.sync="DialogHeight"></dlg-fs>
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
      return { Aggreg: null, ShowSpinner: true, Range: null, ActiveHeight: 300, ReactiveHeight: 300, ExpandedActive: false, ExpandedReactive: false, DialogHeight: 300,
               Data15M: null, Control15M: null, Data1H: null, Control1H: null, DataGraph: null, ControlGraph: null };
    },
    watch: {
      analyzer: function() { this.Update15MValues(true); this.Update1HValues(true); this.UpdateGraphValues(true); },
      Aggreg: function() { this.UpdateGraphValues(true); },
      Range: function() { this.UpdateGraphValues(false); }
    },
    computed: {
      Export: function() {
        var Data = [['Time', 'Active energy', 'Reactive energy']];
        if(this.DataGraph != null) {
          var Div = (this.DataGraph.Aggregation_Source == '1H') ? 1 : 4;
          for(var n in this.DataGraph.Sampletime) Data.push([this.DataGraph.Sampletime[n], this.DataGraph.Active_Power[n] / Div, this.DataGraph.Reactive_Power[n] / Div]);
        }
        return Data;
      }
    },
    created: function() {
      this.Update15MValues(true);
      this.Update1HValues(true);
      this.UpdateGraphValues(true);
    },
    beforeDestroy: function() {
      this.CancelTransfer(this.Control15M);
      this.CancelTransfer(this.Control1H);
      this.CancelTransfer(this.ControlGraph);
    },
    methods: {
      Update15MValues: function(clear) {
        if(clear == true) this.Data15M = null;
        this.Control15M = this.CancelTransfer(this.Control15M);
        if(this.analyzer == null) return;
        this.Control15M.CancelFunc = this.analyzer.GetSeriesLast('15M', ['ACTIVE_POWER', 'REACTIVE_POWER'],
          (working) => {},
          (result) => { this.Data15M = Object.seal(result); },
          (timeout) => { this.Control15M.Timer = setTimeout(() => { this.Update15MValues(false); }, timeout); }
        );
      },
      Update1HValues: function(clear) {
        if(clear == true) this.DataH = null;
        this.Control1H = this.CancelTransfer(this.Control1H);
        if(this.analyzer == null) return;
        this.Control1H.CancelFunc = this.analyzer.GetSeriesLast('1H', ['ACTIVE_POWER', 'REACTIVE_POWER'],
          (working) => {},
          (result) => { this.Data1H = Object.seal(result); },
          (timeout) => { this.Control1H.Timer = setTimeout(() => { this.Update1HValues(false); }, timeout); }
        );
      },
      UpdateGraphValues: function(clear) {
        if(clear == true) this.DataGraph = null;
        this.ControlGraph = this.CancelTransfer(this.ControlGraph);
        if((this.Aggreg == null) || (this.Range == null) || (this.analyzer == null)) return;
        this.ControlGraph.CancelFunc = this.analyzer.GetPricesRange(this.Aggreg, this.Range, this.DataGraph,
          (working) => { this.ShowSpinner = working; },
          (result) => { this.DataGraph = Object.seal(result); },
          (timeout) => { this.ControlGraph.Timer = setTimeout( () => { this.UpdateGraphValues(false); }, timeout); }
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
//          }.bind(this),
//          complete: function() {
//            this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
//            this.ShowSpinner = false;
//          }.bind(this),
//          error: function(response) {
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