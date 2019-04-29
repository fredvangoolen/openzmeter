<template>
  <div class="panel panel-default"> 
    <div class="panel-heading">By duration</div>
    <div class="panel-body">
      <graph :options="GraphOptions"></graph>
    </div>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    data: function() {
      return {};
    },
    created: function() {
      this.Durations = [[0, '20ms'], [20, '50ms'], [50, '100ms'], [100, '200ms'], [200, '500ms'], [500, '1s'], [1000, '2s'], [2000, '5s'], [5000, '10s' ], [10000, '20s' ], [20000, '50s'], [50000, '100s'], [Infinity, 'Others']];
      this.GraphOptions = {};
    },
    components: { Graph }
  };
</script>