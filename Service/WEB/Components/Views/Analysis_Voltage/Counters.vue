<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg">From 10 minutes aggregation<br>{{ Time10M }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-for="phase in CurrentValue10M" v-html="phase"></div>
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
      <div class="line line-border">
        <div class="float-left">
          <div class="counters-stat-type">Max value:</div>
          <div class="counters-stat-time">{{ MaxTime }}</div>
        </div>
        <div class="counters-stat-val" v-bind:class="{ 'counters-stat-val2': (MinArray.length > 1) }"><div v-for="phase in MaxArray" v-html="phase"></div></div>
      </div>
      <div class="line line-border">
        <div class="float-left">
          <div class="counters-stat-type">Average value:</div>
          <div class="counters-stat-time">From {{ AvgSamples }} samples</div>
        </div>
        <div class="counters-stat-val" v-bind:class="{ 'counters-stat-val2': (AvgArray.length > 1) }"><div v-for="phase in AvgArray" v-html="phase"></div></div>
      </div>
      <div class="line line-border">
        <div class="float-left">
          <div class="counters-stat-type">Std deviation:</div>
          <div class="counters-stat-time" v-html="StdPercent"></div>
        </div>
        <div class="counters-stat-val" v-bind:class="{ 'counters-stat-val2': (AvgArray.length > 1) }"><div v-for="phase in StdArray" v-html="phase"></div></div>
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
                AvgSamples: '', AvgArray: [], StdPercent: '', StdArray: [],
                Time10M: '', CurrentValue10M: [],
                TimeStatsFrom: '...', TimeStatsTo: '...'
              };
    },
    created: function() {
      this.NowTimer = null;
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      this.NowValue();
    },
    beforeDestroy: function() {
      clearInterval(this.NowTimer);
    },
    methods: {
      SetAnalyzer: function() {
        clearInterval(this.NowTimer);
        this.NowValue();
      },
      NowValue: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['VOLTAGE'], To: 0, Aggreg: '10M', Period: 1200000, LastSample: 0, Analyzer: this.$root.SelectedAnalyzer.Analyzer }), 
          success: function(result) {
            if(result['Sampletime'].length === 0) {
              this.CurrentValue10M = ['No data'];
              this.Time10M = '';
            } else {
              var Pos = result['Sampletime'].length - 1;
              this.Time10M = 'At ' + moment(result.Sampletime[Pos]).format('HH:mm:ss');
              if(result['Voltage'][Pos] === null) {
                this.CurrentValue10M = ['No data'];
              } else {
                for(var i = 0; i < result['Voltage'][Pos].length; i++) this.CurrentValue10M[i] = oZm.Round2(result['Voltage'][Pos][i]) + ' V';
              }
            }
          }.bind(this),
          complete: function() {
            clearTimeout(this.NowTimer);
            this.NowTimer = setTimeout(this.NowValue.bind(this), 5000);
            this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
          }.bind(this), 
          error: function() {
            this.CurrentValue10M = ['No data'];
            this.Time10M = '';
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
          var Val = oZm.Series['VOLTAGE'].FormatValue(MinArray[p]);
          this.MinArray.push(Val.Value + '<em>' + Val.Units + '</em>');
          Val = oZm.Series['VOLTAGE'].FormatValue(MaxArray[p]);
          this.MaxArray.push(Val.Value + '<em>' + Val.Units + '</em>');
          Val = oZm.Series['VOLTAGE'].FormatValue(AvgArray[p] / AvgSamples);
          this.AvgArray.push(Val.Value + '<em>' + Val.Units + '</em>');
          Val = oZm.Series['VOLTAGE'].FormatValue(Math.sqrt((StdArray[p] / AvgSamples) - Math.pow(AvgArray[p] / AvgSamples, 2)));
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
    border-top: 1px solid #efbab8;
  }
</style>
