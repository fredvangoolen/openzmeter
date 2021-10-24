<template>
  <div style="min-height: 200px">
    <div v-html="Content" />
    <q-inner-loading :showing="ShowSpinner"><q-spinner-dots size="50px" color="orange-10" /></q-inner-loading>
  </div>
</template>

<script>
  import marked from 'marked';
  import axios from 'axios';
  export default {
   data: function() {
     return { ShowSpinner: true, Content: ''};
   },
   mounted: function() {
     axios.get('License.md').then(
       (result) => {
         this.Content = marked(result.data, { sanitize: true });
         this.ShowSpinner = false;
       },
       (fail) => { }
     );
   }
  }
</script>