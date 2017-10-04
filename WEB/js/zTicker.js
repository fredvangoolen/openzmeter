/*
 * Copyright (C) 2017 zRed
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


var zTicker = function (min, max, pixels, opts, dygraph, vals) {
    var ticks = [];
    var space_between_ticks = 10;
    var diff = (Math.ceil(max) + 1 - (Math.floor(min) - 1));
    if (diff < 5) {
        space_between_ticks = 0.1;
    } else if (diff < 10) {
        space_between_ticks = 0.5;
    } else if (diff < 50) {
        space_between_ticks = 1;
    } else if (diff < 100) {
        space_between_ticks = 5;
    }
    var min = Math.floor(min) - 1;
    var num_ticks = diff / space_between_ticks;
    var everyTwo = false;
    if (pixels / num_ticks < 5) {
        everyTwo = true;
        if (space_between_ticks * (space_between_ticks < 0 ? 10 : 1) % 5 == 0) {
            space_between_ticks += space_between_ticks;
        } else {
            //space_between_ticks *= 5;
        }
        num_ticks = (Math.ceil(max) + 1 - (Math.floor(min) - 1)) / space_between_ticks;
    }
    for (var i = 0; i < num_ticks; i++) {
        ticks.push({v: min + space_between_ticks * i, label: '<small>' + (!everyTwo ? (min + space_between_ticks * i).toFixed(2) : ((i % 2 == 0 && i != 0) ? (min + space_between_ticks * i).toFixed(2) : '')) + '</small>'})
    }
    return ticks;
};