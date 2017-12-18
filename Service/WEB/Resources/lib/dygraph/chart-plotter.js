/*
 * Copyright (C) 2015 Pedro Sánchez Alguacil
 *
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

// This function draws bars for a single series. See
// multiColumnBarPlotter below for a plotter which can draw multi-series
// bar charts.
function barChartPlotter(e) {
  var ctx = e.drawingContext;
  var points = e.points;
  var y_bottom = e.dygraph.toDomYCoord(0);
  ctx.fillStyle = darkenColor(e.color);
  var min_sep = Infinity;
  for (var i = 1; i < points.length; i++) {
    var sep = points[i].canvasx - points[i - 1].canvasx;
    if (sep < min_sep)
      min_sep = sep;
  }
  var bar_width = Math.floor(2.0 / 3 * min_sep);
  for (var i = 0; i < points.length; i++) {
    var p = points[i];
    var center_x = p.canvasx;
    ctx.fillRect(center_x - bar_width / 2, p.canvasy, bar_width, y_bottom - p.canvasy);
    ctx.strokeRect(center_x - bar_width / 2, p.canvasy, bar_width, y_bottom - p.canvasy);
  }
}

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
    // We need to handle all the series simultaneously.
    if (e.seriesIndex !== 0)
        return;

    var g = e.dygraph;
    var ctx = e.drawingContext;
    var sets = e.allSeriesPoints;
    var y_bottom = e.dygraph.toDomYCoord(0);

    // Find the minimum separation between x-values.
    // This determines the bar width.
    var min_sep = Infinity;
    for (var j = 0; j < sets.length; j++) {
        var points = sets[j];
        for (var i = 1; i < points.length; i++) {
            var sep = points[i].canvasx - points[i - 1].canvasx;
            if (sep < min_sep)
                min_sep = sep;
        }
    }
    var bar_width = Math.floor(2.0 / 3 * min_sep);

    var fillColors = [];
    var strokeColors = g.getColors();
    for (var i = 0; i < strokeColors.length; i++) {
        fillColors.push(darkenColor(strokeColors[i]));
    }

    for (var j = 0; j < sets.length; j++) {
        ctx.fillStyle = fillColors[j];
        ctx.strokeStyle = strokeColors[j];
        for (var i = 0; i < sets[j].length; i++) {
            var p = sets[j][i];
            var center_x = p.canvasx;
            var x_left = center_x - (bar_width / 2) * (1 - j / (sets.length - 1));

            ctx.fillRect(x_left, p.canvasy,
                    bar_width / sets.length, y_bottom - p.canvasy);

            ctx.strokeRect(x_left, p.canvasy,
                    bar_width / sets.length, y_bottom - p.canvasy);
        }
    }
}
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