<template>
  <div class="chart"></div>
</template>

<script>
  import Dygraph from 'JS/dygraph.js';
  import chroma from 'chroma-js';
  import * as oZm from 'JS/openZmeter.js';
  import Vue from 'vue';
  const This = Vue.prototype;
  var PatternImg = new Image();
  PatternImg.onload = () => {};
  PatternImg.src = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAQAAAAECAIAAAAmkwkpAAAAJElEQVQImT2JsQ0AMAyDyP832q+QIWqZEJDEx6gA0JZr9/mmLvBPKENuQmcdAAAAAElFTkSuQmCC";


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

  function mouseWheelZoom(event, g, context) {
    if(!event.shiftKey) return;
//    if(this.StopAutoupdate) this.StopAutoupdate();
    var normal = event.detail ? event.detail * -1 : event.wheelDelta / 40;
    var percentage = normal / 50;
    if (!event.offsetX) event.offsetX = event.layerX - event.target.offsetLeft;
    var xOffset = g.toDomCoords(g.xAxisRange()[0], null)[0];
    var x = event.offsetX - xOffset;
    var w = g.toDomCoords(g.xAxisRange()[1], null)[0] - xOffset;
    var xPct = w == 0 ? 0 : (x / w);
    var Axis = g.xAxisRange();
    g.updateOptions({ dateWindow: [Axis[0] + (Axis[1] - Axis[0]) * percentage * xPct, Axis[1] - (Axis[1] - Axis[0]) * percentage * (1 - xPct)] });
    event.preventDefault();
    event.stopPropagation();
    clearInterval(context.MouseWheelTimer);
    context.MouseWheelTimer = setTimeout( function() { this.WindowChange(Axis[0] + (Axis[1] - Axis[0]) * percentage * xPct, Axis[1] - (Axis[1] - Axis[0]) * percentage * (1 - xPct)); }.bind(this), 500);
  }
  function MouseDblClick(event, g, context) {
    g.updateOptions({
    dateWindow: null
  });
  //  this.WindowRestore();
  }
  function touchStart(event, g, context) {
    if(this && this.StopAutoupdate) this.StopAutoupdate();
    event.preventDefault();  // touch browsers are all nice.
    if(event.touches.length > 1) context.startTimeForDoubleTapMs = null;
    var touches = [];
    for(var i = 0; i < event.touches.length; i++) {
      var t = event.touches[i];
      touches.push({ pageX: t.pageX, dataX: g.toDataXCoord(t.pageX) });
    }
    context.initialTouches = touches;
    if(touches.length === 1) {
      context.initialPinchCenter = touches[0];
      context.touchDirections = { x: true };
    } else if(touches.length >= 2) {
      context.initialPinchCenter = { pageX: 0.5 * (touches[0].pageX + touches[1].pageX), dataX: 0.5 * (touches[0].dataX + touches[1].dataX) };
//      var initialAngle = 180 / Math.PI * Math.atan2(context.initialPinchCenter.pageY - touches[0].pageY, touches[0].pageX - context.initialPinchCenter.pageX);
//      initialAngle = Math.abs(initialAngle);
//      if(initialAngle > 90) initialAngle = 90 - initialAngle;
//      context.touchDirections = { x: (initialAngle < (90 - 45/2)), y: (initialAngle > 45/2) };
    }
    context.initialRange = { x: g.xAxisRange() };
  }
  function touchMove(event, g, context) {
    context.startTimeForDoubleTapMs = null;
    var i, touches = [];
    for (i = 0; i < event.touches.length; i++) {
      var t = event.touches[i];
      touches.push({ pageX: t.pageX });
    }
    var initialTouches = context.initialTouches;
    var c_now;
    var c_init = context.initialPinchCenter;
    if (touches.length == 1) {
      c_now = touches[0];
    } else {
      c_now = { pageX: 0.5 * (touches[0].pageX + touches[1].pageX) };
    }
    var swipe = { pageX: c_now.pageX - c_init.pageX };
      var dataWidth = context.initialRange.x[1] - context.initialRange.x[0];
//      var dataHeight = context.initialRange.y[0] - context.initialRange.y[1];
      swipe.dataX = (swipe.pageX / g.plotter_.area.w) * dataWidth;
//      swipe.dataY = (swipe.pageY / g.plotter_.area.h) * dataHeight;
      var xScale/*, yScale*/;

      // The residual bits are usually split into scale & rotate bits, but we split
      // them into x-scale and y-scale bits.
      if (touches.length == 1) {
        xScale = 1.0;
//        yScale = 1.0;
      } else if (touches.length >= 2) {
        var initHalfWidth = (initialTouches[1].pageX - c_init.pageX);
        xScale = (touches[1].pageX - c_now.pageX) / initHalfWidth;

//        var initHalfHeight = (initialTouches[1].pageY - c_init.pageY);
//        yScale = (touches[1].pageY - c_now.pageY) / initHalfHeight;
      }

      // Clip scaling to [1/8, 8] to prevent too much blowup.
      xScale = Math.min(8, Math.max(0.125, xScale));
//      yScale = Math.min(8, Math.max(0.125, yScale));

      var didZoom = false;
//      if (context.touchDirections.x) {
        g.dateWindow_ = [
          c_init.dataX - swipe.dataX + (context.initialRange.x[0] - c_init.dataX) / xScale,
          c_init.dataX - swipe.dataX + (context.initialRange.x[1] - c_init.dataX) / xScale
        ];
        didZoom = true;
//      }

//      if (context.touchDirections.y) {
//        for (i = 0; i < 1  /*g.axes_.length*/; i++) {
//          var axis = g.axes_[i];
//          var logscale = g.attributes_.getForAxis("logscale", i);
//          if (logscale) {
//            // TODO(danvk): implement
//          } else {
//            axis.valueRange = [
//              c_init.dataY - swipe.dataY + (context.initialRange.y[0] - c_init.dataY) / yScale,
//              c_init.dataY - swipe.dataY + (context.initialRange.y[1] - c_init.dataY) / yScale
//            ];
//            didZoom = true;
//          }
//        }
//      }

      g.drawGraph_(false);

      // We only call zoomCallback on zooms, not pans, to mirror desktop behavior.
      if (didZoom && touches.length > 1 && g.getFunctionOption('zoomCallback')) {
        var viewWindow = g.xAxisRange();
        g.getFunctionOption("zoomCallback").call(g, viewWindow[0], viewWindow[1], g.yAxisRanges());
      }
    }
  function touchEnd(event, g, context) {
      if (event.touches.length !== 0) {
        // this is effectively a "reset"
        touchStart(event, g, context);
      } else if (event.changedTouches.length == 1) {
          // Could be part of a "double tap"
          // The heuristic here is that it's a double-tap if the two touchend events
          // occur within 500ms and within a 50x50 pixel box.
          var now = new Date().getTime();
          var t = event.changedTouches[0];
          if (context.startTimeForDoubleTapMs &&
              now - context.startTimeForDoubleTapMs < 500 &&
              context.doubleTapX && Math.abs(context.doubleTapX - t.screenX) < 50 &&
              context.doubleTapY && Math.abs(context.doubleTapY - t.screenY) < 50) {
            g.resetZoom();
          } else {
            context.startTimeForDoubleTapMs = now;
            context.doubleTapX = t.screenX;
//            context.doubleTapY = t.screenY;
          }
      }
      this.WindowChange(g.dateWindow_[0], g.dateWindow_[1]);
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
      Legend_Follow: function(data) {
        if(data.x === undefined || data.x === null) return "";
        var Result = '<span class="dygraph-legend-title">' + data.xHTML + '</span><br>';
        for(var i = 0; i < data.series.length; i++)  {
          if(data.series[i].isVisible) {
            Result = Result + data.series[i].label + ' : ' + data.series[i].yHTML + '<br>';
          }
        }
        return Result;
      },
      Y_UnitScale: function(x, units, dec, html) {
        if(dec == null) dec = 2;
        if(units == null) units = '';
        if(html == null) html = false;
        return This.Round(This.ScaleUnits(x, This.BestUnits(x)), dec, html) + units;
      },
      Y_UnitScaleFull: function(x, gran, opts, g, units, dec, html) {
        if(dec == null) dec = 2;
        if(units == null) units = '';
        if(html == null) html = false;
        return (g.axes_[0].valueRange == null) ? '' : This.Round(This.ScaleUnits(x, This.BestUnits(Math.max(Math.abs(g.axes_[0].valueRange[0]), Math.abs(g.axes_[0].valueRange[1])))), dec, html) + units;
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
      Underlay_DrawPeriods: function(canvas, area, g, decoration) {
        if(decoration == null) return;
        canvas.save();
        var Disp = (decoration.Aggreg == '15M') ? 450000 : 1800000;
        canvas.strokeStyle = '#000';
        var EmptyPattern = canvas.createPattern(PatternImg, "repeat");
        canvas.fillStyle = EmptyPattern;
        canvas.fillRect(area.x, area.y, area.w, 13);
        for(var i = 0; i < decoration.Periods.length; i++) {
          if(decoration.Periods[i].Color == null) continue;
          var Left = g.toDomXCoord(decoration.Periods[i].From + Disp);
          var Right = g.toDomXCoord(decoration.Periods[i].To + Disp);
          if(Left < (area.x + 1)) Left = (area.x + 1);
          if(Right > (area.w + area.x - 1)) Right = (area.w + area.x - 1);
          canvas.fillStyle = decoration.Periods[i].Color;
          canvas.fillRect(Left, area.y + 1, Right - Left, 12);
          if(((Right - Left) > 20) && (decoration.Periods[i].Name !== null)) {
            canvas.fillStyle = '#000000';
            canvas.font = "10px monospace";
            canvas.fillText(decoration.Periods[i].Name, Left + 1, 10);
          }
        }
        for(var i = 0; i < decoration.Bills.length; i++) {
          canvas.strokeStyle = '#d9534f';
          canvas.lineWidth = 2;
          canvas.beginPath();
          var X = g.toDomXCoord(decoration.Bills[i].To + Disp);
          canvas.moveTo(X, 13);
          canvas.lineTo(X, area.h);
          canvas.stroke();
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
        if(g.UndoHighlight.cx != undefined && g.UndoHighlight.cx[serieIndex] != undefined) {
          var x_left = g.UndoHighlight.cx[serieIndex] - (g.subbar_width * g.SeriesCount / 2) + VisibleSerie * g.subbar_width;
          canvasContext.fillRect(x_left, g.UndoHighlight.cy[serieIndex], g.subbar_width, g.y_bottom - g.UndoHighlight.cy[serieIndex]);
        }
        g.UndoHighlight.cx[serieIndex]=cx;
        g.UndoHighlight.cy[serieIndex]=cy;
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
      }
    },
    InteractionModels: {
      Scrollable: function(parent) {
        return { mousedown: mouseDown.bind(parent),
                 mousemove: mouseMove.bind(parent),
                 mouseup: mouseUp.bind(parent),
                 /*dblclick: MouseDblClick,
                 mousewheel: mouseWheelZoom,
                 touchstart: touchStart,
                 touchmove: touchMove,
                 touchend: touchEnd*/
        };
      },
      Fixed: function(parent) {
        return { mousedown: mouseDown.bind(parent),
                 mousemove: mouseMove.bind(parent),
                 mouseup: mouseUp.bind(parent),
/*                 dblclick: MouseDblClick,
                 mousewheel : mouseWheelZoom,
                 touchstart: touchStart,
                 touchmove: touchMove,
                 touchend: touchEnd*/
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