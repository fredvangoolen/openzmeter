<template>
  <div class="cubism-serie">
    <div class="graph" ref="graph"></div>
    <div class="legend" ref="legend"></div>
  </div>
</template>

<script>
  import Dygraph from 'dygraphs';
  import * as oZm from '../JS/openZmeter.js';
  function barChartPlotterCubism(e) {
    var ctx = e.drawingContext;
    var area = e.plotArea;
    if(area.h <= 0 || area.w <= 0) return;
    var points = e.points;
    var BarWidth = Infinity;
    for(var i = 1; i < points.length; i++) {
      var Sep = points[i].canvasx - points[i - 1].canvasx;
      if(Sep < BarWidth) BarWidth = Sep;
    }
    var ColorP = Dygraph.toRGB_(e.color);
    ColorP = 'rgba(' + Math.floor(ColorP.r) + ',' + Math.floor(ColorP.g) + ',' + Math.floor(ColorP.b) + ', 0.3)';
    var ColorN = Dygraph.toRGB_(e.color);
    ColorN = 'rgba(' + Math.floor(255 - ColorN.r) + ',' + Math.floor(255 - ColorN.g) + ',' + Math.floor(255 - ColorN.b) + ', 0.3)';
    var Height = area.h - 1;
    var Midpoint = ( e.axis.minyval + e.axis.maxyval) / 2;
    var Scale = (Height * 3) / (e.axis.maxyval - Midpoint);
    for(var i = 0; i < points.length; i++) {
      var BarHeight = (points[i].yval - Midpoint) * Scale;
      if((BarHeight > (Height * 3)) || (BarHeight < (-Height * 3))) continue;
      while(true) {
        ctx.fillStyle = (BarHeight > 0) ? ColorP : ColorN;
        if(BarHeight > area.h) {
          ctx.fillRect(points[i].canvasx - (BarWidth / 2.0), 0, BarWidth, Height);
          BarHeight -= area.h;  
        } else if(-BarHeight > area.h) {
          ctx.fillRect(points[i].canvasx - (BarWidth / 2.0), 0, BarWidth, Height);
          BarHeight += area.h;
        } else {
          if(BarHeight > 0) {
            ctx.fillRect(points[i].canvasx - (BarWidth / 2.0), Height - BarHeight, BarWidth, BarHeight);
            points[i].canvasy = area.h - BarHeight;
          } else {
            ctx.fillRect(points[i].canvasx - (BarWidth / 2.0), 0, BarWidth, -BarHeight);
            points[i].canvasy = -BarHeight;
          }
          break;
        }
      }
    }
  }
  export default {
    props: ['serie', 'first'],
    data() {
      return { Graph: null, SerieProps: {} };
    },
    methods: {
      //function SetCubismNowLabel(graph, labels) {
//  if(graph !== null) {
//    var Index = 0;
//    for(var i = 0; i < graph.user_attrs_.visibility.length; i++) {
//      if(graph.user_attrs_.visibility[i] === true) Index = i;
//    }
//    var Labels = graph.user_attrs_.labels;
//    Labels[0] = (labels === undefined) ? 'Time' : graph.axes_[Index].valueFormatter(labels[Index]);
//    graph.updateOptions( { labels: Labels });
//  }
//}

      SetData: function(data, from, to, index) {
        var Data = [];
        for(var i = 0; i < data[this.serie].length; i++) Data.push([data[this.serie][i][0], data[this.serie][i][1 + index]]);
        this.Graph.updateOptions({ file: Data, dateWindow: [from, to] });
      }
    },
    created: function() {
      this.$parent.$on('SeriesUpdate', this.SetData);
      this.SerieProps = oZm.Series[this.serie];
    },
    mounted: function() {
      this.Graph = new Dygraph(this.$refs.graph, [[0, 0]], { strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], highlightCircleSize: 0, legend: 'always', xAxisHeight: 0.1, xLabelHeight: 1, interactionModel: { }, plotter: barChartPlotterCubism, labels: ["Time", this.SerieProps.Title], colors: ["#d9534f"], labelsDiv: this.$refs.legend,
  //    plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        legendFormatter: function(data) {
          if(data.x === null) {
            var Value = '<span class="cubism-legend-inactive-label">' + data.series[0].label + '</span>';
            if(data.dygraph.user_attrs_.labels[0] !== 'Time') Value = Value + '<br/><span class="cubism-legend-inactive-value"> ' + data.dygraph.user_attrs_.labels[0] + '</span>';
            return  Value;
          } else {
            var Caption = '<span style="font-size: 15px; font-weight: bold">' + data.series[0].label + '</span><br/>' + data.xHTML;
            for(var i = 0; i < data.series.length; i++)  {
              if(data.series[i].yHTML !== undefined) return Caption + '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + data.series[i].yHTML + '</div>';
            }
            return Caption + '<div style="font-size: 14px; line-height: 18px; font-weight: bold">-</div>';
          }
        },
        axes: {
          y: { drawAxis: false, drawGrid: false, valueFormatter: this.SerieProps.FormatValue},
          x: { valueFormatter: function (x) { return '<div class="cubism-x-axis">' + new Date(x).toLocaleTimeString() + '</div>'; },
               axisLabelFormatter: (!this.first) ? function() { return null; } : function(x, i) {
                 var date = new Date(x);
                 if((date.getHours() == 0) && (date.getMinutes() == 0) && (date.getSeconds() == 0)) {
                   return '<div class="timeline-day">' + date.toLocaleDateString() + '</div>';
                 } else {
                   return '<div class="timeline">' + date.toLocaleTimeString() + '</div>';                       
                 }
               }
             }
        }
      });
    }
  }
</script>

<style>
  .cubism-x-axis {
    font-size: 10px;
    line-height: 10px
  }
  .cubism-legend-inactive-label {
    font-size: 18px;
    font-weight: bold;
  }
  .cubism-legend-inactive-value {
    font-size: 20px; 
    font-weight: bold;
  }
</style>