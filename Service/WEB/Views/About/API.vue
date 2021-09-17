<template>
  <div>
    <spinner v-if="ShowSpinner"></spinner>
    <rapi-doc ref="api" regular-font="Titillium Web" schema-description-expanded="true" mono-font="Fira Code" show-header="false" allow-server-selection="false" show-info="false" allow-authentication="false"></rapi-doc>
  </div>
</template>

<script>
  import 'rapidoc';
  export default {
    data: function() {
      return { ShowSpinner: true };
    },
    mounted: function() {
      this.Post('getAPI', {}).then(
        (result) => {
          this.$refs.api.loadSpec(result);
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