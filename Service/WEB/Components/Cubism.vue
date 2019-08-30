<template>
  <div class="cubism-serie">
    <div class="graph" ref="graph" v-bind:class="{'first': (index == 0)}"></div>
    <div class="legend" ref="legend"></div>
  </div>
</template>

<script>
  import Dygraph from 'JS/dygraph.js';
  import 'JS/dygraph-crosshair.js';
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'JS/moment.js';
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
    props: ['serie', 'index'],
    data() {
      return { };
    },
    methods: {
      SetNowLabel: function(label) {
        if(this.Graph === null) return;
        var Labels = this.Graph.user_attrs_.labels;
        if(label === undefined) {
          Labels[0] = 'Time';
        } else {
          var Result = this.SerieProps.FormatValue(label); 
          Labels[0] = Result.Value + Result.Units;
        }
        this.Graph.updateOptions( { labels: Labels });
      },
      SetData: function(data) {
        var Range = 0;
        for(var n = 0; n < data.file.length; n++) {
          if(Math.abs(data.file[n][1]) > Range) Range = Math.abs(data.file[n][1]);
        }
        data.valueRange = [-Range, +Range];
        this.Graph.updateOptions(data);
      },
      HoverIn() {
        this.$emit('mouseOver', true);
      },
      HoverOut() {
        this.$emit('mouseOver', false);
      },
    },
    created: function() {
      this.$parent.$on('SeriesUpdate', this.SetData);
      this.SerieProps = oZm.Series[this.serie];
      this.Graph = null;
    },
    mounted: function() {
      this.Graph = new Dygraph(this.$refs.graph, [[0, 0]], { highlightCircleSize: 0, legend: 'always', xAxisHeight: 0.1, xLabelHeight: 1, interactionModel: { }, plotter: barChartPlotterCubism, labels: ["Time", ((this.SerieProps.Title === undefined) ? this.SerieProps.Names[this.index] : this.SerieProps.Title)], colors: ["#d9534f"], labelsDiv: this.$refs.legend,
        highlightCallback: this.HoverIn,
        unhighlightCallback: this.HoverOut,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        legendFormatter: function(data) {
          if(data.x === undefined) {
            var Value = '<span class="inactive-label">' + data.series[0].label + '</span>';
            if(data.dygraph.user_attrs_.labels[0] !== 'Time') Value = Value + '<br/><span class="inactive-value"> ' + data.dygraph.user_attrs_.labels[0] + '</span>';
            return  Value;
          } else {
            var Caption = '<span class="active-label">' + data.series[0].label + '</span><br/>' + data.xHTML;
            for(var i = 0; i < data.series.length; i++)  {
              if(data.series[i].yHTML !== undefined) return Caption + '<div class="active-value">' + data.series[i].yHTML + '</div>';
            }
            return Caption + '<div class="active-value">-</div>';
          }
        },
        axes: {
          y: { drawAxis: false, drawGrid: false, valueFormatter: function(val) { var Result = this.SerieProps.FormatValue(val); return Result.Value + Result.Units; }.bind(this) },
          x: { valueFormatter: function (x) { return '<div class="x-axis">' + moment(x).format('hh:mm:ss') + '</div>'; },
               axisLabelFormatter: (this.index !== 0) ? function() { return null; } : function(x, i) {
                 var date = moment(x);
                 if((date.hours() === 0) && (date.minutes() === 0) && (date.seconds() === 0)) {
                   return '<div class="timeline-day">' + moment(x).format('DD/MM') + '</div>';
                 } else {
                   return '<div class="timeline">' + moment(x).format('hh:mm') + '</div>';                       
                 }
               }
             }
        }
      });
    },
    beforeDestroy: function() {
      this.Graph.destroy();
    }
  }
</script>

<style>
  .cubism-serie .first {
    margin-top: 16px;
    border-top: 1px solid #BBB;
  }
  .cubism-serie .x-axis {
    font-size: 10px;
    line-height: 10px
  }
  .cubism-serie .inactive-label {
    font-size: 18px;
    font-weight: bold;
  }
  .cubism-serie .inactive-value {
    font-size: 20px; 
    font-weight: bold;
  }
  .cubism-serie .active-label {
    font-size: 15px;
    font-weight: bold;
  }
  .cubism-serie .active-value {
    font-size: 14px;
    line-height: 18px;
    font-weight: bold;
  }
  .cubism-serie {
    position: relative;  
  }
  .cubism-serie > .graph {
    height: 50px !important;
    width: 100% !important;
  }
  .cubism-serie:first-child > .graph {
    margin-top: 16px;
    border-top: 1px solid #BBB;
  }
  .cubism-serie > .legend {
    pointer-events: none;
    position: absolute;
    top: -2px; 
    text-align: left; 
    left: 2px;
  }
  .cubism-serie .timeline {
    margin-top: -63px;
  }
  .cubism-serie .timeline-day {
    margin-top: -63px;
    font-weight: bold;
  }
</style>