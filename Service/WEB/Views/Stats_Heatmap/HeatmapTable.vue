<template>
  <div>
    <div ref="d3" />
    <q-resize-observer @resize="onResize" />
  </div>
</template>

<script>
  import * as d3 from 'd3';
  import moment from 'moment';
//  import chroma from 'chroma-js';
//  import * as oZm from 'JS/openZmeter.js';
//  var DaysNames = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"];
//const CANVAS = {
//  margin: {
//    top: 20,
//    right: 40,
//    bottom: 100,
//    left: 100,
//  },
//  xLabel: 'Years',
//  yLabel: 'Months',
//  barColors: [
//    [0, '#5e4fa2'],
//    [2.7, '#3288bd'],
//    [3.9, '#66c2a5'],
//    [5, '#abdda4'],
//    [6.1, '#e6f598'],
//    [7.2, '#ffffbf'],
//    [8.3, '#fee08b'],
//    [9.4, '#fdae61'],
//    [10.5, '#f46d43'],
//
//    [11.6, '#d53e4f'],
//    [12.7, '#9e0142'],
//  ],
//};
  export default {
    props: { data: Object, year: Number },
    data: function() {
      return { Height: 300, Width: 300, MarginTop: 30, MarginLeft: 30 };
    },
    mounted: function() {
      this.SVG = d3.select(this.$refs.d3).append('svg');
      this.xScale = d3.scalePoint().domain(['00', '01', '02', '03', '04', '05', '06', '07', '08', '09', '10', '11', '12', '13', '14', '15', '16', '17', '18', '19', '20', '21', '22', '23', '']);
      this.xAxis = d3.axisTop(this.xScale).ticks(24);
      this.SVG.append("g").attr("class", "x-axis").attr("transform", "translate(0," + this.MarginTop +")");
      this.yScale = d3.scaleLinear();
      this.yAxis = d3.axisLeft(this.yScale).tickSizeOuter(0);
      this.SVG.append("g").attr("class", "y-axis").attr("transform", "translate(" + this.MarginLeft + ",0)");
    },
    watch: {
      year: function() { this.Redraw(); },
      'data.Rows': function() {
        this.Redraw();
  //    console.log('pintando');
      // X values and scaling function
      //Aqui iria de 0 a 23
  //    let xArray = this.data.Rows.map(a => a[0]);
  //    this.axis.x.values = d3.scaleLinear().domain(d3.extent(xArray)).range([0, this.chartWidth]);

      // this.axis.x.scale becomes a function that converts a x value to a x position
  //    this.axis.x.scale = d3.scaleBand().domain([...(new Set(xArray))])
  //      .paddingInner(0)
  //      .paddingOuter(0)
        // d3.min(xArray) is mapping to this.chartWidth
        // d3.max(xArray) + 5 (same as above) is mapping to starting of axis
  //      .range([0, this.chartWidth]);

      // Y values and scaling function
      //esto seria el dia
  //    let yArray = this.data.Rows.map(a => a[0]);
  //    this.axis.y.values = d3
  //      .scaleLinear()
        // Labels on axis, should be equal to or larger than dataset
  //      .domain(d3.extent(yArray))
        // Together with yGuide translate below, determines where to start drawing the axis
  //      .range([0, this.chartHeight]);

      // this.axis.y.scale becomes a function that converts a y value to a y position
  //    this.axis.y.scale = d3
  //      .scaleLinear()
  //      .domain(d3.extent(yArray))
        // Smallest value is mapping to margin-top
  //      .range([CANVAS.margin.top, CANVAS.margin.top + this.chartHeight - this.barHeight]);


      // Colors
  //    let tArray = this.data.Rows.map(a => this.ArraySUM(a[1]));
  //    this.color.scale = d3
  //      .scaleLinear()
        // .scaleSequential()
        // .domain([d3.min(tArray), d3.mean(tArray), d3.max(tArray)])
  //      .domain(CANVAS.barColors.map(d => d[0]))
        // .interpolator(d3.interpolateRainbow)
  //      .range(CANVAS.barColors.map(d => d[1]));


//      this.drawGuide();
//      this.drawData();
//      this.drawLegends();
//      this.addListeners();
      }
    },
    methods: {
      onResize: function(val) {
        this.Height = val.height;
        this.Width = val.width;
        this.SVG.attr('width', val.width);
        this.SVG.attr('height', 500);

        this.xScale.range([this.MarginLeft, this.Width]);
        d3.select('.x-axis').call(this.xAxis);
        this.yScale.range([this.MarginTop, this.Height]);
        d3.select('.y-axis').call(this.yAxis);

        this.Redraw();
      },
      Redraw: function() {
        this.Cells = [];
        if((this.year != null) && (this.data.Aggreg != null)) {
          let Days = moment([this.year, 11, 31]).dayOfYear();
          for(let i = 0; i < Days; i++) {
            for(let n = 0; n < 24; n++) {
              if(this.data.Aggreg == '1H') {
                this.Cells.push({ Value: null, Day: i, Hour: n });
              } else {
                for(let q = 1; q < 5; q++) this.Cells.push({ Value: null, Day: i, Hour: n, Quarter: q });
              }
            }
          }
//          if(this.data.Headers != null) {
//            let Max = -Infinity;
//            let Min =  Infinity;
//            let TimeCol = this.data.Headers.indexOf('Time');
//            let EnergyCol = this.data.Headers.indexOf('Active_Energy');
//            this.data.Rows.forEach(row => {
//              let RowDate = moment(row[TimeCol]).tz(this.Analyzer.TimeZone);
//            if(Tmp.Date != RowDate.format('DD/MM/YYYY')) {
//              Days.push(Tmp);
//              Tmp = new Object;
//            }
//            if(Tmp.Date == null) {
//              Tmp = { DayOfWeek: DaysNames[RowDate.day()], Date: RowDate.format('DD/MM/YYYY'), Cells: [] };
//              for(let n = 0; n < 24; n++) {
//                if(this.data.Aggreg == '1H') {
//                  Tmp.Cells.push({ Value: null, Date: 'Hour ' + n });
//                } else {
//                  for(let q = 1; q < 5; q++) Tmp.Cells.push({ Value: undefined, Date: 'Hour ' + n + '(' + q + 'Q)' });
//                }
//              }
//            }
//              let Cell = (this.data.Aggreg == '15M') ? RowDate.dayOfYear() RowDate.hour() * 4 + Math.trunc(RowDate.minute() / 15) : RowDate.hour();
//            let CellVal = this.ArraySum(row[EnergyCol]);
//            Max = Math.max(Max, CellVal);
//            Min = Math.min(Min, CellVal);
//            Tmp.Cells[Cell].Value = (Tmp.Cells[Cell].Value ?? 0) + CellVal;
//          });
//          if(Tmp.Date != null) Days.push(Tmp);
//          this.Cells = [];
//          Days.forEach(day => {
//
//          });

//          this.$emit('update:range', [Min, Max]);
//          var ColorScale = chroma.bezier(['#00429d', '#96ffea', '#ffffe0', '#ff005e', '#93003a']);
//          var Export = [];
//          var Row = ['Day', 'Date'];
//          for(var h = 0; h < 24; h++) {
//            if(this.aggreg == '15M') {
//              for(var q = 1; q < 5; q++) Row.push(("0" + h).slice(-2) + ' Q' + q);
//            } else {
//              Row.push(("0" + h).slice(-2));
//            }
//
          }
          let Points = this.SVG.selectAll('.points').data(this.Cells);
            Points.exit()
    .transition()
    .duration(1000)
    .attr("background-color", '#FFF')
    .remove();
            Points.enter()
                .append("rect")
    .attr("class", "points")
    .attr("x", /*this.xScale(0)*/ + 1)
    .attr("y", function(d) {
      return 0//yScale(d.title)
    })
    .attr("width", 0)
    .attr("height", 12/*yScale.bandwidth()*/)
    .attr("fill", function(d) {
      return /*color(letters.indexOf(d.title) + 1)*/ '#400'
    })
    .merge(Points).transition()
    .duration(1000)
    .attr("y", function(d) {
      return 0//yScale(d.title)
    })
    .attr("width", function(d) {
      return /*xScale(100)*/ - this.MarginLeft
    });
          console.log(Points);

      },


      drawLegends: function() {
        let height = 20;
        let width = 35;
        let top = this.graphHeight - (height * 3);
        let left = this.graphWidth - CANVAS.margin.right - CANVAS.margin.left - (width * 11);

        this.createD3Element({ data: CANVAS.barColors, type: 'rect' })
          .attr('fill', d => d[1])
          .attr('height', _ => height)
          .attr('width', _ => width)
          .attr('x', (_, i) => i * width + left)
          .attr('y', _ => top);
        this.createD3Element({ data: CANVAS.barColors, type: 'text' })
          .text(d => d[0])
          .attr('height', _ => height)
          .attr('width', _ => width)
          .attr('x', (_, i) => i * width + left + width / 2)
          .attr('y', _ => top + height * 2)
          .style('font-size', '13px')
          .attr('text-anchor', 'middle')
          .style('fill', '#333');
      },
      drawData: function() {
        // translate(x, y) specifies where bar begins, +1 to move right of y axis
        this.ddd.chart = this
          .createD3Element({
            data: this.d3Data.map(d => ({
              x: d.x,
              y: d.y,
              temperature: d.temperature,
            })),
            type: 'rect',
          })
          .attr('fill', d => this.color.scale(d.temperature))
          .attr('height', _ => this.barHeight)
          .attr('width', _ => this.axis.x.scale.bandwidth())
          .attr('x', d => this.axis.x.scale(d.x))
          .attr('y', d => this.axis.y.scale(d.y));
        // console.log('remove', this.d3Data.map(d => this.axis.y.scale(d.y)))
        // console.log('remove 2', this.axis.x.scale.bandwidth())
      },
      drawGuide: function() {
        this.createD3Element({ data: [...Array(12)].map((_, i) => {
            return {
              x: -10,
              y: (i + 1) * this.barHeight,
              text: moment((i + 1), 'M').format('MMMM'),
            }
          }),
          type: 'text',
        })
        .text(d => d.text)
        .attr('x', d => d.x)
        .attr('y', d => d.y)
        .attr('text-anchor', 'end')
        .style('font-size', '0.7rem');

        // Vertical line
        this
          .createD3Element()
          .append('path')
          .attr('d', d3.line()([[0, CANVAS.margin.top], [0, CANVAS.margin.top + this.chartHeight]]))
          .style('stroke', '#ccc')
          .style('stroke-width', '2')
          .style('stroke-linecap', 'round')


        let yLabel = [{ x: -(this.chartHeight / 2), y: -75, text: CANVAS.yLabel } ];
        this.createD3Element({ data: yLabel, type: 'text' })
          .text(d => d.text)
          .attr("transform", "rotate(-90)")
          .attr('x', d => d.x)
          .attr('y', d => d.y)
          .style('font-size', '15px')
          .style('font-weight', 'bold')
          .attr('text-anchor', 'middle')
          .style('fill', '#6D4C41');

        this.createD3Element({ transformX: CANVAS.margin.left, transformY: CANVAS.margin.top + this.chartHeight })
          .call(d3
            .axisBottom(this.axis.x.values)
            .ticks(26)
            .tickFormat(d => d.toString())
          );
        let xLabel = [
          { x: Math.floor(this.chartWidth / 2), y: this.graphHeight - (CANVAS.margin.bottom - 45), text: CANVAS.xLabel, },
        ];
        this.createD3Element({ data: xLabel, type: 'text' })
          .text(d => d.text)
          .attr('x', d => d.x)
          .attr('y', d => d.y)
          .style('font-size', '15px')
          .style('font-weight', 'bold')
          .attr('text-anchor', 'middle')
          .style('fill', '#6D4C41');
        },
    createD3Element ({
      data = [],
      transformX = CANVAS.margin.left + 1,
      transformY = 0,
      type = '',
    } = {}) {
      let result = this.svg
        .append('g')
        .attr('transform', `translate(${transformX}, ${transformY})`);
      if (type) {
        result = result
          .selectAll(type)
          .data(data)
          .enter()
          .append(type);
      }
      return result;
    },



/*    addListeners () {
      let component = this;
      this.ddd.chart
        .on('mouseover', function(data, index, circles) {
          // Approximate width of tooltip window
          let widthOfTooltip = 180;
          let tooltipX = +d3.event.pageX + ((d3.event.pageX + widthOfTooltip) > window.innerWidth ? (-widthOfTooltip) : 15);
          let tooltipY = +d3.event.pageY - 100;
          component.ddd.tooltip.html(component.d3Data[index].tooltip)
            // Top left of tooltip, relative to screen, not graph
            .style('left', `${tooltipX}px`)
            .style('top', `${tooltipY}px`)
            .style('opacity', 1);

          d3.select(this)
            .style('opacity', .5)
        })
        .on('mouseout', function(data) {
          component.ddd.tooltip.html('')
            .style('opacity', 0);

          d3.select(this)
            .transition()
            .duration(300)
            .style('opacity', 1)
        });
    }*/
  },







     /* CalcOptions: function() {
        this.Days = [];
        if((this.data == null) || (this.aggreg == null)) return;
        let Headers   = this.data[0];
        let Rows      = this.data.slice(1);
        let Tmp       = { };
        let TimeCol   = Headers.indexOf('Time');
        let EnergyCol = Headers.indexOf('Active_Energy');
        for(let i = 0; i < Rows.length; i++) {
          let RowDate = moment(Rows[i][TimeCol]).tz(this.Analyzer.TimeZone);
          if(Tmp.Date != RowDate.format('DD/MM/YYYY')) {
            this.Days.push(Object.assign({}, Tmp));
            Tmp = { };
          }
          if(Tmp.Date == undefined) {
            Tmp = { DayOfWeek: DaysNames[RowDate.day()], Date: RowDate.format('DD/MM/YYYY'), Cells: [] };
            for(let n = 0; n < 24; n++) {
              if(this.aggreg === '1H') {
                Tmp.Cells.push({ Value: undefined, Date: 'Hour ' + n });
              } else {
                for(let q = 1; q < 5; q++) Tmp.Cells.push({ Value: undefined, Date: 'Hour ' + n + '(' + q + 'Q)' });
              }
            }
          }
          var Cell = (this.aggreg === '15M') ? RowDate.hour() * 4 + Math.trunc(RowDate.minute() / 15) : RowDate.hour();
          for(var c = 0; c < Rows[i][EnergyCol].length; c++) {
            Tmp.Cells[Cell].Value = (Tmp.Cells[Cell].Value == null) ? Rows[i][EnergyCol][c] : Tmp.Cells[Cell].Value + Rows[i][EnergyCol][c];
          }
        }
        if(Tmp.Date != undefined) this.Days.push(Tmp);
        let Max = -Infinity;
        let Min =  Infinity;
        for(var i in this.Days) {
          for(var n in this.Days[i].Cells) {
            if(this.Days[i].Cells[n].Value === undefined) continue;
            Max = Math.max(Max, this.Days[i].Cells[n].Value);
            Min = Math.min(Min, this.Days[i].Cells[n].Value);
          }
        }
        this.$emit('update:range', [Min, Max]);
        var ColorScale = chroma.bezier(['#00429d', '#96ffea', '#ffffe0', '#ff005e', '#93003a']);
        var Export = [];
        var Row = ['Day', 'Date'];
        for(var h = 0; h < 24; h++) {
          if(this.aggreg == '15M') {
            for(var q = 1; q < 5; q++) Row.push(("0" + h).slice(-2) + ' Q' + q);
          } else {
            Row.push(("0" + h).slice(-2));
          }
        }
        Export.push(Row);
        for(var i in this.Days) {
          var Row = [this.Days[i].DayOfWeek, this.Days[i].Date];
          for(var n in this.Days[i].Cells) {
            Row.push(this.Round(this.Days[i].Cells[n].Value, 3));
            if(this.Days[i].Cells[n].Value === undefined) {
              this.Days[i].Cells[n].Color = "url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAQAAAAECAIAAAAmkwkpAAAAJElEQVQImT2JsQ0AMAyDyP832q+QIWqZEJDEx6gA0JZr9/mmLvBPKENuQmcdAAAAAElFTkSuQmCC')";
              this.Days[i].Cells[n].BorderColor = '#EEE'
              this.Days[i].Cells[n].Text = 'No data';
            } else {
              this.Days[i].Cells[n].Color = ColorScale((this.Days[i].Cells[n].Value - Min) / (Max - Min));
              this.Days[i].Cells[n].BorderColor = chroma(this.Days[i].Cells[n].Color).darken(0.1);
              var Units = this.BestUnits(this.Days[i].Cells[n].Value);
              this.Days[i].Cells[n].Text = this.Round(this.ScaleUnits(this.Days[i].Cells[n].Value, Units), 2, false) + ' ' + Units + 'Wh';

            }
          }
          if(Row.length > 2) Export.push(Row);
        }
        this.Days = this.Days.reverse();
        this.$emit('update:export', Export);
      }*/

  }
</script>