<template>
  <div class="panel panel-other">
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="col-12" ref="swagger"></div>
    </div>
  </div>
</template>

<script>
  import Swagger from 'swagger-ui';
  import 'swagger-ui/dist/swagger-ui.css';
  export default {
   data: function() {
     return { ShowSpinner: true };
   },
   mounted: function() {

  this.$http.get('API.json').then(
       (result) => {
         result.data.servers[0].url = document.baseURI;
         const ui = Swagger( { spec: result.data, domNode: this.$refs.swagger, layout: 'BaseLayout', deepLinking: false, displayRequestDuration: true, defaultModelsExpandDepth: 0 } );
         this.ShowSpinner = false;
       },
       (fail) => { }
     );
   }
  }
</script>

<style scoped>
  .col-12 {
    min-height: 200px;
  }
  .col-12 >>> .scheme-container {
    display: none;
  }
  .col-12 >>> pre {
    background-color: initial;
    border: initial;
  }
  .col-12 >>> .opblock-section-request-body > .opblock-section-header {
    display: none;
  }
  .panel-other {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
</style>