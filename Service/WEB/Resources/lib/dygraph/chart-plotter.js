/*
 * Copyright (C) 2015 Pedro Sánchez Alguacil
 *               2017 Eduardo Viciana Gamez
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Darken a color
function darkenColor(colorStr) {
  var color = Dygraph.toRGB_(colorStr);
  color.r = Math.floor((255 + color.r) / 2);
  color.g = Math.floor((255 + color.g) / 2);
  color.b = Math.floor((255 + color.b) / 2);
  return 'rgba(' + color.r + ',' + color.g + ',' + color.b + ', 0.8)';
}

/* Polar coordinate plotter */
function polarLinePlotter(e) {
  if((e.seriesIndex !== 0) || (e.plotArea.h < 0) || (e.plotArea.w < 0)) return;
  var ctx = e.drawingContext;
  ctx.lineWidth = 0.5;
  ctx.strokeStyle = e.dygraph.attrs_.axisLineColor;
  ctx.beginPath();
  ctx.moveTo(e.dygraph.toDomXCoord(-100), e.dygraph.toDomYCoord(0));
  ctx.lineTo(e.dygraph.toDomXCoord(100), e.dygraph.toDomYCoord(0));
  ctx.moveTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(-100));
  ctx.lineTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(100));
  ctx.stroke();
  ctx.beginPath();
  ctx.setLineDash(e.dygraph.user_attrs_.gridLinePattern);
  ctx.strokeStyle = e.dygraph.user_attrs_.gridLineColor;
  ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(100) - e.dygraph.toDomXCoord(0), 0, 2 * Math.PI);
  ctx.stroke();
  ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(75) - e.dygraph.toDomXCoord(0), 0, 2 * Math.PI);
  ctx.stroke();
  ctx.beginPath();
  ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(50) - e.dygraph.toDomXCoord(0), 0, 2 * Math.PI);
  ctx.stroke();
  ctx.beginPath();
  ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(25) - e.dygraph.toDomXCoord(0), 0, 2 * Math.PI);
  ctx.stroke();
  ctx.beginPath();
  ctx.fillStyle = '#000';
  ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), 5, 0, 2 * Math.PI, false);
  ctx.fill();
  ctx.setLineDash([]);
  for(i = 0; i < e.points.length; i++) {
    var X = e.dygraph.toDomXCoord(100.0 * e.allSeriesPoints[1][i].yval * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0));
    var Y = e.dygraph.toDomYCoord(100.0 * e.allSeriesPoints[1][i].yval * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0));
    e.points[i].canvasx = X;
    e.points[i].canvasy = Y;
    ctx.strokeStyle = e.dygraph.getColors()[i % 2];
    ctx.fillStyle = e.dygraph.getColors()[i % 2];
    ctx.lineWidth = (i % 2 === 0) ? 4 : 2;
    ctx.beginPath();
    ctx.moveTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0));
    ctx.lineTo(X, Y);
    ctx.stroke();
    ctx.beginPath();
    ctx.arc(X, Y, 3, 0, 2 * Math.PI, false);
    ctx.fill();
  }
}

/* PolarCrosshair */
Dygraph.Plugins.PolarCrosshair = (function() {
  "use strict";
  var polarCrosshair = function(opt_options) {
    this.canvas_ = document.createElement("canvas");
    opt_options = opt_options || {};
  };
  polarCrosshair.prototype.toString = function() {
    return "PolarCrosshair Plugin";
  };
  polarCrosshair.prototype.activate = function(g) {
    g.graphDiv.appendChild(this.canvas_);
    return { select: this.select, deselect: this.deselect };
  };
  polarCrosshair.prototype.select = function(e) {
    this.canvas_.width = e.dygraph.width_;
    this.canvas_.height = e.dygraph.height_;
    var ctx = this.canvas_.getContext("2d");
    ctx.clearRect(0, 0, width, height);
    ctx.strokeStyle = "rgba(0, 0, 0, 0.3)";
    ctx.beginPath();
    ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(e.dygraph.selPoints_[1].yval * 100.0) - e.dygraph.toDomXCoord(0), 0, 2 * Math.PI);
    ctx.stroke();
    ctx.closePath();
  };
  polarCrosshair.prototype.deselect = function(e) {
    var ctx = this.canvas_.getContext("2d");
    ctx.clearRect(0, 0, this.canvas_.width, this.canvas_.height);
  };
  polarCrosshair.prototype.destroy = function() {
    this.canvas_ = null;
  };
  return polarCrosshair;
})();

