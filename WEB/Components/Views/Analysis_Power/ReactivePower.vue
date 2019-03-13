<template>
  <div class="panel panel-reactive-power" id="ReactivePowerPanel">
    <div class="panel-heading" id="ReactivePowerPanel">
      <div class="header-text">Fund. Reactive Power</div>
      <div class="pull-right btn-group">
        <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="$emit('ExportClick')" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <div class="col-md-6 col-lg-4 col-xl-3" ref="counters">
          <div class="subpanel">
            <div class="counters-title">Last value:</div>
            <div class="counters-aggreg">From 3s seconds aggregation</div>
            <div class="clearfix"></div>
            <div class="current-value" v-html="CurrentValue3S"></div>
          </div>
          <div class="subpanel">
            <div class="counters-title">Last value:</div>
            <div class="counters-aggreg">From 1 minute aggregation</div>
            <div class="clearfix"></div>
            <div class="current-value" v-html="CurrentValue1M"></div>
          </div>
          <div class="subpanel clearfix">
            <div class="counters-title">Period stats:</div>
            <div class="counters-aggreg">{{ CountersAggregation }}</div>
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
        <div class="col-md-6 col-lg-8 col-xl-9">
          <graph ref="graph" :options="GraphOptions"></graph>
        </div>
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
    created: function() {
      this.GraphOptions = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow,
        labels: ['Time', 'Frequency'], zoomCallback: this.ZoomEvt.bind(this), interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: this.MouseUpEvt.bind(this), dblclick: this.DoubleClickEvt.bind(this) }, colors: [oZm.Series['FREQUENCY'].Color], xAxisHeight: 24, 
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return oZm.Round3(x) + ' <em class="x-small">Hz</em>'; },
               axisLabelFormatter: function (x) { return oZm.Round3(x); }
             },
          x: { axisLabelFormatter: function(x) { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); },
               valueFormatter: function (x) { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); }
             }
        }
      };
    },
    mounted: function() {
      $(window).on('resize', this.ResizeHandler.bind(this));
    },
    beforeDestroy: function() {
      $(window).off('resize', this.ResizeHandler);
    },
    methods: {
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