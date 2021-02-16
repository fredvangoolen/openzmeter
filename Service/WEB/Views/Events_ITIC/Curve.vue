<template>
  <div class="panel-body">
    <spinner v-if="data==null"></spinner>
    <graph :options="GraphOptions"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    props: { data: Object, analyzer: Object },
    data: function() {
      return { };
    },
    computed: {
      GraphOptions: function() {
        var Opt = { axes: { x: {}, y: {} }, file: [[0, 0]] };
        Opt.strokeWidth = 0;
        Opt.labels = ["Duration", "Variation"];
        Opt.legend = 'follow';
        Opt.showRangeSelector = false;
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.drawPoints = true;
        Opt.pointSize = 3;
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.highlightCircleSize = 6;
        Opt.colors = ["#d9534f"];
        Opt.dateWindow = [1, 20000];
        Opt.valueRange = [0, 200];
        Opt.selectMode = 'euclidian';
        Opt.interactionModel = {};
        Opt.underlayCallback = this.Underlay;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 24;
        Opt.axes.y.valueFormatter = (x) => { return this.Round(x, 2, false) + '%'; };
        Opt.axes.y.axisLabelFormatter = (x) => { return x + '%'; };
        Opt.axes.x.pixelsPerLabel = 34;
        Opt.axes.x.axisLabelWidth = 34;
        Opt.axes.x.logscale = true;
        Opt.axes.x.valueFormatter = (x) => { return (x < 1000) ? this.Round(x, 2, false) + 'ms' : this.Round(x / 1000, 2, false) + 's'; };
        Opt.axes.x.axisLabelFormatter = (x) => { return (x < 1000) ? x + 'ms' : (x / 1000) + 's'; };
        if(this.data != null) {
          var PeakCol  = this.data.Headers.indexOf('Peak');
          var DurationCol = this.data.Headers.indexOf('Duration');
          var SamplerateCol = this.data.Headers.indexOf('SampleRate');
          for(var i = 0; i < this.data.Rows.length; i++) Opt.file.push([(this.data.Rows[i][DurationCol] / this.data.Rows[i][SamplerateCol]) * 1000.0, 100 + this.MaxDiff(this.data.Rows[i][PeakCol])]);
        }
        return Object.seal(Opt);
      }
    },
    methods: {
      Underlay: function(canvas, area, g) {
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
        canvas.font = "12px Titillium Web";
        canvas.fillStyle = "#333";
        canvas.textAlign = 'right';
        canvas.fillText("No damage region",  c + w - 5 , g.toDomYCoord(0) - 5);
        canvas.fillText("Prohibited region", c + w - 5 , 14);
        canvas.textAlign = "left";
        canvas.fillText("No interruption region", c + 5 , g.toDomYCoord(0) - 5);
        canvas.restore();
        Graph.Callbacks.Underlay_DrawBorder(canvas, area, g);
      },
      MaxDiff: function(a) {
        var Max = a[0] - this.analyzer.NominalVoltage;
        for(var n = 1; n < a.length; n++) {
          if(Math.abs(a[n] - this.analyzer.NominalVoltage) > Math.abs(Max)) Max = a[n] - this.analyzer.NominalVoltage;
        }
        return (Max * 100.0) / this.analyzer.NominalVoltage;
      }
    },
    components: { Graph }
  };
</script>