<template>
  <graph :options="GraphOptions" ref="graph"></graph>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'JS/moment.js';
  export default {
    data () {
      return { };
    },
    created: function() {
      this.Labels = [];
      this.GraphOptions = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', 
        xAxisHeight: 18, legendFormatter: oZm.LegendFormatter_Follow, plotter: Graph.multiColumnBarPlotter, labels: ['Time', 'Cost'], colors: [oZm.Series['ACTIVE_ENERGY'].Color], includeZero: true,
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 30,
               valueFormatter: function (x) { return oZm.Round2(x) + ' <em class="x-small">' + '€' + '</em>'; },
               axisLabelFormatter: function (x) { return oZm.Round2(x); }
             },
          x: { valueFormatter: function (x) { return this.Labels[x]; }.bind(this), 
               axisLabelFormatter: function (x) { return this.Labels[x]; }.bind(this) 
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
        if(typeof this.$refs.graph !== 'undefined') this.$refs.graph.Redraw('100%', '300px');
      },
      SetData: function(values) {
        var Summary = [];
        var MonthsNames = moment.months();
        for(var i = 0; i < values.length; i++) {
          this.Labels.push(MonthsNames[moment(values[i].Sampletime).month()]);
          Summary.push([i, values[i].Total]); //result.Bills[i].Sampletime, 
        }

        
        
        
        this.$refs.graph.SetOptions( { dateWindow: [-0.5, values.length - 0.5], file: Summary } );
        this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
      }
    },
    components: { Graph }
  }
</script>