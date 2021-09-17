<template>
  <div>
    <spinner v-if="ShowSpinner"></spinner>
    <div v-html="Content"></div>
  </div>
</template>

<script>
  import marked from 'marked';
  export default {
   data: function() {
     return { ShowSpinner: true, Content: ''};
   },
   mounted: function() {
     this.$http.get('License.md').then(
       (result) => {
         this.Content = marked(result.data, { sanitize: true });
         this.ShowSpinner = false;
       },
       (fail) => { }
     );
   }
  }
</script>

<style scoped>
  .panel-body {
    padding: 20px;
    text-align: justify;
  }
  .panel-body >>> p {
    font-size: 110%;
  }
  .panel-other {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
</style>