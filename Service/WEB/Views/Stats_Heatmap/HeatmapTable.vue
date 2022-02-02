<template>
  <div>
    <svg />
    <q-resize-observer @resize="onResize" />
  </div>
</template>

<script>
  import * as d3 from 'd3';
  export default {
    props: { data: Object, year: Number, max: Number },
    emits: ['update:max'],
    data: function() {
      return { MarginTop: 12, MarginLeft: 60, Width: 100, Height: 100 };
    },
    mounted: function() {
      this.SVG = d3.select(this.$el).select('svg');
      this.SVG.append('g').attr('class', 'points').attr('transform', 'translate(0, 0)');
      this.SVG.append('g').attr('class', 'x-axis').attr('transform', 'translate(0, ' + this.MarginTop + ')');
      this.SVG.append('g').attr('class', 'y-axis').attr('transform', 'translate(' + this.MarginLeft + ', 0)');
      this.SVG.append('g').attr('class', 'border');
    },
    watch: {
      'data.Rows': function() {
        this.Redraw();
      },
      year: function(val, prev) {
        if(val == prev) this.Redraw();
      }
    },
    methods: {
      onResize: function(evt) {
        let Days = (this.year == null) ? 365 : this.$dayjs('31/12/' + this.year, 'DD/MM/YYYY').dayOfYear();
        this.Height = 8 * Days + this.MarginTop;
        this.Width = evt.width;
        this.SVG.attr('height', this.Height);
        this.SVG.attr('width', this.Width);
        if(this.Width != evt.width) this.Redraw();
      },
      Redraw: function() {
        let Max = 0;
        let Days = (this.year == null) ? 365 : this.$dayjs('31/12/' + this.year, 'DD/MM/YYYY').dayOfYear();
        let xScale = d3.scaleLinear()
          .domain([0, 96]);
        let xAxis = d3.axisTop(xScale)
          .tickSize(0)
          .tickSizeInner(2)
          .ticks(96)
          .tickFormat(d => ((d < 96) && (d % 4 == 0)) ? (d / 4).toString().padStart(2, '0') : '');
        let yScale = d3.scaleBand();
        let yAxis = d3.axisLeft(yScale)
          .tickSize(2)
          .tickFormat(d => this.$dayjs(this.Year).dayOfYear(d + 1).format('dd - DD/MM'));
        if(this.data.Headers == null) {
          d3.select('.points').html('');
          return;
        }
        this.Cells = [];
        this.Days = [];
        let TimeCol   = this.data.Headers.indexOf('Time');
        let EnergyCol = this.data.Headers.indexOf('Active_Energy');
        for(let i = 0; i < Days; i++) {
          this.Days.push(i);
          if(this.data.Aggreg == '1H') {
            for(let n = 0; n < 24; n++) this.Cells.push({ Value: null, Day: i, Block: n });
          } else {
            for(let n = 0; n < 96; n++) this.Cells.push({ Value: null, Day: i, Block: n });
          }
        }
        xScale.range([this.MarginLeft, this.Width]);
        d3.select('.x-axis').html('');
        d3.select('.x-axis').call(xAxis);
        yScale.range([this.MarginTop, 8 * Days + this.MarginTop]);
        yScale.domain(this.Days);
        d3.select('.y-axis').html('');
        d3.select('.y-axis').call(yAxis);
        d3.select('.border').html('');
        d3.select('.border').append('line')
          .attr('x1', this.Width - 1)
          .attr('x2', this.Width - 1)
          .attr('y1', this.MarginTop)
          .attr('y2', 8 * Days + this.MarginTop)
          .attr('stroke', 'black').attr('stroke-width', 1)
          .attr('shape-rendering', 'crispEdges');
        d3.select('.border').append('line')
          .attr('x1', this.MarginLeft)
          .attr('x2', this.Width)
          .attr('y1', 8 * Days + this.MarginTop - 1)
          .attr('y2', 8 * Days + this.MarginTop - 1)
          .attr('stroke', 'black').attr('stroke-width', 1)
          .attr('shape-rendering', 'crispEdges');
        this.data.Rows.forEach(row => {
          let RowDate = this.$dayjs(row[TimeCol]).tz(this.Analyzer.TimeZone);
          let Cell = (this.data.Aggreg == '1H') ? (RowDate.dayOfYear() - 1) * 24 + RowDate.hour() : (RowDate.dayOfYear() - 1) * 96 + RowDate.hour() * 4 + Math.trunc(RowDate.minute() / 15);
          this.Cells[Cell].Value = (this.Cells[Cell].Value == null) ? this.ArraySUM(row[EnergyCol]) : this.Cells[Cell].Value + this.ArraySUM(row[EnergyCol]);
          Max = Math.max(Max, Math.abs(this.Cells[Cell].Value));
        });
        this.$emit('update:max', Max);
        this.SVG.select('.points').selectAll('rect').data(this.Cells).join(
          enter => enter.append('rect')
            .attr('x', d => xScale((this.data.Aggreg == '1H') ? d.Block * 4 : d.Block))
            .attr('y', d => yScale(d.Day))
            .attr('width', (this.Width - this.MarginLeft) / ((this.data.Aggreg == '1H') ? 24 : 96))
            .attr('height', 8)
            .attr('fill', d => (d.Value == null) ? '#eee' : d3.interpolateRdYlBu(1 - ((d.Value / (2.0 * Max)) + 0.5)))
            .attr('shape-rendering', 'crispEdges')
          ,
          update => update
            .attr('x', d => xScale((this.data.Aggreg == '1H') ? d.Block * 4 : d.Block))
            .attr('y', d => yScale(d.Day))
            .attr('width', (this.Width - this.MarginLeft) / ((this.data.Aggreg == '1H') ? 24 : 96))
            .attr('fill', d => (d.Value == null) ? '#eee' : d3.interpolateRdYlBu(1 - ((d.Value / (2.0 * Max)) + 0.5)))
          ,
          exit => exit.remove()
        );
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