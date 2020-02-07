<template>
  <div>
    <graph :options="GraphOptions" :height="height" @range="$emit('range', $event)"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object, data: Object, height: { type: Number, default: 300 } },
    data() {
      return { };
    },
    computed: {
      GraphOptions: function() {
        var Opt = {};
        Opt.strokeWidth = 0.5;
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.gridLineColor = '#BBB';
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.LegendFormatter_Follow;
        Opt.labels = ['Time', 'Frequency'];
        Opt.interactionModel = Graph.ScrollableInteractionModel(this);
        Opt.colors = [this.analyzer.Color.Frequency]
        Opt.xAxisHeight = 24;
        Opt.axes = {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: (x) => { return oZm.Round3(x) + ' <em class="x-small">Hz</em>'; },
               axisLabelFormatter: (x) => { return oZm.Round3(x); }
             },
          x: { axisLabelFormatter: (x) => { return this.analyzer.FormatDateTime(x, '<br/>'); },
               valueFormatter: (x) => { return this.analyzer.FormatDateTime(x); }
             }
        };
        Opt.file = [];
        if(this.data != null) {
          for(var n in this.data.Sampletime) Opt.file.push([this.data.Sampletime[n], this.data.Frequency[n]]);
        } else {
          Opt.file.push([0, 0]);
        }
        return Opt;
      }
    },
    methods: {
      MaximizeClick: function() {
        $(window).unbind('resize', this.ResizeHandler);
        this.$refs.tools.Maximize('Energy consumption', this.$refs.graph.Graph, this.$refs.Toolbar);
      },
      MinimizeClick: function() {
        $(window).bind('resize', this.ResizeHandler.bind(this));
      },
      WindowRestore: function() {
        this.$refs.period.PopState();
      },
      WindowChange: function(minDate, maxDate) {
        this.$refs.period.SetCustom(minDate, maxDate, true);
      }
    }
  }
</script>