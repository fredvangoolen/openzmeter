<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg">From 3 seconds aggregation<br>{{ Time3S }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-for="phase in CurrentValue3S" v-html="phase"></div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg">From 1 minute aggregation<br>{{ Time1M }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-for="phase in CurrentValue1M" v-html="phase"></div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Period stats:</div>
      <div class="counters-aggreg">{{ CountersAggregation }}<br>From {{ TimeStatsFrom }} to {{ TimeStatsTo }}</div>
      <div class="line">
        <div class="float-left">
          <div class="counters-stat-type">Min value:</div>
          <div class="counters-stat-time">{{ MinTime }}</div>
        </div>
        <div class="counters-stat-val" v-bind:class="{ 'counters-stat-val2': (MinArray.length > 1) }"><div v-for="phase in MinArray" v-html="phase"></div></div>
      </div>
      <div class="line line-border" v-bind:class="{'line-border-reactive': (variable == 'REACTIVE_POWER')}">
        <div class="float-left">
          <div class="counters-stat-type">Max value:</div>
          <div class="counters-stat-time">{{ MaxTime }}</div>
        </div>
        <div class="counters-stat-val" v-bind:class="{ 'counters-stat-val2': (MinArray.length > 1) }"><div v-for="phase in MaxArray" v-html="phase"></div></div>
      </div>
    </div>
  </div>
</template>

<script>
  import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
  import ToolbarPeriod from 'Components/ToolbarPeriod.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: ['variable'],
    data () {
      return { CountersAggregation: 'Loading data...', MinTime: '', MinArray: [], MaxTime: '', MaxArray: [],
                Time3S: '', CurrentValue3S: [], Time1M: '', CurrentValue1M: [],
                TimeStatsFrom: '...', TimeStatsTo: '...'
              };
    },
    created: function() {
      this.NowTimer3S = null;
      this.NowTimer1M = null;
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      this.NowValue3S();
      this.NowValue1M();
    },
    beforeDestroy: function() {
      clearInterval(this.NowTimer3S);
      clearInterval(this.NowTimer1M);
    },
    methods: {
      SetAnalyzer: function() {
        clearInterval(this.NowTimer3S);
        clearInterval(this.NowTimer1M);
        this.NowValue3S();
        this.NowValue1M();
      },
      NowValue3S: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['CURRENT'], To: 0, Aggreg: '3S', Period: 6000, Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            if(result['Sampletime'].length === 0) {
              this.CurrentValue3S = ['No data'];
              this.Time3S = '';
            } else {
              var Pos = result['Sampletime'].length - 1;
              this.Time3S = 'At ' + moment(result.Sampletime[Pos]).format('HH:mm:ss');
              if(result['Current'][Pos] === null) {
                this.CurrentValue3S = ['No data'];
              } else {
                for(var i = 0; i < result['Current'][Pos].length; i++) this.CurrentValue3S[i] = oZm.Round2(result['Current'][Pos][i]);
              }
            }
          }.bind(this),
          complete: function() {
            this.NowTimer = setTimeout(this.NowValue3S.bind(this), 5000);
            this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
          }.bind(this), 
          error: function() {
            this.CurrentValue3S = ['No data'];
            this.Time3S = '';
          }.bind(this)
        });
      },
      NowValue1M: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['CURRENT'], To: 0, Aggreg: '1M', Period: 120000, Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            if(result['Sampletime'].length === 0) {
              this.CurrentValue1M = ['No data'];
              this.Time1M = '';
            } else {
              var Pos = result['Sampletime'].length - 1;
              this.Time1M = 'At ' + moment(result.Sampletime[Pos]).format('HH:mm:ss');
              if(result['Current'][Pos] === null) {
                this.CurrentValue1M = ['No data'];
              } else {
                for(var i = 0; i < result['Current'][Pos].length; i++) this.CurrentValue1M[i] = oZm.Round2(result['Current'][Pos][i]);
              }
            }
          }.bind(this),
          complete: function() {
            clearTimeout(this.NowTimer);
            this.NowTimer = setTimeout(this.NowValue1M.bind(this), 30000);
            this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
          }.bind(this), 
          error: function() {
            this.CurrentValue1M = ['No data'];
            this.Time1M = '';
          }.bind(this)
        });
      },
      SetStats: function(datewindow, data, aggreg) {
        this.CountersAggregation = 'From ' + oZm.GetAggregLongName(aggreg) + ' aggregation';
        this.TimeStatsFrom = moment(datewindow[0]).format('DD/MM/YYYY HH:mm:ss');
        this.TimeStatsTo = moment(datewindow[1]).format('DD/MM/YYYY HH:mm:ss');
        var MinValue = +Infinity, MaxValue = -Infinity, MinCount = 0, MaxCount = 0, MinTime = null, MaxTime = null, StdPercent = 0, AvgSamples = 0, MinArray, MaxArray;
        var AvgArray = Array(data[0].length).fill(0), StdArray = Array(data[0].length).fill(0);
        for(var x = 0; x < data.length; x++) {
          for(var p = 1; p < data[x].length; p++) {
            if(data[x][p] === null) continue;
            if(data[x][p] < MinValue) {
              MinArray = data[x].slice(1);
              MinValue = data[x][p];
              MinCount = 0;
              MinTime = data[x][0];
            } else if(data[x][p] === MinValue) {
              MinCount++;
            }
            if(data[x][p] > MaxValue) {
              MaxArray = data[x].slice(1);
              MaxValue = data[x][p];
              MaxCount = 0;
              MaxTime = data[x][0];
            } else if(data[x][p] === MaxValue) {
              MaxCount++;
            }
            AvgArray[p - 1] = AvgArray[p - 1] + data[x][p];
            StdArray[p - 1] = StdArray[p - 1] + Math.pow(data[x][p], 2.0); 
          }
          var InRange = true;
          for(var p = 1; p < data[x].length; p++) {
            if((data[x][p] > 240) || (data[x][1] < 220)) InRange = false;
          }
          if(InRange === true) StdPercent++;
          AvgSamples++;
        }
        this.AvgSamples = AvgSamples;
        this.MinTime = moment(MinTime).format("hh:mm:ss - DD/MM/YYYY") + ((MinCount > 1) ? ' (and ' + MinCount + ' times more)' : '');
        this.MaxTime = moment(MaxTime).format("hh:mm:ss - DD/MM/YYYY") + ((MaxCount > 1) ? ' (and ' + MaxCount + ' times more)' : '');
        this.StdPercent = oZm.Round2(StdPercent / AvgSamples * 100) + '% within range';    
        this.MinArray = [];
        this.MaxArray = [];
        this.StdArray = [];
        this.AvgArray = [];
        for(var p = 0; p < MinArray.length; p++) {
          var Val = oZm.Series['CURRENT'].FormatValue(MinArray[p]);
          this.MinArray.push(Val.Value + '<em>' + Val.Units + '</em>');
          Val = oZm.Series['CURRENT'].FormatValue(MaxArray[p]);
          this.MaxArray.push(Val.Value + '<em>' + Val.Units + '</em>');
          Val = oZm.Series['CURRENT'].FormatValue(AvgArray[p] / AvgSamples);
          this.AvgArray.push(Val.Value + '<em>' + Val.Units + '</em>');
          Val = oZm.Series['CURRENT'].FormatValue(Math.sqrt((StdArray[p] / AvgSamples) - Math.pow(AvgArray[p] / AvgSamples, 2)));
          this.StdArray.push(Val.Value + '<em>' + Val.Units + '</em>');
        }
        this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
      }
    }
  }
</script>

<style scoped>
  .counters-phases {
    text-align: left;
    display: inline-block;
    font-size: 12px;
    line-height: 12px;
    font-weight: bold;
    padding-left: 2px;
    border-left: 1px solid #f0ad4e;
    margin-left: 5px;
  }
  .counters-title {
    float: left;
    text-align: left;
    font-size: 16px;
  }
  .counters-aggreg {
    float: right;
    text-align: right;
    font-size: 12px;
  }
  .counters-stat-type {
    text-align: left;
    font-weight: bold;
  }
  .counters-stat-time {
    font-size: 12px;
    text-align: left;
  }
  .counters-stat-val {
    font-size: 28px;
    line-height: 37px;
    float: right;
  }
  .counters-stat-val2 {
    font-size: 12px !important;
    font-weight: bold;
    line-height: 12px;
  }
  .current-value {
    font-size: 30px;
    display: inline-block;
    margin: 0px 10px
  }
  .line {
    clear: both;
  }
  .line-border {
    border-top: 1px solid #cce0f1;
  }
</style>
