<template>
  <div class="cubism-serie" @mouseenter="$emit('mouseOver', +1)" @mouseleave="$emit('mouseOver', -1)">
    <div class="graph" ref="graph" :class="{ first: (index==0) }"></div>
    <div class="legend" ref="legend"></div>
  </div>
</template>

<script>
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';
  import 'JS/dygraph-crosshair.js';
  import moment from 'moment';
  export default {
    props: { index: Number, data: Object, title: String, format: Function, now: Object, paused: Boolean },
    data() {
      return { };
    },
    watch: {
      data: function(data) {
        if((this.paused == true) || (this.Graph == null)) return;
        var Opt = { file: data[this.title].Rows };
        var Range = 1;
        if(data[this.title].Rows.length == 0) {
          this.Graph.updateOptions({ file: [[0,0]] });
          return;
        }
        for(var n = 0; n < data[this.title].Rows.length; n++) Range = Math.max(Range, Math.abs(data[this.title].Rows[n][1]));
        Opt.valueRange = [-Range, +Range];
        Opt.dateWindow = [data[this.title].From, data[this.title].To];
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
      DrawBorder: function(ctx, area, dygraph) {
        ctx.strokeStyle = 'black';
        ctx.strokeRect(area.x, area.y, area.w, area.h);
      },
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
      Opt.underlayCallback = this.DrawBorder;
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
      Opt.axes.x.valueFormatter = (x) => { return '<div class="x-axis">' + moment.tz(x, this.data.TimeZone).format('HH:mm:ss') + '</div>'; };
      Opt.axes.x.ticker = (min, max) => {
        var ticks = [];
        if(this.data.TimeZone == null) return ticks;
        var Min = moment.tz(min, this.data.TimeZone).startOf('hour');
        var Max = moment.tz(max, this.data.TimeZone).endOf('hour');
        for(var h = Min; h.isBefore(Max); h.add(1, 'hour')) ticks.push({ v: h.valueOf(), label: (this.index != 0) ? '' : (h.hours() == 0) ? '<div class="timeline-day">' + h.format('DD/MM') + '</div>' : '<div class="timeline">' + h.format('HH') + 'h</div>' });
        return ticks;
      };
      this.Graph = new Dygraph(this.$refs.graph, [[0, 0]], Opt);
    },
    beforeDestroy: function() {
      if(this.Graph != null) this.Graph.destroy();
    }
  }
</script>

<style>
  .cubism-serie .first {
    margin-top: 16px;
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
    cursor: crosshair;
    margin-top: -1px;
  }
  .cubism-serie > .graph {
    height: 50px !important;
    width: 100% !important;
  }
  .cubism-serie {
    user-select: none;
  }
  .cubism-serie:first-child > .graph {
    margin-top: 16px;
/*    border-top: 1px solid #BBB;*/
  }
  .cubism-serie > .legend {
    position: absolute;
    top: -2px;
    text-align: left;
    left: 2px;
  }
  .cubism-serie .timeline {
    margin-top: -63px;
    font-size: 10px;
    color: black;
    line-height: 10px;
    pointer-events: none;
  }
  .cubism-serie .timeline-day {
    margin-top: -63px;
    font-weight: bold;
    font-size: 10px;
    color: black;
    line-height: 10px;
    pointer-events: none;
  }
</style>