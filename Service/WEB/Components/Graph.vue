<template>
  <div class="chart"></div>
</template>

<script>
  import Dygraph from 'JS/dygraph.js';
  import chroma from 'JS/chroma.js';
  import * as oZm from 'JS/openZmeter.js';
  function mouseDown(event, g, context) {
    if(this.StopAutoupdate) this.StopAutoupdate();
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
      if(context.regionWidth > 0) this.WindowChange(g.dateWindow_[0], g.dateWindow_[1]);
    } else if (context.isZooming) {
      Dygraph.endZoom(event, g, context);
      if(context.regionWidth > 0) this.WindowChange(g.dateWindow_[0], g.dateWindow_[1]);
    }
  }
  function mouseWheelZoom(event, g, context) {
    if(!event.shiftKey) return;
    if(this.StopAutoupdate) this.StopAutoupdate();
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
    this.WindowRestore();
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
    WaveTimeTicks: function(max, analyzer) {
      var ticks = [];
      for(var i = 0; i <= Math.floor(max / analyzer.SampleRate * analyzer.NominalFreq); i++) {
        ticks.push({ v: i * analyzer.SampleRate / analyzer.NominalFreq, label: oZm.Round2(i / analyzer.NominalFreq * 1000) });
      }
      return ticks;
    },
    DBTicks: function(min, max, offset) {
      var ticks = [];
      for(var i = Math.floor(min - offset); i < Math.ceil(max - offset); i++) {
        if((i % 10) === 0) ticks.push({ v: i + offset, label: i });
      }
      return ticks;
    },
    OddHarmonicsTicks: function () {
      var Ticks = [];
      for(var i = 0; i < oZm.Series['HARMONICS'].Names.length; i = i + 2) Ticks.push({ v: i, label: oZm.Series['HARMONICS'].Names[i] });
      return Ticks;
    },
    multiColumnBarPlotter: function(e) {
      if(e.seriesIndex !== 0) return;
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
    LegendFormatter_Follow: function(data) {
      if(data.x === undefined || data.x === null) return "";
      var Result = '<span class="dygraph-legend-title">' + data.xHTML + '</span><br>';
      for(var i = 0; i < data.series.length; i++)  {
        if(data.series[i].isVisible) {
          Result = Result + data.series[i].label + ' : ' + data.series[i].yHTML + '<br>';
        }
      }
      return Result;
    },
    //interactions models
    FixedRangeInteractionModel: function(parent) {
      return { mousedown: mouseDown, mousemove: mouseMove, mouseup: mouseUp.bind(parent), dblclick: MouseDblClick.bind(parent), mousewheel : mouseWheelZoom.bind(parent),
                touchstart: touchStart, touchmove: touchMove, touchend: touchEnd.bind(parent) };
    },
    ScrollableInteractionModel: function(parent) {
      return { mousedown: mouseDown.bind(parent), mousemove: mouseMove, mouseup: mouseUp.bind(parent), dblclick: MouseDblClick.bind(parent), mousewheel : mouseWheelZoom.bind(parent),
                touchstart: touchStart.bind(parent), touchmove: touchMove, touchend: touchEnd.bind(parent) };
    },
    MouseUpEvt: function(event, g, contex) {
        oZm.mouseUp(event, g, contex);
        if((contex.isPanning) && (contex.regionWidth > 0)) this.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
    },
    DrawHighlightBarChartCallback: function(g, seriesName, canvasContext, cx, cy, color, pointSize) {
      var serieIndex = g.user_attrs_.labels.indexOf(seriesName) - 1;
      var VisibleSerie = serieIndex;
      for(var n = 0; n < serieIndex; n++) {
        if(g.user_attrs_.visibility[n] === false) VisibleSerie--;
      }
      if(g.UndoHighlight.cx !== undefined && g.UndoHighlight.cx[serieIndex] !== undefined) {
        var x_left = g.UndoHighlight.cx[serieIndex] - (g.subbar_width * g.SeriesCount / 2) + VisibleSerie * g.subbar_width;
        canvasContext.fillRect(x_left, g.UndoHighlight.cy[serieIndex], g.subbar_width, g.y_bottom - g.UndoHighlight.cy[serieIndex]);
      }
      g.UndoHighlight.cx[serieIndex]=cx;
      g.UndoHighlight.cy[serieIndex]=cy;
      canvasContext.fillStyle = chroma(color).brighten(2);
      
      var x_left = cx - (g.subbar_width * g.SeriesCount / 2) + VisibleSerie * g.subbar_width;
      canvasContext.fillRect(x_left, cy, g.subbar_width, g.y_bottom - cy);
    },
    props: ['options'],
    data() {
      return {};
    },
    methods: {
      SetOptions: function(options) {
        this.Options = Object.assign({}, this.Options, options);
        if(this.Graph === null) return;
        if(this.AllowUpdate == true) {
          this.Graph.updateOptions(this.Options);
        } else {
          this.OptionsChanged = true;
        }
      },
      SetData: function(data) {
        this.Options = Object.assign({}, this.Options, { file: data });
        if(this.Graph === null) return;
        if(this.AllowUpdate == true) {
          this.Graph.updateOptions(this.Options);
        } else {
          this.OptionsChanged = true;
        }
      },
      Redraw: function(w, h) {
        $(this.Graph.graphDiv).parent().removeAttr('style');
        if((w !== undefined) && (h !== undefined)) this.Graph.resize(w, h);
        this.Graph.resize();
      },
      HoverIn() {
        this.AllowUpdate = false;
        this.$emit('mouseOver', true);
      },
      HoverOut() {
        this.AllowUpdate = true;
        this.$emit('mouseOver', false);
        if(this.OptionsChanged == true) {
          this.Graph.updateOptions(this.options);
          this.OptionsChanged = false;
        }
      },
    },
    created: function() {
      this.Graph = null;
      this.AllowUpdate = true;
      this.Options = Object.assign({ highlightCallback: this.HoverIn, unhighlightCallback: this.HoverOut, strokeWidth: 0.5, gridLineColor: "rgba(0, 0, 0, 0.1)", gridLinePattern: [4, 4], labels: ['Index', 'Value'], interactionModel: {} }, this.options);
    },
    mounted: function() {
      var Data = [[0, 0]];
      if(this.Options.labels !== undefined) Data = [new Array(this.Options.labels.length).fill(0.0)];
      this.Graph = new Dygraph(this.$el, Data, this.Options);
    },
    beforeDestroy: function() {
      this.Graph.destroy();
    }
  }
  
</script>