/* Draw bars without spacing*/
function barChartNoSpacedPlotter(e) {
  var ctx = e.drawingContext;
  var points = e.points;
  var y_bottom = e.dygraph.toDomYCoord(0);
  ctx.fillStyle = darkenColor(e.color);
  ctx.strokeWidth = 0.5;
  var min_sep = Infinity;
  for(var i = 1; i < points.length; i++) {
    var sep = Math.abs(points[i].canvasx - points[i - 1].canvasx);
    if(sep < min_sep) min_sep = sep;
  }
  var bar_width = Math.floor(min_sep);
  if(bar_width == 0) bar_width = min_sep;
  for (var i = 0; i < points.length; i++) {
    var p = points[i];
    var center_x = p.canvasx;
    ctx.fillRect(center_x - bar_width, p.canvasy, bar_width, y_bottom - p.canvasy);
    if (bar_width > 5) ctx.strokeRect(center_x - bar_width, p.canvasy, bar_width, y_bottom - p.canvasy);
    p.canvasx = p.canvasx - bar_width / 2;
  }
}

// Multiple column bar chart
function multiColumnBarPlotter(e) {
  if(e.seriesIndex !== 0) return;
  var g = e.dygraph;
  var ctx = e.drawingContext;
  var sets = e.allSeriesPoints;
  var y_bottom = e.dygraph.toDomYCoord(0);

  var min_sep = Infinity;
  for(var j = 0; j < sets.length; j++) {
    for(var i = 1; i < sets[j].length; i++) min_sep = Math.min(sets[j][i].canvasx - sets[j][i - 1].canvasx, min_sep);
  }
  var bar_width = Math.floor(2.5 / 3 * min_sep);
  var subbar_width = bar_width / sets.length;
  for(var j = 0; j < sets.length; j++) {
    ctx.strokeStyle = g.getColors()[j];
    ctx.fillStyle = darkenColor(ctx.strokeStyle);
    for(var i = 0; i < sets[j].length; i++) {
      var p = sets[j][i];
      var x_left = p.canvasx - (subbar_width * sets.length / 2) + j * subbar_width;
      ctx.fillRect(x_left, p.canvasy, bar_width / sets.length, y_bottom - p.canvasy);
      ctx.strokeRect(x_left, p.canvasy, bar_width / sets.length, y_bottom - p.canvasy);
    }
  }
}

/* Dibuja un grafico de forma similar a el plugin Cubism.js */
function barChartPlotterCubism(e) {
  var ctx = e.drawingContext;
  var area = e.plotArea;
  if(area.h <= 0 || area.w <= 0) return;
  var points = e.points;
  var max_range = Math.abs(points[0].yval);
  var BarWidth = Infinity;
  for(var i = 1; i < points.length; i++) {
    var Value = Math.abs(points[i].yval);
    var Sep = points[i].canvasx - points[i - 1].canvasx;
    if(Value > max_range) max_range = Value;
    if(Sep < BarWidth) BarWidth = Sep;
  }
  var ColorP = Dygraph.toRGB_(e.color);
  ColorP = 'rgba(' + Math.floor(ColorP.r) + ',' + Math.floor(ColorP.g) + ',' + Math.floor(ColorP.b) + ', 0.3)';
  var ColorN = Dygraph.toRGB_(e.color);
  ColorN = 'rgba(' + Math.floor(255 - ColorN.r) + ',' + Math.floor(255 - ColorN.g) + ',' + Math.floor(255 - ColorN.b) + ', 0.3)';
  var Height = area.h - 1;
  for(var i = 0; i < points.length; i++) {
    var BarHeight = (points[i].yval / max_range) * Height * 3;
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

/* Llamar en underlayCallback para resaltar los fines de semana */
function HightlightWeekends(color, canvas, area, g) {
  canvas.fillStyle = color;
  var fromTimestamp = g.dateWindow_[0];
  var toTimestamp = g.dateWindow_[1];
  while(fromTimestamp < toTimestamp) {
    var FromDate = new Date(fromTimestamp);
    var Tmp = Math.min(new Date(FromDate.getFullYear(), FromDate.getMonth(), FromDate.getDate() + 1, 0, 0, 0, 0).getTime(), toTimestamp);
    if((FromDate.getDay() === 6) || (FromDate.getDay() === 0)) {
      var canvas_left_x = g.toDomXCoord(fromTimestamp);
      var canvas_right_x = g.toDomXCoord(Tmp);
      var canvas_width = canvas_right_x - canvas_left_x;
      canvas.fillRect(canvas_left_x, area.y, canvas_width, area.h);
    }
    fromTimestamp = Tmp;
  }
}