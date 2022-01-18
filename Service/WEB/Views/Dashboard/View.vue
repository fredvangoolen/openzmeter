<template>
  <q-page class="q-gutter-sm">
    <energy />
    <!--graphs /-->
    <variables />
  </q-page>
</template>

<script>
  import Energy from './Energy.vue';
  import Graphs from './Graphs.vue';
  import Variables from './Variables.vue';
  import 'JS/dygraph-synchronizer.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    data: function() {
      return { };
    },
    created: function() {
//      this.TransferNow.Update(this.analyzer);
//      this.Transfer3S.Update(this.analyzer, '3S', 'LastHours');
//      this.Transfer1M.Update(this.analyzer, '1M', 'LastDay');
//      this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
    },
    beforeDestroy: function() {
//      this.TransferNow.Destroy();
//      this.Transfer3S.Destroy();
//      this.Transfer1M.Destroy();
//      this.TransferGraph.Destroy();
    },
    mounted: function() {
//      this.BindGraphs();
    },
    watch: {
//      analyzer: function() {
//        this.TransferNow.Update(this.analyzer);
//        this.Transfer3S.Update(this.analyzer, '3S', 'LastHours');
//        this.Transfer1M.Update(this.analyzer, '1M', 'LastDay');
//        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
//        this.BindGraphs();
//      },
//      Aggreg: function(val, prev) {
//        if(prev != null) this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
//      },
//      Range: function() {
//        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
//      }
    },
    methods: {
//      BindGraphs: function() {
//        if(this.analyzer != null) setTimeout(() => {
//          var Graphs = [];
//          if(this.$refs.g0 != null) Graphs.push(this.$refs.g0.$refs.graph.Graph);
//          if(this.$refs.g1 != null) Graphs.push(this.$refs.g1.$refs.graph.Graph);
//          if(this.$refs.g2 != null) Graphs.push(this.$refs.g2.$refs.graph.Graph);
//          if(this.$refs.g3 != null) Graphs.push(this.$refs.g3.$refs.graph.Graph);
//          Dygraph.synchronize(Graphs, { range: false, zoom: false });
//        }, 500);
//      },
    },
    components: { Energy, Graphs, Variables }
  }
</script>