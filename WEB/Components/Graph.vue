<template>
  <div class="chart"></div>
</template>

<script>
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: ['options'],
    data() {
      return { Graph: null };
    },
    methods: {
      SetOptions: function(options) {
        this.Options = Object.assign({}, this.Options, options);
        this.Graph.updateOptions(this.Options);
      },
      SetData: function(data) {
        this.Options = Object.assign({}, this.Options, { file: data });
        this.Graph.updateOptions(this.options);
      },
      Redraw: function() {
        $(this.Graph.graphDiv).parent().removeAttr('style');
      }
    },
    created: function() {
      this.Options = Object.assign({ strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], labels: ['Index', 'Value'], interactionModel: {} }, this.options);
    },
    mounted: function() {
      this.Graph = new Dygraph(this.$el, [[0, 0]], this.Options);
    }
  }
  
</script>