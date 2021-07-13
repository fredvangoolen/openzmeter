<template>
  <div class="panel panel-other">
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="col-xs-12" v-html="Content"></div>
    </div>
  </div>
</template>

<script>
  import marked from 'marked';
  export default {
   data: function() {
     return { ShowSpinner: true, Content: ''};
   },
   mounted: function() {
     this.$http.get('Changelog.md').then(
       (result) => {
         this.Content = marked(result.data, { });
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