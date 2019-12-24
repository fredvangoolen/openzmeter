<template>
  <graph :options="GraphOptions" ref="graph"></graph>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  export default {
    props: { value: Array },
    data () {
      return { };
    },
    created: function() {
      this.Labels = [];
      this.GraphOptions = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', 
        xAxisHeight: 18, legendFormatter: Graph.LegendFormatter_Follow, plotter: Graph.multiColumnBarPlotter, labels: ['Time', 'Cost'], colors: [oZm.Series['ACTIVE_ENERGY'].Color], includeZero: true,
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
      if(this.value != null) this.SetData();
    },
    watch: {
      value: function(val) {
        this.$refs.graph.SetOptions( { dateWindow: [-0.5, 0.5], file: [[0, 0]] } );
        if(val != null) this.SetData();
      }
    },
    methods: {
      ResizeHandler: function() {
        if(typeof this.$refs.graph !== 'undefined') this.$refs.graph.Redraw('100%', '300px');
      },
      SetData: function() {
        var Summary = [];
        var MonthsNames = moment.months();
        for(var i = 0; i < this.value.length; i++) {
          this.Labels.push(MonthsNames[moment(this.value[i].To).month()]);
          Summary.push([i, (this.value[i].Complete === true) ? this.value[i].Total : null]);
        }
        this.$refs.graph.SetOptions( { dateWindow: [-0.5, this.value.length - 0.5], file: Summary } );
        this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
      }
    },
    components: { Graph }
  }
</script>