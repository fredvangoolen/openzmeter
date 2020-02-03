<template>
  <div class="cubism-serie" @mouseenter="$emit('mouseOver', +1)" @mouseleave="$emit('mouseOver', -1)">
    <div class="graph" ref="graph" :class="{ first: (index==0) }"></div>
    <div class="legend" ref="legend"></div>
  </div>
</template>

<script>
  import Dygraph from 'JS/dygraph.js';
  import 'JS/dygraph-crosshair.js';
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  export default {
    props: { index: Number, data: Object, title: String, format: Function, now: Object, paused: Boolean },
    data() {
      return { };
    },
    watch: {
      data: function(data) {
        if((this.paused == true) || (this.Graph == null)) return;
        var Opt = { file: [] };
        var Range = 0;
        for(var n in data.Sampletime) {
          if(Math.abs(data[this.title][n]) > Range) Range = Math.abs(data[this.title][n]);
          Opt.file.push([data.Sampletime[n], data[this.title][n]]);
        }
        if(Range < 1.0) Range = 1.0;
        Opt.valueRange = [-Range, +Range];
        Opt.dateWindow = [data.From, data.To];
        this.Graph.updateOptions(Opt);
      },
      now: function(label) {
        if((this.paused == true) || (this.Graph == null)) return;
        var Labels = this.Graph.user_attrs_.labels;
        if(label == null) {
          Labels[0] = 'Time';
        } else {
          Labels[0] = (label[this.title] == null) ? '-' : this.format(label[this.title]);
        }
        this.Graph.updateOptions( { labels: Labels });
      },
    },
    methods: {
      Plotter: function(e) {
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
    },
    created: function() {
      this.Graph = null;
    },
    mounted: function() {
      var Opt = { axes: { x: {}, y: {} } };
      Opt.highlightCircleSize = 0;
      Opt.legend = 'always';
      Opt.xAxisHeight = 0.1;
      Opt.xLabelHeight = 1;
      Opt.interactionModel = { };
      Opt.plotter = this.Plotter;
      Opt.labels = ["Time", this.title];
      Opt.colors = ["#d9534f"];
      Opt.labelsDiv = this.$refs.legend;
      Opt.plugins = [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ];
      Opt.legendFormatter = (data) => {
        if(data.x == null) {
          var Value = '<span class="inactive-label">' + data.series[0].label + '</span>';
          if(data.dygraph.user_attrs_.labels[0] != 'Time') Value = Value + '<br/><span class="inactive-value"> ' + data.dygraph.user_attrs_.labels[0] + '</span>';
          return  Value;
        } else {
          var Caption = '<span class="active-label">' + data.series[0].label + '</span><br/>' + data.xHTML;
          if(data.series[0].yHTML !== undefined) return Caption + '<div class="active-value">' + data.series[0].yHTML + '</div>';
          return Caption + '<div class="active-value">-</div>';
        }
      };
      Opt.axes.y.drawAxis = false;
      Opt.axes.y.drawGrid = false;
      Opt.axes.y.valueFormatter = (val) => { return this.format(val); };
      Opt.axes.x.valueFormatter = (x) => { return '<div class="x-axis">' + moment(x).format('HH:mm:ss') + '</div>'; };
      Opt.axes.x.axisLabelFormatter = (x, i) => {
        if(this.index != 0) return null;
        var date = moment(x).tz(this.data.TimeZone);
        if((date.hours() === 0) && (date.minutes() === 0) && (date.seconds() === 0)) {
          return '<div class="timeline-day">' + moment(x).format('DD/MM') + '</div>';
        } else {
          return '<div class="timeline">' + moment(x).format('HH:mm') + '</div>';
        }
      };
      this.Graph = new Dygraph(this.$refs.graph, [[0, 0]], Opt);
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