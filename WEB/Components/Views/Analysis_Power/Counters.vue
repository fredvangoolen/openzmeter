<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg">From 3s seconds aggregation<br>{{ Time3S }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-html="CurrentValue3S"></div>
        <div class="counters-phases" v-html="CurrentValue3SPhases" v-if="CurrentValue3SPhases != ''"></div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg">From 1 minute aggregation<br>{{ Time1M }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-html="CurrentValue1M"></div>
        <div class="counters-phases" v-html="CurrentValue1MPhases" v-if="CurrentValue1MPhases != ''"></div>
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
        <div class="counters-stat-val" v-html="MinValue"></div>
      </div>
      <div class="line line-border">
        <div class="float-left">
          <div class="counters-stat-type">Max value:</div>
          <div class="counters-stat-time">{{ MaxTime }}</div>
        </div>
        <div class="counters-stat-val" v-html="MaxValue"></div>
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
    data () {
      return { CountersAggregation: 'Loading data...', MinTime: '', MinValue: '', MaxTime: '', MaxValue: '',
               Time3S: '', CurrentValue3S: '', CurrentValue3SPhases: '', 
               Time1M: '', CurrentValue1M: '', CurrentValue1MPhases: '', 
               TimeStatsFrom: '...', TimeStatsTo: '...',
               ShowSpinner: true };
    },
    beforeDestroy: function() {
      $(window).off('resize', this.ResizeHandler);
    },
    methods: {
      FormatPhases: function(values) {
        var Return = '';
        if(values.length > 1) {
          for(var n = 0; n < values.length; n++) {
            var Format = oZm.Series['ACTIVE_POWER'].FormatValue(values[n]);
            Return += Format.Value + '<em>' + Format.Units +'</em><br>';
          }
        }
        return Return;
      },
      SumPhases: function(values) {
        var Sum = 0;
        for(var n = 0; n < values.length; n++) Sum += values[n];
        return Sum;
      },
      FormatAggregBlock(result) {
        if(result === null) return ['', 'No data', ''];
        
        if(result['Sampletime'].length === 0) return ['', 'No data', ''];
        var Pos = result['Sampletime'].length - 1;
        var Time = 'At ' + moment(result.Sampletime[Pos]).format('HH:mm:ss');
        var Value = 'No data';
        var Phases = '';
        if(result['Active_Power'][Pos] !== null) {
          Phases = this.FormatPhases(result['Active_Power'][Pos]);
          var Format = oZm.Series['ACTIVE_POWER'].FormatValue(this.SumPhases(result['Active_Power'][Pos]));
          Value = Format.Value + '<em>' + Format.Units +'</em>';
        }
        return [Time, Value, Phases];
      },
      Set3SValues: function(result) {
        [this.Time3S, this.CurrentValue3S, this.CurrentValue3SPhases] = this.FormatAggregBlock(result);
      },
      Set1MValues: function(result) {
          [this.Time1M, this.CurrentValue1M, this.CurrentValue1MPhases] = this.FormatAggregBlock(result);
      },
      
      
      
      
      
      ResizeHandler: function() {
        if(typeof this.$refs.graph !== 'undefined') this.$refs.graph.Redraw('100%', $(this.$refs.counters).height());
      },
      MaximizeClick: function() {
        $(window).unbind('resize', this.ResizeHandler);
        this.$refs.tools.Maximize('Energy consumption', this.$refs.graph.Graph, this.$refs.Toolbar);
      },
      MinimizeClick: function() {
        $(window).bind('resize', this.ResizeHandler.bind(this));
      },
      MouseUpEvt: function(event, g, contex) {
        oZm.mouseUp(event, g, contex);
        if((contex.isPanning) && (contex.regionWidth > 0)) this.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
      },
      DoubleClickEvt: function(event, g, context) {
        oZm.dblClick(event, g, context);
        this.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
      },
      ZoomEvt: function(minDate, maxDate, yRanges) {
        this.$emit('ZoomEvt', minDate, maxDate);
      }
    },
    components: { Spinner, ToolbarAggregation, ToolbarPeriod, ToolbarTools, Graph }
  }

</script>