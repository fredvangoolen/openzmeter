<template>
  <div style="min-height: 200px">
    <rapi-doc ref="api" regular-font="Titillium Web" schema-description-expanded="true" mono-font="Fira Code" show-header="false" allow-server-selection="false" show-info="false" allow-authentication="false" />
    <q-inner-loading :showing="ShowSpinner"><q-spinner-dots size="50px" color="orange-10" /></q-inner-loading>
  </div>
</template>

<script>
  import 'rapidoc';
  export default {
    data: function() {
      return { ShowSpinner: true };
    },
    mounted: function() {
      this.API.Post('getAPI', {}).then(
        (result) => {
          this.$refs.api.loadSpec(result);
          this.ShowSpinner = false;
        },
        () => { }
      );
    }
  }
</script>