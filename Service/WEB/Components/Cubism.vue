<template>
  <div>
    <svg />
    <q-resize-observer @resize="onResize" />
  </div>
</template>

<script>
  import * as d3 from 'd3';
  export default {
    props: { data: Array, titles: Array, format: Array, now: Array, paused: Boolean },
    data() {
      return { MarginTop: 12, RowHeight: 50 };
    },
    mounted: function() {
      this.SVG = d3.select(this.$el).select('svg');
      for(let n = 0; n < this.titles.length; n++) {
        this.SVG.append('g')
          .attr('class', 'points-' + n)
          .attr('transform', 'translate(0, ' + (this.MarginTop + this.RowHeight * n) + ')');
        this.SVG.append('g')
          .attr('class', 'legend-' + n)
          .attr('transform', 'translate(0, ' + (this.MarginTop + this.RowHeight * n) + ')');
      }
      this.SVG.append('g')
        .attr('class', 'x-axis')
        .attr('transform', 'translate(0, ' + this.MarginTop + ')');
      this.SVG.append('g')
        .attr('class', 'x-grid')
        .attr('transform', 'translate(0, ' + this.MarginTop + ')');
      this.SVG.append('g')
        .attr('class', 'border');
    },
    watch: {
      data: function() {
        this.Redraw();
      },
      now: function() {
//        if((this.paused == true) || (this.Graph == null)) return;
//        var Labels = this.Graph.user_attrs_.labels;
//        if(label == null) {
//          Labels[0] = 'Time';
//        } else {
//          Labels[0] = (label[this.title] == null) ? '-' : this.format(label[this.title]);
//        }
//        this.Graph.updateOptions( { labels: Labels });
      },
//      width: function(val, prev) {
//        if(val == prev) return;
//        this.SVG.attr('width', this.width);
//        this.Redraw();
//      }
    },
    methods: {
      onResize: function(evt) {
        this.Height = this.RowHeight * this.titles.length + this.MarginTop;
        this.Width = evt.width;
        this.SVG.attr('height', this.Height);
        this.SVG.attr('width', this.Width);
         this.Redraw();
      },
      Redraw: function() {
        let Max = 0;
        let Now = this.$dayjs();
        let xScale = d3.scaleLinear()
          .domain([Now - 86400000, Now]);
        let xAxis = d3.axisTop(xScale)
          .tickSize(0)
          .tickSizeInner(2)
          .tickFormat(d => { let Time = this.$dayjs(d).tz(this.Analyzer.TimeZone); return (Time.hour() == 0) ? Time.format('DD/MM') : ((Time.hour() % 3) == 0) ? Time.format('HH') + 'h' : ''})
          .tickValues(Array.from(Array(25).keys()).map(d => Now.endOf('hour').subtract(d, 'hour').valueOf()));
        xScale.range([0, this.Width]);
        d3.select('.x-axis')
          .html('')
          .call(xAxis)
          .selectAll('text')
          .attr('font-weight', d => (this.$dayjs(d).tz(this.Analyzer.TimeZone).hour() == 0) ? 'bold' : 'normal');
        d3.select('.x-grid')
          .call(xAxis.tickFormat(d => '').tickSize(-this.Height))
          .selectAll('line')
          .attr('color', 'rgba(0, 0, 0, 0.2)');
        for(let i = 0; i < this.titles.length; i++) {
          let SerieData = [];
          for(let n = 0; n < this.data.length; n++) Max = Math.max(Max, Math.abs(this.data[n][i + 1]));
          Max = Max / 3.0;
          let yScale = d3.scaleLinear()
            .range([this.RowHeight - 1, 0])
            .domain([0, Max]);
          for(let n = 0; n < this.data.length; n++) {
            let Val = this.data[n][i + 1];
            if(Val > 0) {
              if(Val < (Max * 1)) {
                SerieData.push([this.data[n][0], 0, yScale(Val), '#FFFFFF']);
                SerieData.push([this.data[n][0], yScale(Val), this.RowHeight - yScale(Val), '#FFBFBF']);
              } else if(Val < (Max * 2)) {
                SerieData.push([this.data[n][0], 0, this.RowHeight, '#FFBFBF']);
                SerieData.push([this.data[n][0], yScale(Val- Max), this.RowHeight - yScale(Val - Max), '#FF8080']);
              } else {
                SerieData.push([this.data[n][0], 0, this.RowHeight, '#FF8080']);
                SerieData.push([this.data[n][0], yScale(Val - Max * 2.0), this.RowHeight - yScale(Val - Max * 2.0), '#FF4040']);
              }
            } else {
              Val = -Val;
              if(Val < (Max * 1)) {
                SerieData.push([this.data[n][0], this.RowHeight - yScale(Val), this.RowHeight, '#FFFFFF']);
                SerieData.push([this.data[n][0], 0, this.RowHeight - yScale(Val), '#BFBFFF']);
              } else if(Val < (Max * 2)) {
                SerieData.push([this.data[n][0], 0, this.RowHeight, '#BFBFFF']);
                SerieData.push([this.data[n][0], 0, this.RowHeight - yScale(Val - Max), '#8080FF']);
              } else {
                SerieData.push([this.data[n][0], 0, this.RowHeight, '#8080FF']);
                SerieData.push([this.data[n][0], 0, this.RowHeight - yScale(Val - Max * 2.0), '#4040FF']);
              }
            }
          }
          this.SVG.select('.points-' + i).selectAll('rect').data(SerieData).join(
            enter => enter.append('rect')
              .attr('x', d => xScale(d[0]))
              .attr('y', d => d[1])
              .attr('width', this.Width / 1440.0)
              .attr('height', d => d[2])
              .attr('fill', d => d[3])
              .attr('shape-rendering', 'crispEdges')
            ,
            update => update
              .attr('x', d => xScale(d[0]))
              .attr('y', d => d[1])
              .attr('width', this.Width / 1440.0)
              .attr('height', d => d[2])
              .attr('fill', d => d[3])
            ,
            exit => exit.remove()
          );
        }
        let Border = d3.select('.border').html('');
        for(let i = 0; i < this.titles.length; i++) {
          Border.append('line')
            .attr('x1', 0)
            .attr('x2', this.Width)
            .attr('y1', this.MarginTop + this.RowHeight * (1 + i) - 1)
            .attr('y2', this.MarginTop + this.RowHeight * (1 + i) - 1)
            .attr('stroke', 'black')
            .attr('stroke-width', 1)
            .attr('shape-rendering', 'crispEdges')
        }
        Border.append('line')
          .attr('x1', this.Width - 1)
          .attr('x2', this.Width - 1)
          .attr('y1', this.MarginTop)
          .attr('y2', this.Height)
          .attr('stroke', 'black')
          .attr('stroke-width', 1)
          .attr('shape-rendering', 'crispEdges');
        Border.append('line')
          .attr('x1', 0)
          .attr('x2', 0)
          .attr('y1', this.MarginTop)
          .attr('y2', this.Height)
          .attr('stroke', 'black')
          .attr('stroke-width', 1)
          .attr('shape-rendering', 'crispEdges');
      },



//        if((this.paused == true) || (this.Graph == null)) return;
//        var Opt = { file: data[this.title].Rows };
//        var Range = 1;
//        if(data[this.title].Rows.length == 0) {
//          this.Graph.updateOptions({ file: [[0,0]] });
//          return;
//        }
//        for(var n = 0; n < data[this.title].Rows.length; n++) Range = Math.max(Range, Math.abs(data[this.title].Rows[n][1]));
//        Opt.valueRange = [-Range, +Range];
//        Opt.dateWindow = [data[this.title].From, data[this.title].To];
//        this.Graph.updateOptions(Opt);




//      var Opt = { axes: { x: {}, y: {} } };
//      Opt.labels = ["Time", this.title];
//      Opt.colors = ["#d9534f"];
//      Opt.labelsDiv = this.$refs.legend;
//      Opt.plugins = [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ];
//      Opt.legendFormatter = (data) => {
//        if(data.x == null) {
//          var Value = '<span class="inactive-label">' + data.series[0].label + '</span>';
//          if(data.dygraph.user_attrs_.labels[0] != 'Time') Value = Value + '<br/><span class="inactive-value"> ' + data.dygraph.user_attrs_.labels[0] + '</span>';
//          return  Value;
//        } else {
//          var Caption = '<span class="active-label">' + data.series[0].label + '</span><br/>' + data.xHTML;
//          if(data.series[0].yHTML !== undefined) return Caption + '<div class="active-value">' + data.series[0].yHTML + '</div>';
//          return Caption + '<div class="active-value">-</div>';
//        }
//      };
//      Opt.axes.y.drawAxis = false;
//      Opt.axes.y.drawGrid = false;
//      Opt.axes.y.valueFormatter = (val) => { return this.format(val); };
//      Opt.axes.x.valueFormatter = (x) => { return '<div class="x-axis">' + moment.tz(x, this.Analyzer.TimeZone).format('HH:mm:ss') + '</div>'; };
//      this.$emit('init');



//      Plotter: function(e) {
//        var ctx = e.drawingContext;
//        var area = e.plotArea;
//        if(area.h <= 0 || area.w <= 0) return;
//        var points = e.points;
//        var BarWidth = Infinity;
//        for(var i = 1; i < points.length; i++) {
//          var Sep = points[i].canvasx - points[i - 1].canvasx;
//          if(Sep < BarWidth) BarWidth = Sep;
//        }
//        var ColorP = Dygraph.toRGB_(e.color);
//        ColorP = 'rgba(' + Math.floor(ColorP.r) + ',' + Math.floor(ColorP.g) + ',' + Math.floor(ColorP.b) + ', 0.3)';
//        var ColorN = Dygraph.toRGB_(e.color);
//        ColorN = 'rgba(' + Math.floor(255 - ColorN.r) + ',' + Math.floor(255 - ColorN.g) + ',' + Math.floor(255 - ColorN.b) + ', 0.3)';
//        var Height = area.h - 1;
//        var Midpoint = ( e.axis.minyval + e.axis.maxyval) / 2;
//        var Scale = (Height * 3) / (e.axis.maxyval - Midpoint);
//        for(var i = 0; i < points.length; i++) {
//          var BarHeight = (points[i].yval - Midpoint) * Scale;
//          if((BarHeight > (Height * 3)) || (BarHeight < (-Height * 3))) continue;
//          while(true) {
//            ctx.fillStyle = (BarHeight > 0) ? ColorP : ColorN;
//            if(BarHeight > area.h) {
//              ctx.fillRect(points[i].canvasx - (BarWidth / 2.0), 0, BarWidth, Height);
//              BarHeight -= area.h;
//            } else if(-BarHeight > area.h) {
//              ctx.fillRect(points[i].canvasx - (BarWidth / 2.0), 0, BarWidth, Height);
//              BarHeight += area.h;
//            } else {
//              if(BarHeight > 0) {
//                ctx.fillRect(points[i].canvasx - (BarWidth / 2.0), Height - BarHeight, BarWidth, BarHeight);
//                points[i].canvasy = area.h - BarHeight;
//              } else {
//                ctx.fillRect(points[i].canvasx - (BarWidth / 2.0), 0, BarWidth, -BarHeight);
//                points[i].canvasy = -BarHeight;
//              }
//              break;
//            }
//          }
//        }
//      }
    }
  }
</script>