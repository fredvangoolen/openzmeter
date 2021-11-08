<template>
  <div>
    <div ref="d3" />
    <q-resize-observer @resize="onResize" />
  </div>
</template>

<script>
  import * as d3 from 'd3';
  import moment from 'moment';
  export default {
    props: { data: Object, year: Number },
    data: function() {
      return { Width: 300, MarginTop: 50, MarginLeft: 60 };
    },
    mounted: function() {
      this.SVG = d3.select(this.$refs.d3).append('svg');
      this.xScale = d3.scaleLinear().domain([0, 96]);
      this.xAxis = d3.axisTop(this.xScale).tickSize(2).ticks(96).tickFormat(d => ((d < 96) && (d % 4 == 0)) ? (d / 4).toString().padStart(2, '0') : '');
      this.yScale = d3.scaleBand();
      this.yAxis = d3.axisLeft(this.yScale).tickSize(2).tickFormat(d => moment(this.Year).dayOfYear(d + 1).format('dd - DD/MM'));
      this.SVG.append('g').attr('class', 'points').attr('transform', 'translate(0, 0)').on('mouseover', this.onMouseOver).on('mouseout', this.onMouseOut);
      this.SVG.append('g').attr('class', 'legend').attr('transform', 'translate(0, 0)');
      this.SVG.append('g').attr('class', 'x-axis').attr('transform', 'translate(0, ' + this.MarginTop + ')');
      this.SVG.append('g').attr('class', 'y-axis').attr('transform', 'translate(' + this.MarginLeft + ', 0)');
    },
    watch: {
      'data.Rows': function() {
        console.log("redraw");
      this.Redraw();
      }
    },
    methods: {
      onMouseOver: function(d) {
        d3.select(d.toElement).transition().duration(10).style("fill-opacity", 0.3);
      },
      onMouseOut: function(d) {
        d3.select(d.fromElement).transition().duration(200).style("fill-opacity", 1);
      },
      onResize: function(val) {
        if(this.Width == val.width) return;
        this.Width = val.width;
        this.SVG.attr('width', val.width);
        this.Redraw();
      },
      Redraw: function() {
        let Max = 0;
        if(this.data.Headers == null) {
          this.SVG.attr('height', 500);
          d3.select('.x-axis').html('');
          d3.select('.y-axis').html('');
          return;
        }
        this.Cells = [];
        this.Days = [];
        let TimeCol   = this.data.Headers.indexOf('Time');
        let EnergyCol = this.data.Headers.indexOf('Active_Energy');
        let Days = moment([this.year, 11, 31]).dayOfYear();
        for(let i = 0; i < Days; i++) {
          this.Days.push(i);
          if(this.data.Aggreg == '1H') {
            for(let n = 0; n < 24; n++) this.Cells.push({ Value: null, Day: i, Block: n });
          } else {
            for(let n = 0; n < 96; n++) this.Cells.push({ Value: null, Day: i, Block: n });
          }
        }
        this.SVG.attr('height', 12 * Days + 30);
        this.xScale.range([this.MarginLeft, this.Width]);
        this.yScale.range([this.MarginTop, 12 * Days + 30]);
        this.yScale.domain(this.Days);
        d3.select('.x-axis').call(this.xAxis);
        d3.select('.y-axis').call(this.yAxis);
        this.data.Rows.forEach(row => {
          let RowDate = moment(row[TimeCol]).tz(this.Analyzer.TimeZone);
          let Cell = (this.data.Aggreg == '1H') ? (RowDate.dayOfYear() - 1) * 24 + RowDate.hour() : (RowDate.dayOfYear() - 1) * 96 + RowDate.hour() * 4 + Math.trunc(RowDate.minute() / 15);
          this.Cells[Cell].Value = (this.Cells[Cell].Value == null) ? this.ArraySUM(row[EnergyCol]) : this.Cells[Cell].Value + this.ArraySUM(row[EnergyCol]);
          Max = Math.max(Max, Math.abs(this.Cells[Cell].Value));
        });
        this.SVG.select('.points').selectAll('.point').data(this.Cells).join(
          enter => enter.append("rect")
            .attr("class", "point")
            .attr("x", d => this.xScale((this.data.Aggreg == '1H') ? d.Block * 4 : d.Block))
            .attr("y", d => this.yScale(d.Day))
            .attr("width", ((this.Width - this.MarginLeft) / ((this.data.Aggreg == '1H') ? 24 : 96)) - 0.1)
            .attr("height", 12 - 0.1)
            .attr("fill", d => (d.Value == null) ? '#eee' : d3.interpolateRdYlBu(1 - ((d.Value / (2.0 * Max)) + 0.5)))
          ,
          update => update
            .attr("x", d => this.xScale((this.data.Aggreg == '1H') ? d.Block * 4 : d.Block))
            .attr("y", d => this.yScale(d.Day))
            .attr("width", ((this.Width - this.MarginLeft) / ((this.data.Aggreg == '1H') ? 24 : 96)) - 0.1)
            .attr("fill", d => (d.Value == null) ? '#eee' : d3.interpolateRdYlBu(1 - ((d.Value / (2.0 * Max)) + 0.5)))
          ,
          exit => exit.remove()
        );
        d3.select('.legend').html('');
        var Colors = [];
        for(let n = 0; n < 100; n++) {
          this.SVG.select('.legend').append("rect")
            .attr("x", (n / 100.0) * this.Width)
            .attr("y", 12)
            .attr("width", this.Width / 100.0)
            .attr("height", 12)
            .attr("fill", d3.interpolateRdYlBu(1 - (n / 100.0)));
        }
        this.SVG.select('.legend')
          .append("text")
          .attr("text-anchor", "start")
          .attr("x", 1)
          .attr("y", 10).attr('font-size', 10)
          .text("a");
        this.SVG.select('.legend')
          .append("text")
          .attr("text-anchor", "end")
          .attr("x", this.Width - 1)
          .attr("y", 10).attr('font-size', 10)
          .text("b");
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