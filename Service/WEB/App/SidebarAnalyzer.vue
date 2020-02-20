<template>
  <div class="this" v-bind:class="{ expanded: expanded }">
    <img class="icon" :src="require('Imgs/disconnect32.png')">
    <div class="btn-group">
      <button type="button" class="btn btn-default btn-xs" v-on:click="ShowEvents" data-toggle="tooltip" :title="(EventsCount == 0) ? 'No events since last login' : EventsCount + ' events since last login'"><img v-bind:class="{blink_img: (EventsCount != 0)}" :src="require('Imgs/exclamation.png')"></button>
      <button type="button" class="btn btn-default btn-xs" @click="ShowDialog=true" data-toggle="tooltip" title="Select and configure avaliable analyzers"><img :src="require('Imgs/magnifier.png')"></button>
    </div>
    <div class="name">{{ (analyzer == null) ? 'None selected' : analyzer.Name }}</div>
    <analyzer-table v-if="ShowDialog" :analyzer="analyzer" @update:analyzer="SetAnalyzer($event)" @close="ShowDialog=false"></analyzer-table>
  </div>
</template>

<script>
  import AnalyzerTable from './Analyzer/Table.vue';
  import moment from 'moment';
  import 'moment-timezone';
  export default {
    props: { analyzer: Object, expanded: Boolean },
    data() {
      return { ShowDialog: false, EventsCount: 0 };
    },
    created: function() {
      this.LastSample = 0;
      this.Timer = null;
      if(localStorage.getItem("Analyzer") == null) {
        this.SetAnalyzer(null);
        this.View = 'Table';
      } else {
        this.$http.post('getAnalyzer', { Analyzer: localStorage.getItem("Analyzer") }).then(
          (result) => {
            result.data.Analyzer = localStorage.getItem("Analyzer");
            this.SetAnalyzer(result.data);
          },
          (fail) => {
            this.SetAnalyzer(null);
            this.ShowDialog = true;
          }
        );
      }
    },
    beforeDestroy: function () {
      clearInterval(this.Timer);
    },
    methods: {
      SetAnalyzer: function(analyzer) {
        if(analyzer != null) {
          analyzer.PhaseNames = ['R', 'S', 'T', 'N'];
          analyzer.VoltageChannels = (analyzer.Mode == '1PHASE') ? 1 : 3;
          analyzer.CurrentChannels = (analyzer.Mode == '1PHASE') ? 1 : (analyzer.Mode == '3PHASE') ? 3 : 4;
          analyzer.Color = { Voltage: '#d9534f', Current: '#428bca', Active_Power: '#f0ad4e', Active_Energy: '#3d3d8b', Frequency: '#5cb85c' };
          analyzer.Unit = { Voltage: 'V', Current: 'A', Active_Power: 'W', Active_Energy: 'Wh', Frequency: 'Hz'};
          analyzer.HarmonicsNames = ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th', '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th', '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th', '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th'];
          analyzer.GetSeriesNames = this.GetSeriesNames.bind(this);
          analyzer.GetSeriesRange = this.GetSeriesRange.bind(this);
          analyzer.GetPricesRange = this.GetPricesRange.bind(this);
          analyzer.GetSeriesLast = this.GetSeriesLast.bind(this);
          analyzer.GetSeriesNow = this.GetSeriesNow.bind(this);
          analyzer.FormatAggregation = this.FormatAggregation.bind(this);
          analyzer.FormatTime = this.FormatTime.bind(this);
          analyzer.FormatTimeMS = this.FormatTimeMS.bind(this);
          analyzer.FormatDateTime = this.FormatDateTime.bind(this);
        }
        this.$emit('update:analyzer', analyzer);
//        this.$parent.$parent.Analyzer = result;
//              this.$root.SelectedAnalyzer = result;
//              this.$root.$emit('AnalyzerChanged', result);


//        this.SelAnalyzer = analyzer;
//        this.LastSample = 0;
//        this.EventsCount = 0;
//        this.GetEvents();
      },
      GetSeriesNames: function(serie, len) {
        if(len == 1) return [serie];
        var Result = [];
        for(var n = 0; n < len; n++) Result.push(serie + ' (' + this.analyzer.PhaseNames[n] + ')');
        return Result;
      },
      GetSeriesRange: function(aggreg, range, prev, series, working, outs, timeout) {
        var Cancelled = false;
        var Timer = setTimeout(() => { if(working != null) working(true); }, 500);
        var ReqRange = null;
        var Now = moment().tz(this.analyzer.TimeZone);
        var Period = (aggreg == '3S') ? 3000 : (aggreg == '1M') ? 60000 : (aggreg == '10M') ? 600000 : (aggreg == '15M') ? 900000 : 3600000;
        if(range == 'LastDay') ReqRange = [Now.valueOf() - 86400000, Now.valueOf()];
        if(range == 'LastWeek') ReqRange = [Now.valueOf() - 604800000, Now.valueOf()];
        if(range == 'LastMonth') ReqRange = [Now.valueOf() - 2592000000, Now.valueOf()];
        if(range == 'LastYear') ReqRange = [Now.valueOf() - 31536000000, Now.valueOf()];
        if(range == 'Day') ReqRange = [Now.startOf('day').valueOf(), Now.endOf('day').valueOf()];
        if(range == 'Week') ReqRange = [Now.startOf('week').valueOf(), Now.endOf('week').valueOf()];
        if(range == 'Month') ReqRange = [Now.startOf('month').valueOf(), Now.endOf('month').valueOf()];
        if(range == 'Year') ReqRange = [Now.startOf('year').valueOf(), Now.endOf('year').valueOf()];
        if(Array.isArray(range)) ReqRange = [range[0], range[1]];
        var Out = {};
        if(prev != null) Out.Aggregation_Source = prev.Aggregation_Source;
        if(Out.Aggregation_Source != aggreg) {
          prev = null;
          Out = { Aggregation_Source: aggreg };
        } else if((prev != null) && (prev.Sampletime != undefined)) {
          for(var serie in prev) {
            if(Array.isArray(prev[serie]) == false) continue;
            if(Out[serie] == undefined) Out[serie] = [];
            for(var pos in prev.Sampletime) {
              if((prev.Sampletime[pos] >= ReqRange[0]) && (prev.Sampletime[pos] <= ReqRange[1])) Out[serie].push(prev[serie][pos]);
            }
          }
          Out.From = prev.From;
          Out.To = prev.To;
        }
        var Promises = [];
        if((prev == null) || (prev.From == undefined) || (ReqRange[0] < prev.From)) {
          var ToTime = ((prev == null) || (prev.From == undefined)) ? ReqRange[1] : prev.From - 1;
          Promises.push(this.$http.post('getSeries', { LastSample: 0, Analyzer: this.analyzer.Analyzer, Aggreg: aggreg, To: ToTime, From: ReqRange[0], Series: series }));
        }
        if((prev != null) && (prev.To != undefined) && (ReqRange[1] > prev.To)) {
          Promises.push(this.$http.post('getSeries', { LastSample: 0, Analyzer: this.analyzer.Analyzer, Aggreg: aggreg, To: ReqRange[1], From: prev.To - 1, Series: series }));
        }
        if(Promises.length == 0) {
          Out.To = ReqRange[1]
          Out.From = ReqRange[0];
          if(outs != null) outs(Out);
          clearTimeout(Timer);
          if(working != null) working(false);
          if(timeout != null) timeout(Period / 2);
        }
        Promise.all(Promises).then(
          (result) => {
            clearTimeout(Timer);
            if(Cancelled == true) return;
            for(var req in result) {
              if(result[req].data.From == ReqRange[0]) {
                for(var serie in result[req].data) {
                  if(Array.isArray(result[req].data[serie]) == false) continue;
                  Out[serie] = ((prev == null) || (prev[serie] == undefined)) ? result[req].data[serie] : result[req].data[serie].concat(Out[serie]);
                }
              } else {
                for(var serie in result[req].data) {
                  if(Array.isArray(result[req].data[serie]) == false) continue;
                  Out[serie] = ((prev == null) || (prev[serie] == undefined)) ? result[req].data[serie] : Out[serie].concat(result[req].data[serie]);
                }
              }
            }
            Out.To = ReqRange[1]
            Out.From = ReqRange[0];
            if(outs != null) outs(Out);
            if(working != null) working(false);
            if(timeout != null) timeout(Period / 2);
          },
          (fail) => {
            if(Cancelled == true) return;
            if(timeout != null) timeout(Period / 2);
          }
        );
        return () => { this.Cancelled = true; };
      },
      GetPricesRange: function(aggreg, range, prev, working, outs, timeout) {
        var Cancelled = false;
        var Timer = setTimeout(() => { if(working != null) working(true); }, 500);
        var ReqRange = null;
        var Now = moment().tz(this.analyzer.TimeZone);
        if(range == 'LastDay') ReqRange = [Now.valueOf() - 86400000, Now.valueOf()];
        if(range == 'LastWeek') ReqRange = [Now.valueOf() - 604800000, Now.valueOf()];
        if(range == 'LastMonth') ReqRange = [Now.valueOf() - 2592000000, Now.valueOf()];
        if(range == 'LastYear') ReqRange = [Now.valueOf() - 31536000000, Now.valueOf()];
        if(range == 'Day') ReqRange = [Now.startOf('day').valueOf(), Now.endOf('day').valueOf()];
        if(range == 'Week') ReqRange = [Now.startOf('week').valueOf(), Now.endOf('week').valueOf()];
        if(range == 'Month') ReqRange = [Now.startOf('month').valueOf(), Now.endOf('month').valueOf()];
        if(range == 'Year') ReqRange = [Now.startOf('year').valueOf(), Now.endOf('year').valueOf()];
        if(Array.isArray(range)) ReqRange = [range[0], range[1]];
        var Out = {};
        if(prev != null) Out.Aggregation_Source = prev.Aggregation_Source;
        if(Out.Aggregation_Source != aggreg) {
          prev = null;
          Out = { Aggregation_Source: aggreg };
        } else if((prev != null) && (prev.Sampletime != undefined)) {
          for(var serie in prev) {
            if(Array.isArray(prev[serie]) == false) continue;
            if(Out[serie] == undefined) Out[serie] = [];
            for(var pos in prev.Sampletime) {
              if((prev.Sampletime[pos] >= ReqRange[0]) && (prev.Sampletime[pos] <= ReqRange[1])) Out[serie].push(prev[serie][pos]);
            }
          }
          Out.From = prev.From;
          Out.To = prev.To;
        }
        var Promises = [];
        if((prev == null) || (prev.From == undefined) || (ReqRange[0] < prev.From)) {
          var ToTime = ((prev == null) || (prev.From == undefined)) ? ReqRange[1] : prev.From - 1;
          Promises.push(this.$http.post('getPrices', { LastSample: 0, Analyzer: this.analyzer.Analyzer, Aggreg: aggreg, To: ToTime, From: ReqRange[0] }));
        }
        if((prev != null) && (prev.To != undefined) && (ReqRange[1] > prev.To)) {
          Promises.push(this.$http.post('getPrices', { LastSample: 0, Analyzer: this.analyzer.Analyzer, Aggreg: aggreg, To: ReqRange[1], From: prev.To - 1 }));
        }
        if(Promises.length == 0) {
          Out.To = ReqRange[1]
          Out.From = ReqRange[0];
          clearTimeout(Timer);
          if(outs != null) outs(Out);
          if(working != null) working(false);
          if(timeout != null) timeout((aggreg == '15M') ? 900000 : 3600000);
        }
        Promise.all(Promises).then(
          (result) => {
            clearTimeout(Timer);
            if(Cancelled == true) return;
            for(var req in result) {
              if(result[req].data.From == ReqRange[0]) {
                for(var serie in result[req].data) {
                  if(Array.isArray(result[req].data[serie]) == false) continue;
                  Out[serie] = ((prev == null) || (prev[serie] == undefined)) ? result[req].data[serie] : result[req].data[serie].concat(prev[serie]);
                }
              } else {
                for(var serie in result[req].data) {
                  if(Array.isArray(result[req].data[serie]) == false) continue;
                  Out[serie] = ((prev == null) || (prev[serie] == undefined)) ? result[req].data[serie] : prev[serie].concat(result[req].data[serie]);
                }
              }
            }
//            Out.To = ReqRange[1];
//            Out.From = ReqRange[0];
            if(outs != null) outs(Out);
            if(working != null) working(false);
            if(timeout != null) timeout((aggreg == '15M') ? 900000 : 3600000);
          },
          (fail) => {
            if(Cancelled == true) return;
            if(outs != null) outs(Out);
            if(working != null) working(false);
            if(timeout != null) timeout((aggreg == '15M') ? 900000 : 3600000);
          }
        );
        return () => { Cancelled = true };
      },
      GetSeriesLast: function(aggreg, series, working, outs, timeout) {
        var Cancelled = false;
        var Timer = setTimeout(() => { if(working != null) working(true); }, 500);
        var Period = (aggreg == '3S') ? 6000 : (aggreg == '1M') ? 120000 : (aggreg == '10M') ? 1200000 : (aggreg == '15M') ? 1800000 : 7200000;
        this.$http.post('getSeries', { Series: series, To: moment().valueOf(), Aggreg: aggreg, From: moment().valueOf() - Period, Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            clearTimeout(Timer);
            if(Cancelled == true) return;
            if(result.data.Sampletime.length > 0) {
              var Result = {};
              Result.Aggregation_Source = result.data.Aggregation_Source;
              for(var n in result.data) {
                if(Array.isArray(result.data[n])) Result[n] = result.data[n].slice(-1)[0];
              }
              if(outs != null) outs(Result);
            } else {
              if(outs != null) outs(null);
            }
            if(working != null) working(false);
            if(timeout != null) timeout(Period / 2);
          },
          () => {
            if(outs != null) outs(null);
            if(timeout != null) timeout(Period / 2);
          }
        );
        return () => { Cancelled = true };
      },
      GetSeriesNow: function(series, working, outs, timeout) {
        var Cancelled = false;
        var Timer = setTimeout(() => { if(working != null) working(true); }, 500);
        this.$http.post('getSeriesNow', { Series: series, Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            clearTimeout(Timer);
            if(Cancelled == true) return;
            if(outs != null) outs((result.status == 200) ? result.data : null);
            if(working != null) working(false);
            if(timeout != null) timeout(1000);
          },
          () => {
            if(Cancelled == true) return;
            if(outs != null) outs(null);
            if(timeout != null) timeout(1000);
          }
        );
        return () => { Cancelled = true };
      },
      FormatAggregation: function(val) {
        return (val == '1H') ? '1 hour' : (val == '15M') ? '15 minutes' : (val == '10M') ? '10 minutes' : (val == '1M') ? '1 minute' : (val == '3S') ? '3 seconds' : 'unknown';
      },
      FormatTime: function(val) {
        return moment(val).tz(this.analyzer.TimeZone).format('HH:mm:ss');
      },
      FormatTimeMS: function(val) {
        return moment(val).tz(this.analyzer.TimeZone).format('HH:mm:ss.SSS');
      },
      FormatDateTime: function(val, spacer) {
        if(spacer == null) spacer = ' - ';
        return moment(val).tz(this.analyzer.TimeZone).format('hh:mm:ss') + spacer + moment(val).tz(this.analyzer.TimeZone).format('DD/MM/YYYY');
      },








      ShowEvents: function() {
        localStorage.setItem('Period', JSON.stringify({Period: moment().valueOf() - parseInt(localStorage.getItem('LastLogin')), To: moment().valueOf() }));
        this.$router.push({path: '/Events/Disturbances'});
      },
      GetEvents: function() {
        clearInterval(this.Timer);
        var Analyzer = this.$root.SelectedAnalyzer;
        if(Analyzer === null) return;
        $.ajax({ url: 'getEvents', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ To: moment().valueOf(), LastSample: this.LastSample, Analyzer: Analyzer.Analyzer, Period: moment().valueOf() - parseInt(localStorage.getItem('LastLogin')) }),
          success: function(result) {
            if(result.StartTime.length > 0) {
              this.EventsCount += result.StartTime.length;
              this.LastSample = result.StartTime.slice(-1)[0];
            }
          }.bind(this),
          complete: function() {
            this.Timer = setTimeout(this.GetEvents.bind(this), 60000);
          }.bind(this)
        });
      }
    },
    components: { AnalyzerTable }
  }
</script>

<style scoped>
  .this {
    background-color: #474544;
    color: #FAFAFA;
    width: 304px;
    height: 33px;
    display: flex;
    margin-left: 0;
    transition: margin-left 0.2s ease-in-out;
    border-bottom: 1px solid #333;
    border-top: 1px solid #333;
  }
  .this.expanded {
    margin-left: -48px;
    transition: margin-left 0.2s ease-in-out;
  }
  .icon {
    height: 24px;
    width: 24px;
    margin: 4px 12px;
  }
  .name {
    width: 192px;
    padding-left: 2px;
    line-height: 24px;
    padding-top: 4px;
  }
  .btn-group {
    margin-top: 4px;
    margin-left: -2px;
  }
  .blink_img {
    animation: blinker_img 1s linear infinite;
  }
  .btn-group > .btn {
    background: #444;
    border-color: #333;
    padding: 6px 6px 7px 6px;
    margin-top: -5px;
    border-radius: 0px;
    margin-left: -1px;
  }
  .btn-group > .btn:hover {
    background: #555;
    border-color: #333;
  }
  @keyframes blinker_img {
    20% { opacity: 0.2; }
  }
</style>
