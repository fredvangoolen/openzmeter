<template>
  <div class="panel panel-other">
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <rapi-doc ref="api" regular-font="Titillium Web" mono-font="Fira Code" show-header="false" allow-server-selection="false" show-info="false" allow-authentication="false"></rapi-doc>
    </div>
  </div>
</template>

<script>
  import 'rapidoc';
  import 'swagger-ui/dist/swagger-ui.css';
  export default {
    data: function() {
      return { ShowSpinner: true };
    },
    mounted: function() {
      this.$http.get('API.json').then(
        (result) => {
          result.data.servers[0].url = document.baseURI;
          this.$refs.api.loadSpec(result.data);
          this.ShowSpinner = false;
        },
        (fail) => { }
      );
    }
  }
</script>

<style scoped>
  .panel-other {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
</style>