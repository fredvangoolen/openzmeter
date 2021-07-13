<template>
  <div class="chart"></div>
</template>

<script>
  import Dygraph from 'JS/dygraph.js';
  import 'JS/dygraph-crosshair.js';
  import chroma from 'chroma-js';
  import * as oZm from 'JS/openZmeter.js';
  import Vue from 'vue';
  import moment from 'moment';
  const This = Vue.prototype;
  var PatternImg = new Image();
  PatternImg.onload = () => {};
  PatternImg.src = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAQAAAAECAIAAAAmkwkpAAAAJElEQVQImT2JsQ0AMAyDyP832q+QIWqZEJDEx6gA0JZr9/mmLvBPKENuQmcdAAAAAElFTkSuQmCC";
  function nullAction() {
  }
  function mouseDown(event, g, context) {
    context.initializeMouseDown(event, g, context);
    if(event.altKey || event.shiftKey) {
      Dygraph.startZoom(event, g, context);
    } else {
      Dygraph.startPan(event, g, context);
    }
  }
  function mouseMove(event, g, context) {
    context.is2DPan = false;
    if(context.isPanning) {
      Dygraph.movePan(event, g, context);
    } else if(context.isZooming) {
      Dygraph.moveZoom(event, g, context);
    }
  }
  function mouseUp(event, g, context) {
    if(context.isPanning) {
      Dygraph.endPan(event, g, context);
      if(context.regionWidth > 0) this.$emit('range', [Math.round(g.dateWindow_[0]), Math.round(g.dateWindow_[1])]);
    } else if (context.isZooming) {
      Dygraph.endZoom(event, g, context);
      if(context.regionWidth > 0) this.$emit('range', [Math.round(g.dateWindow_[0]), Math.round(g.dateWindow_[1])]);
    }
  }
  function mouseDblClick(event, g, context) {
    g.updateOptions({ dateWindow: null });
  }
  function mouseWheelZoom(event, g, context) {
    if(!event.shiftKey) return;
    var normal = event.detail ? event.detail * -1 : event.wheelDelta / 40;
    var percentage = normal / 50;
    if (!event.offsetX) event.offsetX = event.layerX - event.target.offsetLeft;
    var xOffset = g.toDomCoords(g.xAxisRange()[0], null)[0];
    var x = event.offsetX - xOffset;
    var w = g.toDomCoords(g.xAxisRange()[1], null)[0] - xOffset;
    var xPct = w == 0 ? 0 : (x / w);
    var Axis = g.xAxisRange();
    var Window = [Axis[0] + (Axis[1] - Axis[0]) * percentage * xPct, Axis[1] - (Axis[1] - Axis[0]) * percentage * (1 - xPct)];
    if((this.Fixed == true) && (g.file_.length > 0)) {
      if(Window[0] < g.file_[0][0]) Window[0] = g.file_[0][0];
      if(Window[1] > g.file_.slice(-1)[0][0]) Window[1] = g.file_.slice(-1)[0][0];
    } else  {
      clearInterval(context.MouseWheelTimer);
      context.MouseWheelTimer = setTimeout( () => {
        if(g.file_.length > 0) g.updateOptions({ file: [Array(g.file_[0].length).fill(0)] }, false);
        this.This.$emit('range', [Math.round(Window[0]), Math.round(Window[1])]);
      }, 500);
    }
    g.updateOptions({ dateWindow: Window });
  }
  function touchStart(event, g, context) {
    event.preventDefault();
    if(event.touches.length > 1) context.startTimeForDoubleTapMs = null;
    var touches = [];
    for(var i = 0; i < event.touches.length; i++) touches.push({ pageX: event.touches[i].pageX, dataX: g.toDataXCoord(event.touches[i].pageX) });
    context.initialTouches = touches;
    if(touches.length == 1) {
      context.initialPinchCenter = touches[0];
      context.touchDirections = { x: true };
    } else if(touches.length == 2) {
      context.initialPinchCenter = { pageX: 0.5 * (touches[0].pageX + touches[1].pageX), dataX: 0.5 * (touches[0].dataX + touches[1].dataX) };
    }
    context.initialRange = { x: g.xAxisRange() };
  }
  function touchMove(event, g, context) {
    context.startTimeForDoubleTapMs = null;
    var touches = [];
    for(var i = 0; i < event.touches.length; i++) {
      var t = event.touches[i];
      touches.push({ pageX: t.pageX });
    }
    var initialTouches = context.initialTouches;
    var c_now = (touches.length == 1) ? touches[0] : { pageX: 0.5 * (touches[0].pageX + touches[1].pageX) };
    var c_init = context.initialPinchCenter;
    var swipe = { pageX: c_now.pageX - c_init.pageX };
    var dataWidth = context.initialRange.x[1] - context.initialRange.x[0];
    swipe.dataX = (swipe.pageX / g.plotter_.area.w) * dataWidth;
    var xScale = Math.min(8, Math.max(0.125, (touches.length == 1) ? 1.0 : (touches[1].pageX - c_now.pageX) / (initialTouches[1].pageX - c_init.pageX)));
    g.dateWindow_ = [c_init.dataX - swipe.dataX + (context.initialRange.x[0] - c_init.dataX) / xScale, c_init.dataX - swipe.dataX + (context.initialRange.x[1] - c_init.dataX) / xScale];
    if(this.Fixed == true) {
      if(g.dateWindow_[0] < g.file_[0][0]) g.dateWindow_[0] = g.file_[0][0];
      if(g.dateWindow_[1] > g.file_.slice(-1)[0][0]) g.dateWindow_[1] = g.file_.slice(-1)[0][0];
    }
    g.drawGraph_(false);
  }
  function touchEnd(event, g, context) {
    if(event.touches.length != 0) {
      touchStart(event, g, context);
    } else if(event.changedTouches.length == 1) {
      var now = new Date().getTime();
      var t = event.changedTouches[0];
      if((context.startTimeForDoubleTapMs != null) && ((now - context.startTimeForDoubleTapMs) < 500) && (context.doubleTapX != null) && (Math.abs(context.doubleTapX - t.screenX) < 50) && (context.doubleTapY != null) && (Math.abs(context.doubleTapY - t.screenY) < 50)) {
        if(this.Fixed == true) g.updateOptions({ dateWindow: null });
      } else {
        context.startTimeForDoubleTapMs = now;
        context.doubleTapX = t.screenX;
        context.doubleTapY = t.screenY;
      }
    }
    if(this.Fixed == false) {
      if(g.file_.length > 0) g.updateOptions({ file: [Array(g.file_[0].length).fill(0)] }, false);
      this.This.$emit('range', [Math.round(g.dateWindow_[0]), Math.round(g.dateWindow_[1])]);
    }
  }
  export default {
    Tickers: {
      Default: function(a, b, pixels, opts, dygraph, vals) {
        return Dygraph.numericTicks(a, b, pixels, opts, dygraph, vals);
      },
      DBTicks: function(min, max, offset) {
        var ticks = [];
        for(var i = Math.floor(min - offset); i < Math.ceil(max - offset); i++) {
          if((i % 10) == 0) ticks.push({ v: i + offset, label: i });
        }
        return ticks;
      },
      OddHarmonicsTicks: function(analyzer) {
        var Ticks = [];
        for(var i = 0; i < analyzer.HarmonicsNames.length; i = i + 2) Ticks.push({ v: i, label: analyzer.HarmonicsNames[i] });
        return Ticks;
      },
      WaveTimeTicks: function(max, analyzer) {
        var ticks = [];
        for(var i = 0; i <= Math.floor(max / analyzer.SampleRate * analyzer.NominalFrequency); i++) ticks.push({ v: i * analyzer.SampleRate / analyzer.NominalFrequency, label: This.Round(i / analyzer.NominalFrequency * 1000, 2, false) });
        return ticks;
      }
    },
    Formatters: {
      FFTVal: function(analyzer, val) {
        var Div = (analyzer.NominalFrequency == 50) ? 10 : 12;
        return This.Round(val / Div * analyzer.NominalFrequency, 0, true) + ' Hz';
      },
      FFTAxis: function(analyzer, val) {
        var Div = (analyzer.NominalFrequency == 50) ? 10 : 12;
        val = Math.round(val / Div * analyzer.NominalFrequency);
        if(val >= 1000) {
          return (((val / 1000) % 1) != 0) ? "" : (val / 1000) + ' kHz';
        } else {
          return ((val % 1) != 0) ? "" : val + ' Hz';
        }
      },
      Legend_Follow: function(data) {
        if(data.x === undefined || data.x === null) return "";
        var Result = '<span class="dygraph-legend-title">' + data.xHTML + '</span><br>';
        for(var i = 0; i < data.series.length; i++)  {
          if((data.series[i].isVisible) && (data.series[i].yHTML != null)) {
            Result = Result + data.series[i].label + ' : ' + data.series[i].yHTML + '<br>';
          }
        }
        return Result;
      },
      X_HourlyFull: function(min, max, pixels, tz) {
        var ticks = [];
        if((tz == null) || (isNaN(pixels))) return ticks;
        var Min = moment.tz(min, tz).startOf('hour');
        var Max = moment.tz(max, tz).endOf('hour');
        var HoursDiff = moment.duration(Max.diff(Min)).asHours();
        var DaysDiff = moment.duration(Max.diff(Min)).asDays();
        if(HoursDiff < 48) {
          for(var h = Min; h.isBefore(Max); h.add(1, 'hour')) ticks.push({ v: h.valueOf(), label: (h.hours() == 0) ? '<strong>' + h.format('DD/MM') + '</strong>' : h.format('HH') + 'h' });
        } else {
          var Inc = Math.max(1, Math.trunc(DaysDiff / (pixels / 50)));
          var Min = Min.startOf('day');
          var Max = Max.endOf('day');
          for(var h = Min; h.isBefore(Max); h.add(Inc, 'day')) ticks.push({ v: h.valueOf(), label: h.format('DD/MM') });
        }
        return ticks;
      },
      X_Hourly: function(x, tz, quarters) {
        var Time = moment(x).tz(tz);
        if(quarters == true) {
          return Time.hours() + 'h (' + (Math.trunc(Time.minute() / 15) + 1) + 'Q) - ' + Time.format('DD/MM/YYYY');
        } else {
          return Time.hours() + 'h - ' + Time.format('DD/MM/YYYY');
        }
      },
      Y_UnitScale: function(x, units, dec, html) {
        if(dec == null) dec = 2;
        if(units == null) units = '';
        if(html == null) html = false;
        var Prefix = This.BestUnits(x);
        return This.Round(This.ScaleUnits(x, Prefix), dec, html) + Prefix + units;
      },
      Y_UnitScaleFull: function(x, gran, opts, g, units, dec, html) {
        if(dec == null) dec = 2;
        if(units == null) units = '';
        if(html == null) html = false;
        var Range = g.axes_[0].valueRange;
        if(Range == null) Range = g.axes_[0].computedValueRange;
        return (Range == null) ? '' : This.Round(This.ScaleUnits(x, This.BestUnits(Math.max(Math.abs(Range[0]), Math.abs(Range[1])))), dec, html) + units;
      }
    },
    Plugins: {
      Vertical_Crosshair: function() {
        return new Dygraph.Plugins.Crosshair( { direction: "vertical" } );
      }
    },
    Callbacks: {
      Underlay_DrawBorder: function(ctx, area, dygraph) {
        ctx.save();
        ctx.strokeRect(area.x, area.y, area.w, area.h);
        ctx.restore();
      },
      Underlay_DrawPeriods: function(canvas, area, g, data) {
        var DrawPeriod = (color, period, from, to) => {
          if((Color == null) || (period == null)) return;
          var Left = g.toDomXCoord(from);
          var Right = g.toDomXCoord(to);
          if(Left < (area.x + 1)) Left = (area.x + 1);
          if(Right > (area.w + area.x - 1)) Right = (area.w + area.x - 1);
          canvas.fillStyle = color;
          canvas.fillRect(Left, area.y + 1, Right - Left, 12);
          if((Right - Left) > 20) {
            canvas.fillStyle = '#000000';
            canvas.font = "10px 'Fira Code";
            canvas.fillText('P' + period, Left + 1, 10);
          }
        };
        canvas.save();
        var TimeCol = (data != null) ? data.Headers.indexOf('Time') : -1;
        var BillCol = (data != null) ? data.Headers.indexOf('Bill') : -1;
        var PeriodCol = (data != null) ? data.Headers.indexOf('Period') : -1;
        var ColorCol = (data != null) ? data.Headers.indexOf('Color') : -1;
        if(BillCol != -1) {
          canvas.strokeStyle = '#d9534f';
          canvas.lineWidth = 1.5;
          for(var i = 0; i < data.Rows.length; i++) {
            if(data.Rows[i][BillCol] == false) continue;
            var X = g.toDomXCoord(data.Rows[i][TimeCol]);
            canvas.beginPath();
            canvas.moveTo(X, 13);
            canvas.lineTo(X, area.h);
            canvas.stroke();
          }
        }
        if(PeriodCol != -1) {
          var EmptyPattern = canvas.createPattern(PatternImg, "repeat");
          var Color = null;
          var From = null;
          var Period = null;
          canvas.strokeStyle = '#000';
          canvas.fillStyle = EmptyPattern;
          canvas.fillRect(area.x, area.y, area.w, 13);
          for(var i = 0; i < data.Rows.length; i++) {
            if((Color != data.Rows[i][ColorCol]) || (Period != data.Rows[i][PeriodCol])) {
              DrawPeriod(Color, Period, From, data.Rows[i][TimeCol] - 900000);
              Color = data.Rows[i][ColorCol];
              Period = data.Rows[i][PeriodCol];
              From = data.Rows[i][TimeCol] - 899999;
            }
          }
          if(data.Rows.length > 1) DrawPeriod(Color, Period, From, data.Rows.slice(-1)[0][TimeCol]);
        }
        canvas.strokeStyle = '#000';
        canvas.strokeRect(area.x, area.y, area.w, area.h);
        canvas.lineWidth = 0.25;
        canvas.beginPath();
        canvas.moveTo(area.x, 13.5);
        canvas.lineTo(area.x + area.w, 13.5);
        canvas.stroke();
        canvas.restore();
        canvas.rect(area.x + 1, area.y + 14, area.w - 2, area.h - 15);
        canvas.clip();
      },
      DrawHighlightPoint_HighlightBar: function(g, seriesName, canvasContext, cx, cy, color, pointSize) {
        var serieIndex = g.user_attrs_.labels.indexOf(seriesName) - 1;
        var VisibleSerie = serieIndex;
        for(var n = 0; n < serieIndex; n++) {
          if(g.user_attrs_.visibility[n] == false) VisibleSerie--;
        }
        if(g.UndoHighlight.cx != null && g.UndoHighlight.cx[serieIndex] != null) {
          var x_left = g.UndoHighlight.cx[serieIndex] - (g.subbar_width * g.SeriesCount / 2) + VisibleSerie * g.subbar_width;
          canvasContext.fillRect(x_left, g.UndoHighlight.cy[serieIndex], g.subbar_width, g.y_bottom - g.UndoHighlight.cy[serieIndex]);
        }
        g.UndoHighlight.cx[serieIndex] = cx;
        g.UndoHighlight.cy[serieIndex] = cy;
        canvasContext.fillStyle = chroma(color).brighten(2);
        var x_left = cx - (g.subbar_width * g.SeriesCount / 2) + VisibleSerie * g.subbar_width;
        canvasContext.fillRect(x_left, cy, g.subbar_width, g.y_bottom - cy);
      }
    },
    Plotters: {
      Bar: function(e) {
        var ctx = e.drawingContext;
        var points = e.points;
        var y_bottom = e.dygraph.toDomYCoord(0);
        ctx.fillStyle = e.color;
        var min_sep = Infinity;
        for (var i = 1; i < points.length; i++) {
          var sep = points[i].canvasx - points[i - 1].canvasx;
          if (sep < min_sep) min_sep = sep;
        }
        var bar_width = Math.floor(2.5 / 3 * min_sep);
        for (var i = 0; i < points.length; i++) {
          var p = points[i];
          var center_x = p.canvasx;
          ctx.fillRect(center_x, p.canvasy, bar_width, y_bottom - p.canvasy);
          ctx.strokeRect(center_x, p.canvasy, bar_width, y_bottom - p.canvasy);
        }
      },
      MultiBar: function(e) {
        if(e.seriesIndex != 0) return;
        var g = e.dygraph;
        g.UndoHighlight = { cx: [], cy: [] };
        var ctx = e.drawingContext;
        var sets = e.allSeriesPoints;
        g.SeriesCount = sets.length;
        var y_bottom = e.dygraph.toDomYCoord(0);
        g.y_bottom = y_bottom;
        var min_sep = Infinity;
        for(var j = 0; j < sets.length; j++) {
          for(var i = 1; i < sets[j].length; i++) min_sep = Math.min(sets[j][i].canvasx - sets[j][i - 1].canvasx, min_sep);
        }
        var bar_width = Math.floor(2.5 / 3 * min_sep);
        g.subbar_width = bar_width / sets.length;
        if(g.subbar_width < 2) g.subbar_width = 2;
        for(var j = 0; j < sets.length; j++) {
          ctx.strokeStyle = g.getColors()[j];
          ctx.fillStyle = g.getColors()[j];
          for(var i = 0; i < sets[j].length; i++) {
            var p = sets[j][i];
            var x_left = p.canvasx - (g.subbar_width * sets.length / 2) + j * g.subbar_width;
            ctx.fillRect(x_left, p.canvasy, g.subbar_width, y_bottom - p.canvasy);
          }
        }
      },
      BarOffset: function(e, offset) {
        var ctx = e.drawingContext;
        var points = e.points;
        var y_bottom = e.dygraph.toDomYCoord(0);
        var g = e.dygraph;
        g.UndoHighlight = { cx: [], cy: [] };
        g.SeriesCount = 1;
        g.y_bottom = e.dygraph.toDomYCoord(0);
        g.subbar_width = Math.max(1.0, e.dygraph.toDomXCoord(offset) - e.dygraph.toDomXCoord(0));
        ctx.strokeStyle = e.color;
        ctx.fillStyle = e.color;
        for(var j = 0; j < points.length; j++) {
          var p = points[j];
          if(isNaN(p.y)) {
            p.canvasx = null;
            p.canvasy = null;
            continue;
          }
          p.canvasx = p.canvasx - (g.subbar_width / 2.0);
          ctx.fillRect(p.canvasx - (g.subbar_width / 2.0), p.canvasy, g.subbar_width, y_bottom - p.canvasy);
        }
      },
      LineOffset: function(e, offset) {
        var ctx = e.drawingContext;
        var points = e.points;
        var y_bottom = e.dygraph.toDomYCoord(0);
        var g = e.dygraph;
        g.UndoHighlight = { cx: [], cy: [] };
        g.SeriesCount = 1;
        g.y_bottom = e.dygraph.toDomYCoord(0);
        g.subbar_width = Math.max(1.0, e.dygraph.toDomXCoord(offset) - e.dygraph.toDomXCoord(0));
        ctx.strokeStyle = e.color;
        if(e.seriesIndex == 0) {
          ctx.fillStyle = chroma(e.color).alpha(0.2);
          ctx.beginPath();
          var prevX = null;
          for(var j = 0; j < points.length; j++) {
            var p = points[j];
            if(isNaN(p.y)) {
              p.canvasx = null;
              p.canvasy = null;
              continue;
            }
            if(prevX == null) {
              ctx.moveTo(e.dygraph.toDomXCoord(p.xval - offset), g.y_bottom);
              ctx.lineTo(e.dygraph.toDomXCoord(p.xval - offset), p.canvasy);
            } else {
              ctx.lineTo(prevX, p.canvasy);
            }
            ctx.lineTo(p.canvasx, p.canvasy);
            prevX = p.canvasx;
          }
          if(prevX != null) ctx.lineTo(prevX, g.y_bottom);
          ctx.fill();
        }
        var prevX = null;
        ctx.beginPath();
        for(var j = 0; j < points.length; j++) {
          var p = points[j];
          if(isNaN(p.y)) {
            p.canvasx = null;
            p.canvasy = null;
            continue;
          }
          if(prevX == null) {
            ctx.moveTo(e.dygraph.toDomXCoord(p.xval - offset), p.canvasy);
          } else {
            ctx.lineTo(prevX, p.canvasy);
          }
          ctx.lineTo(p.canvasx, p.canvasy);
          prevX = p.canvasx;
          p.canvasx = p.canvasx - (g.subbar_width / 2.0);
        }
        ctx.stroke();
      }
    },
    InteractionModels: {
      Scrollable: function(parent) {
        return { mousedown: mouseDown.bind(parent),
                 mousemove: mouseMove.bind(parent),
                 mouseup: mouseUp.bind(parent),
                 dblclick: nullAction.bind(parent),
                 mousewheel: mouseWheelZoom.bind({ This: parent, Fixed: false }),
                 touchstart: touchStart.bind(parent),
                 touchmove: touchMove.bind({ This: parent, Fixed: false }),
                 touchend: touchEnd.bind({ This: parent, Fixed: false })
        };
      },
      Fixed: function(parent) {
        return { mousedown: mouseDown.bind(parent),
                 mousemove: mouseMove.bind(parent),
                 mouseup: mouseUp.bind(parent),
                 dblclick: mouseDblClick.bind(parent),
                 mousewheel : mouseWheelZoom.bind({ This: parent, Fixed: true }),
                 touchstart: touchStart.bind(parent),
                 touchmove: touchMove.bind({ This: parent, Fixed: true }),
                 touchend: touchEnd.bind({ This: parent, Fixed: true })
        };
      },
    },
    props: { options: Object, height: Number, width: Number, paused: Boolean, window: Array, analyzer: null },
    data: function() {
      return { };
    },
    watch: {
      paused: function(val) {
        if((val == false) && (this.PausedOptions != null) && (this.Graph != null)) {
          if(this.PausedOptions.labels == null) this.PausedOptions.labels = ['A', 'B'];
          if(this.PausedOptions.file == null) this.PausedOptions.file = [];
          if(this.PausedOptions.file.length == 0) this.PausedOptions.file.push(Array(val.labels.length).fill(0));
          this.Graph.updateOptions({ showRangeSelector: true}, false);
          this.PausedOptions.showRangeSelector = false;
          this.Graph.updateOptions(this.PausedOptions);
          this.PausedOptions = null;
        }
      },
      options: function(val, prev) {
        if(val.labels == null) val.labels = ['A', 'B'];
        if(val.file == null) val.file = [];
        if(val.file.length == 0) val.file.push(new Array(val.labels.length).fill(0));
        if((this.Graph == null) || ((this.paused == true) && (this.Graph.file_.length > 1))) {
          this.PausedOptions = val;
          return;
        }
        this.PausedOptions = null;
        this.Graph.updateOptions({ showRangeSelector: true}, false);
        val.showRangeSelector = false;
        this.Graph.updateOptions(val);
      },
      height: function(val) {
        if(this.Graph == null) return;
        $(this.Graph.graphDiv).parent().removeAttr('style');
        this.Graph.resize((this.width == null) ? '100%' : this.width, val);
        this.Graph.resize();
      },
      width: function(val) {
        if(this.Graph == null) return;
        $(this.Graph.graphDiv).parent().removeAttr('style');
        this.Graph.resize((this.width == null) ? '100%' : this.width, this.height);
        this.Graph.resize();
      }
    },
    created: function() {
      this.Graph = null;
      this.PausedOptions = null;
    },
    mounted: function() {
      var Data = [];
      if(this.options.labels == null) this.options.labels = ['A', 'B'];
      if(this.options.file == null) this.options.file = [];
      if(this.options.file.length == 0) this.options.file.push(new Array(this.options.labels.length).fill(0));
      this.Graph = new Dygraph(this.$el, this.options.file, this.options);
      this.$nextTick(() => {
        $(this.Graph.graphDiv).parent().removeAttr('style');
        this.Graph.resize((this.width == null) ? '100%' : this.width, this.height);
        this.Graph.resize();
      });
    },
    beforeDestroy: function() {
      if(this.Graph != null) this.Graph.destroy();
    }
  }
</script>

<style scoped>
  .chart {
    width: 100%;
    user-select: none;
    cursor: crosshair;
    display: inline-block;
  }
  .chart >>> .dygraph-legend-title {
    color: #FFF;
    font-size: 10px;
    font-weight: bold;
  }
  .chart >>> .dygraph-axis-label {
    font-size: 11px;
    color: black;
    line-height: 10px;
  }
  .chart >>> .dygraph-legend {
    position: absolute;
    color: #FFF !important;
    font-size: 11px;
    z-index: 10;
    width: 160px;
    background-color: #474543 !important;
    border: 1px solid #212121;
    border-radius: 3px;
    line-height: 13px;
    text-align: center;
    overflow: hidden;
    pointer-events: none;
  }
  .chart >>> .dygraph-legend-line {
    display: inline-block;
    position: relative;
    bottom: .5ex;
    padding-left: 1em;
    height: 1px;
    border-bottom-width: 2px;
    border-bottom-style: solid;
  }
  .chart >>> .dygraph-legend-dash {
    display: inline-block;
    position: relative;
    bottom: .5ex;
    height: 1px;
    border-bottom-width: 2px;
    border-bottom-style: solid;
  }
</style>