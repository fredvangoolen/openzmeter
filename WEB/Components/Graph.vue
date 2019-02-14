<template>
  <div></div>
</template>

<script>
  import Dygraph from 'dygraphs';
  export default {
    data {
      return { Graph: null };
    },
    methods: {
      SetOptions(options) {
        if(this.Graph === null) return;
        this.Graph.updateOptions(options);
      }
    }
  }
</script>