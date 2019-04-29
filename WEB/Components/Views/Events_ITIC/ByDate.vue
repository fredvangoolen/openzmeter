<template>
  <div class="panel panel-default"> 
    <div class="panel-heading">By date</div>
    <div class="panel-body">
      <graph></graph>
    </div>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    data: function() {
      return {};
    },
    components: { Graph }
  };
</script>