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
        this.SVG.html('');
        let Prefix = this.BestUnits(Math.max(Math.abs(this.range[0]), Math.abs(this.range[1])));
        let Min = this.Round(this.ScaleUnits(this.range[0], Prefix), 2);
        let Max = this.Round(this.ScaleUnits(this.range[1], Prefix), 2);
        this.SVG.append("text")
          .attr("text-anchor", "start")
          .attr("x", 1)
          .attr("y", 10)
          .attr('font-size', 10)
          .attr('fill', 'black')
          .text(Min);
        this.SVG.append("text")
          .attr("text-anchor", "end")
          .attr("x", this.width - 1)
          .attr("y", 10)
          .attr('font-size', 10)
          .attr('fill', 'black')
          .text(Max);
        this.SVG.append("text")
          .attr("text-anchor", "center")
          .attr("x", this.width / 2)
          .attr("y", 10)
          .attr('font-size', 10)
          .attr('fill', 'black')
          .text(Prefix + this.units);
        for(let n = 0; n < this.width; n++) this.SVG.append("rect")
          .attr("x", n)
          .attr("y", 12)
          .attr("width", 1)
          .attr("height", this.height)
          .attr("fill", d3.interpolateRdYlBu(1 - (n / this.width)));
        this.SVG.append("rect")
          .attr("x", 0)
          .attr("y", 12)
          .attr("width", this.width - 1)
          .attr("height", this.height)
          .attr("stroke", 'black')
          .attr('fill', 'rgba(0,0,0,0)')
          .attr('stroke-width', 0.5);
      }
    }
  }
</script>