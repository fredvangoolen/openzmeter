<template>
  <div />
</template>

<script>
  import * as d3 from 'd3';
  export default {
    props: { width: Number, height: Number, range: Array, units: String },
    data() {
      return { };
    },
    mounted: function() {
      this.SVG = d3.select(this.$el).append('svg');
      this.SVG.append('g').attr('class', 'legend');
      this.Redraw();
    },
    watch: {
      range: function() { this.Redraw(); },
      units: function() { this.Redraw(); },
      width: function() { this.Redraw(); },
      height: function() { this.Redraw(); }
    },
    methods: {
      Redraw: function() {
        if(this.SVG == null) return;
        this.SVG.attr('width', this.width);
        this.SVG.attr('height', 12 + this.height);
        d3.select('.legend').html('');
        let Prefix = this.BestUnits(Math.max(Math.abs(this.range[0]), Math.abs(this.range[1])));
        let Min = this.Round(this.ScaleUnits(this.range[0], Prefix), 2);
        let Max = this.Round(this.ScaleUnits(this.range[1], Prefix), 2);
        for(let n = 0; n < 100; n++) {
          this.SVG.select('.legend').append("rect")
            .attr("x", (n / 100.0) * this.width)
            .attr("y", 12)
            .attr("width", this.width / 100.0)
            .attr("height", this.height)
            .attr("fill", d3.interpolateRdYlBu(1 - (n / 100.0)));
        }
        this.SVG.select('.legend')
          .append("text")
          .attr("text-anchor", "start")
          .attr("x", 1)
          .attr("y", 10).attr('font-size', 10)
          .text(Min);
        this.SVG.select('.legend')
          .append("text")
          .attr("text-anchor", "end")
          .attr("x", this.width - 1)
          .attr("y", 10).attr('font-size', 10)
          .text(Max);
        this.SVG.select('.legend')
          .append("text")
          .attr("text-anchor", "end")
          .attr("x", this.width / 2)
          .attr("y", 10).attr('font-size', 10)
          .text(Prefix + this.units);
        this.SVG.select('.legend').append("rect")
          .attr("x", 0)
          .attr("y", 12)
          .attr("width", this.width)
          .attr("height", this.height)
          .attr('fill', 'none')
          .attr("stroke", 'black');
      }
    }
  }
</script>