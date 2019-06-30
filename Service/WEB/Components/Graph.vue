<template>
  <div class="chart"></div>
</template>

<script>
  import Dygraph from 'JS/dygraph.js';
  import chroma from 'JS/chroma.js';
  import * as oZm from 'JS/openZmeter.js';
  var lastClicked = null;
  export default {
    WaveTimeTicks: function(max, analyzer) {
      var ticks = [];
      for(var i = 0; i <= Math.floor(max / analyzer.SampleFreq * analyzer.NominalFreq); i++) {
        ticks.push({ v: i * analyzer.SampleFreq / analyzer.NominalFreq, label: oZm.Round2(i / analyzer.NominalFreq * 1000) });
      }
      return ticks;
    },
    multiColumnBarPlotter: function(e) {
      if(e.seriesIndex !== 0) return;
      var g = e.dygraph;
      var ctx = e.drawingContext;
      var sets = e.allSeriesPoints;
      var y_bottom = e.dygraph.toDomYCoord(0);
      g.y_bottom=y_bottom;
      var min_sep = Infinity;
      for(var j = 0; j < sets.length; j++) {
        for(var i = 1; i < sets[j].length; i++) min_sep = Math.min(sets[j][i].canvasx - sets[j][i - 1].canvasx, min_sep);
      }
      var bar_width = Math.floor(2.5 / 3 * min_sep);
      g.bar_width=bar_width;
      var subbar_width = bar_width / sets.length;
      g.subbar_width=subbar_width;
      for(var j = 0; j < sets.length; j++) {
        ctx.strokeStyle = g.getColors()[j];
        ctx.fillStyle = g.getColors()[j];
        for(var i = 0; i < sets[j].length; i++) {
          var p = sets[j][i];
          var x_left = p.canvasx - (subbar_width * sets.length / 2) + j * subbar_width;
          ctx.fillRect(x_left, p.canvasy, bar_width / sets.length, y_bottom - p.canvasy);
          ctx.strokeRect(x_left, p.canvasy, bar_width / sets.length, y_bottom - p.canvasy);
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
    DBTicks: function(min, max, offset) {
      var ticks = [];
      for(var i = Math.floor(min - offset); i < Math.ceil(max - offset); i++) {
        if((i % 10) === 0) ticks.push({ v: i + offset, label: i });
      }
      return ticks;
    },
    OddHarmonics: function () {
      var Ticks = [];
      for(var i = 0; i < oZm.Series['HARMONICS'].Names.length; i = i + 2) Ticks.push({ v: i, label: oZm.Series['HARMONICS'].Names[i] });
      return Ticks;
    },
    offsetToPercentage: function(g, offsetX, offsetY) {
      // This is calculating the pixel offset of the leftmost date.
      var xOffset = g.toDomCoords(g.xAxisRange()[0], null)[0];
      var yar0 = g.yAxisRange(0);

      // This is calculating the pixel of the higest value. (Top pixel)
      var yOffset = g.toDomCoords(null, yar0[1])[1];

      // x y w and h are relative to the corner of the drawing area,
      // so that the upper corner of the drawing area is (0, 0).
      var x = offsetX - xOffset;
      var y = offsetY - yOffset;

      // This is computing the rightmost pixel, effectively defining the
      // width.
      var w = g.toDomCoords(g.xAxisRange()[1], null)[0] - xOffset;

      // This is computing the lowest pixel, effectively defining the height.
      var h = g.toDomCoords(null, yar0[0])[1] - yOffset;

      // Percentage from the left.
      var xPct = w == 0 ? 0 : (x / w);
      // Percentage from the top.
      var yPct = h == 0 ? 0 : (y / h);

      // The (1-) part below changes it from "% distance down from the top"
      // to "% distance up from the bottom".
      return [xPct, (1-yPct)];
    },
    mouseDown: function(event, g, context) {
      context.initializeMouseDown(event, g, context);
      lastClicked = g;
      if (event.altKey || event.shiftKey) {
        Dygraph.startZoom(event, g, context);
      } else {
        Dygraph.startPan(event, g, context);
      }
    },
    mouseMove: function(event, g, context) {
      context.is2DPan = false;
      if (context.isPanning) {
        Dygraph.movePan(event, g, context);
      } else if (context.isZooming) {
        Dygraph.moveZoom(event, g, context);
      }
    },
    startTouch: function(event, g, context) {
      event.preventDefault(); // touch browsers are all nice.
      if (event.touches.length > 1) {
        // If the user ever puts two fingers down, it's not a double tap.
        context.startTimeForDoubleTapMs = null;
      }

      var touches = [];
      for (var i = 0; i < event.touches.length; i++) {
        var t = event.touches[i];
        // we dispense with 'dragGetX_' because all touchBrowsers support pageX
        touches.push({
          pageX: t.pageX,
          pageY: t.pageY,
          dataX: g.toDataXCoord(t.pageX),
          dataY: g.toDataYCoord(t.pageY)
          // identifier: t.identifier
        });
      }
      context.initialTouches = touches;

      if (touches.length == 1) {
        // This is just a swipe.
        context.initialPinchCenter = touches[0];
        context.touchDirections = { x: true, y: true };
      } else if (touches.length >= 2) {
        // It's become a pinch!
        // In case there are 3+ touches, we ignore all but the "first" two.

        // only screen coordinates can be averaged (data coords could be log scale).
        context.initialPinchCenter = {
          pageX: 0.5 * (touches[0].pageX + touches[1].pageX),
          pageY: 0.5 * (touches[0].pageY + touches[1].pageY),

          // TODO(danvk): remove
          dataX: 0.5 * (touches[0].dataX + touches[1].dataX),
          dataY: 0.5 * (touches[0].dataY + touches[1].dataY)
        };

        // Make pinches in a 45-degree swath around either axis 1-dimensional zooms.
        var initialAngle = 180 / Math.PI * Math.atan2(context.initialPinchCenter.pageY - touches[0].pageY, touches[0].pageX - context.initialPinchCenter.pageX);

        // use symmetry to get it into the first quadrant.
        initialAngle = Math.abs(initialAngle);
        if (initialAngle > 90) initialAngle = 90 - initialAngle;

        context.touchDirections = {
          x: initialAngle < 90 - 45 / 2,
          y: initialAngle > 45 / 2
        };
      }

      // save the full x & y ranges.
      context.initialRange = {
        x: g.xAxisRange(),
        y: g.yAxisRange()
      };
    },
    moveTouch: function(event, g, context) {
      // If the tap moves, then it's definitely not part of a double-tap.
      context.startTimeForDoubleTapMs = null;

      var i,
          touches = [];
      for (i = 0; i < event.touches.length; i++) {
        var t = event.touches[i];
        touches.push({
          pageX: t.pageX,
          pageY: t.pageY
        });
      }
      var initialTouches = context.initialTouches;

      var c_now;

      // old and new centers.
      var c_init = context.initialPinchCenter;
      if (touches.length == 1) {
        c_now = touches[0];
      } else {
        c_now = {
          pageX: 0.5 * (touches[0].pageX + touches[1].pageX)//,
          //pageY: 0.5 * (touches[0].pageY + touches[1].pageY)
        };
      }

      // this is the "swipe" component
      // we toss it out for now, but could use it in the future.
      var swipe = {
        pageX: c_now.pageX - c_init.pageX//,
        //pageY: c_now.pageY - c_init.pageY
      };
      var dataWidth = context.initialRange.x[1] - context.initialRange.x[0];
      var dataHeight = context.initialRange.y[0] - context.initialRange.y[1];
      swipe.dataX = swipe.pageX / g.plotter_.area.w * dataWidth;
      //swipe.dataY = swipe.pageY / g.plotter_.area.h * dataHeight;
      var xScale, yScale;

      // The residual bits are usually split into scale & rotate bits, but we split
      // them into x-scale and y-scale bits.
      if (touches.length == 1) {
        xScale = 1.0;
        //yScale = 1.0;
      } else if (touches.length >= 2) {
        var initHalfWidth = initialTouches[1].pageX - c_init.pageX;
        xScale = (touches[1].pageX - c_now.pageX) / initHalfWidth;

        //var initHalfHeight = initialTouches[1].pageY - c_init.pageY;
        //yScale = (touches[1].pageY - c_now.pageY) / initHalfHeight;
      }

      // Clip scaling to [1/8, 8] to prevent too much blowup.
      xScale = Math.min(8, Math.max(0.125, xScale));
      //yScale = Math.min(8, Math.max(0.125, yScale));

      var didZoom = false;
      if (context.touchDirections.x) {
        g.dateWindow_ = [c_init.dataX - swipe.dataX + (context.initialRange.x[0] - c_init.dataX) / xScale, c_init.dataX - swipe.dataX + (context.initialRange.x[1] - c_init.dataX) / xScale];
        didZoom = true;
      }

      /*if (context.touchDirections.y) {
        for (i = 0; i < 1 ; i++) {
          var axis = g.axes_[i];
          var logscale = g.attributes_.getForAxis("logscale", i);
          if (logscale) {
            // TODO(danvk): implement
          } else {
              axis.valueRange = [c_init.dataY - swipe.dataY + (context.initialRange.y[0] - c_init.dataY) / yScale, c_init.dataY - swipe.dataY + (context.initialRange.y[1] - c_init.dataY) / yScale];
              didZoom = true;
            }
        }
      }*/

      g.drawGraph_(false);

      // We only call zoomCallback on zooms, not pans, to mirror desktop behavior.
      if (didZoom && touches.length > 1 && g.getFunctionOption('zoomCallback')) {
        var viewWindow = g.xAxisRange();
        g.getFunctionOption("zoomCallback").call(g, viewWindow[0], viewWindow[1], g.yAxisRanges());
      }
    },
    endTouch: function(event, g, context) {
      if (event.touches.length !== 0) {
        // this is effectively a "reset"
        startTouch(event, g, context);
      } else if (event.changedTouches.length == 1) {
        // Could be part of a "double tap"
        // The heuristic here is that it's a double-tap if the two touchend events
        // occur within 500ms and within a 50x50 pixel box.
        var now = new Date().getTime();
        var t = event.changedTouches[0];
        if (context.startTimeForDoubleTapMs && now - context.startTimeForDoubleTapMs < 500 && context.doubleTapX && Math.abs(context.doubleTapX - t.screenX) < 50 && context.doubleTapY && Math.abs(context.doubleTapY - t.screenY) < 50) {
          g.resetZoom();
        } else {
          context.startTimeForDoubleTapMs = now;
          context.doubleTapX = t.screenX;
          context.doubleTapY = t.screenY;
        }
      }
    },
    mouseWheelZoom: function(event, g, context, callbackFunc) {
      if(lastClicked != g) return;
      var normal = event.detail ? event.detail * -1 : event.wheelDelta / 40;
      // For me the normalized value shows 0.075 for one click. If I took
      // that verbatim, it would be a 7.5%.
      var percentage = normal / 50;

      if (!(event.offsetX && event.offsetY)){
        event.offsetX = event.layerX - event.target.offsetLeft;
        //event.offsetY = event.layerY - event.target.offsetTop;
      }
      var percentages = offsetToPercentage(g, event.offsetX, event.offsetY);
      var xPct = percentages[0];
      var yPct = percentages[1];
      zoom(g, percentage, xPct, yPct);
      event.preventDefault();
      event.stopPropagation();
      clearInterval(context.MouseWheelTimer);
      context.MouseWheelTimer = setTimeout( function() { this.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]); }, 500);  
    },
    EndTouchEvt: function(event, g, contex) {
      endTouch(event, g, contex);
        //if((contex.isPanning) /*&& (contex.regionWidth > 0)*/) {
          this.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
        //}
    },
    MouseUpEvt: function(event, g, contex) {
        oZm.mouseUp(event, g, contex);
        if((contex.isPanning) && (contex.regionWidth > 0)) this.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
    },
    zoomWheelGraphReset: function(){
      lastClicked = null;
    },
    drawHighlighBarChartCallback: function(g, seriesName, canvasContext, cx, cy, color, pointSize){
      canvasContext.fillStyle=chroma(canvasContext.fillStyle).brighten(2);
      canvasContext.fillRect(cx-g.bar_width/2,cy,g.bar_width,g.y_bottom - cy);
    },
    props: ['options'],
    data() {
      return { };
    },
    methods: {
      SetOptions: function(options) {
        this.Options = Object.assign({}, this.Options, options);
        if(this.Graph !== null) this.Graph.updateOptions(this.Options);
      },
      SetData: function(data) {
        this.Options = Object.assign({}, this.Options, { file: data });
        this.Graph.updateOptions(this.options);
      },
      Redraw: function(w, h) {
        $(this.Graph.graphDiv).parent().removeAttr('style');
        if((w !== undefined) && (h !== undefined)) this.Graph.resize(w, h);
        this.Graph.resize();
      }
    },
    created: function() {
      this.Graph = null;
      this.Options = Object.assign({ strokeWidth: 0.5, gridLineColor: "rgba(0, 0, 0, 0.1)", gridLinePattern: [4, 4], labels: ['Index', 'Value'], interactionModel: {} }, this.options);
    },
    mounted: function() {
      this.Graph = new Dygraph(this.$el, [[0, 0]], this.Options);
    },
    beforeDestroy: function() {
      this.Graph.destroy();
    }
  }
  
</script>