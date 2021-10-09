<template>
  <div style="min-height: 200px">
    <div v-html="Content" />
    <q-inner-loading :showing="ShowSpinner"><q-spinner-dots size="50px" color="orange-10" /></q-inner-loading>
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