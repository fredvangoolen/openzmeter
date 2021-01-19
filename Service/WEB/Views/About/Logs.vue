<template>
  <div class="panel panel-other">
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="col-xs-12">
        <tree-view :data="Logs" :options="{ modifiable: false, maxDepth: 2, rootObjectKey: 'LogInfo'}"></tree-view>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
   data: function() {
     return { ShowSpinner: true, Logs: {Bliu: 'blau'}};
   },
   mounted: function() {
     this.$http.get('License.md').then(
       (result) => {
         //this.Content = marked(result.data, { sanitize: true });
         this.ShowSpinner = false;
       },
       (fail) => { }
     );
   }
  }
</script>