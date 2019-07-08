<template>
  <div class="panel-body"> 
    <graph ref="graph" :options="GraphOptions"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data: function() {
      return {};
    },
    created: function() {
      this.GraphOptions = { strokeWidth: 0, labels: ["Duration", "Variation"], legend: 'follow', drawPoints: true, pointSize: 3, highlightCircleSize: 6, colors: ["#d9534f"], dateWindow: [1, 20000], valueRange: [0, 200], selectMode: 'euclidian',
        interactionModel: {},
        underlayCallback: function(canvas, area, g) {
          canvas.save();
          var w = g.toDomXCoord(20000) - g.toDomXCoord(1);
          var c = g.toDomXCoord(1);
          var k = (w - c) / (2000 - 1);
          canvas.lineWidth = 1;
          canvas.fillStyle = '#bde2bd';
          canvas.fillRect(c, 0, area.w, area.h);
          canvas.beginPath();
          canvas.moveTo(c + (w * Math.log10(1)) / Math.log10(20000), g.toDomYCoord(100));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(100));
          canvas.stroke();
          canvas.lineWidth = 1;
          canvas.beginPath();
          canvas.moveTo(c + (w * Math.log10(1)) / Math.log10(20000), g.toDomYCoord(200));
          canvas.lineTo(c + (w * Math.log10(3)) / Math.log10(20000), g.toDomYCoord(140));
          canvas.lineTo(c + (w * Math.log10(3)) / Math.log10(20000), g.toDomYCoord(120));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(120));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(110));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(110));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(200));
          canvas.fillStyle = '#f3cbca';
          canvas.fill();
          canvas.beginPath();
          canvas.moveTo(c + (w * Math.log10(1)) / Math.log10(20000), g.toDomYCoord(200));
          canvas.lineTo(c + (w * Math.log10(3)) / Math.log10(20000), g.toDomYCoord(140));
          canvas.lineTo(c + (w * Math.log10(3)) / Math.log10(20000), g.toDomYCoord(120));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(120));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(110));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(110));
          canvas.strokeStyle = '#000';
          canvas.stroke();
          canvas.beginPath();
          canvas.moveTo(c + (w * Math.log10(20)) / Math.log10(20000), g.toDomYCoord(0));
          canvas.lineTo(c + (w * Math.log10(20)) / Math.log10(20000), g.toDomYCoord(70));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(70));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(80));
          canvas.lineTo(c + (w * Math.log10(10000)) / Math.log10(20000), g.toDomYCoord(80));
          canvas.lineTo(c + (w * Math.log10(10000)) / Math.log10(20000), g.toDomYCoord(90));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(90));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(0));
          canvas.fillStyle = '#f3cbca';
          canvas.fill();
          canvas.beginPath();
          canvas.moveTo(c + (w * Math.log10(20)) / Math.log10(20000), g.toDomYCoord(0));
          canvas.lineTo(c + (w * Math.log10(20)) / Math.log10(20000), g.toDomYCoord(70));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(70));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(80));
          canvas.lineTo(c + (w * Math.log10(10000)) / Math.log10(20000), g.toDomYCoord(80));
          canvas.lineTo(c + (w * Math.log10(10000)) / Math.log10(20000), g.toDomYCoord(90));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(90));
          canvas.stroke();
          canvas.font = "16px sans-serif";
          canvas.fillStyle = "#333";
          canvas.fillText("No interruption region", c + 5 , g.toDomYCoord(0) - 5);
          canvas.textAlign = "end";
          canvas.fillText("No damage region",  c + w - 5 , g.toDomYCoord(0) - 5);
          canvas.textBaseline = "top";
          canvas.fillText("Prohibited region", c + w - 5 , 5);
          canvas.restore();
        },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 24,
               valueFormatter: function (x) { return oZm.Round2(x) + '%'; },
               axisLabelFormatter: function (x) { return x + '%'; }
             },
          x: { pixelsPerLabel: 34, axisLabelWidth: 34, logscale: true,
               valueFormatter: function (x) { return (x < 1000) ? oZm.Round2(x) + 'ms' : oZm.Round2(x / 1000) + 's'; },
               axisLabelFormatter: function (x) { return (x < 1000) ? x + 'ms' : (x / 1000) + 's'; }
             }
        }
      };
    },
    mounted: function() {
      $(window).bind('resize', this.ResizeHandler.bind(this));
      window.dispatchEvent(new Event('resize'));
    },
    methods: {
      ResizeHandler: function() {
        if(typeof this.$refs.graph !== 'undefined') this.$refs.graph.Redraw('100%', $(this.$parent.$refs.subgraphs).height() - 20);
      },
      MaxDiff: function(a) {
        if(this.$root.SelectedAnalyzer === null) return 0;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Max = a[0] - Analyzer.NominalVoltage;
        for(var n = 1; n < a.length; n++) {
          if(Math.abs(a[n] - Analyzer.NominalVoltage) > Math.abs(Max)) Max = a[n] - Analyzer.NominalVoltage;
        }
        return (Max * 100.0) / Analyzer.NominalVoltage;
      },
      SetData: function(start, end, events) {
        var ByITIC = [[0, 0]];
        for(var i = 0; i < events.StartTime.length; i++) {
          if((events.StartTime[i] < start) || (events.StartTime[i] > end)) continue;
          ByITIC.push([(events.Duration[i] / events.SampleRate[i]) * 1000.0, 100 + this.MaxDiff(events.Peak[i])]);
        }
        this.$refs.graph.SetOptions({ file: ByITIC });
      }
    },
    components: { Graph }
  };
</script>