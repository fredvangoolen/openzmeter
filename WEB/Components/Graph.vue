<template>
  <div class="chart"></div>
</template>

<script>
  import Dygraph from 'JS/dygraph.js';
  export default {
    multiColumnBarPlotter: function(e) {
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
        ctx.fillStyle = g.getColors()[j];
        for(var i = 0; i < sets[j].length; i++) {
          var p = sets[j][i];
          var x_left = p.canvasx - (subbar_width * sets.length / 2) + j * subbar_width;
          ctx.fillRect(x_left, p.canvasy, bar_width / sets.length, y_bottom - p.canvasy);
          ctx.strokeRect(x_left, p.canvasy, bar_width / sets.length, y_bottom - p.canvasy);
        }
      }
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
      this.Options = Object.assign({ strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], labels: ['Index', 'Value'], interactionModel: {} }, this.options);
    },
    mounted: function() {
      this.Graph = new Dygraph(this.$el, [[0, 0]], this.Options);
    },
    beforeDestroy: function() {
      this.Graph.destroy();
    }
  }
  
</script>