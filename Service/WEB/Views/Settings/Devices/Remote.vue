<template>
  <div>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="value.Error != null">{{ value.Error }}</div>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enabled</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="value.Enabled"></toggle>
      </div>
      <div class="col-xs-12 help">While enabled authorized user can connect to the service. Each new shared analyzer will be added to the analyzer list on first connection. Users an permissions on this machine will be applied.</div>
    </div>
  </div>
</template>

<script>
  export default {
    props: { value: Object },
    data() {
      return { Error: '' };
    }
  }
</script>

<style scoped>
  .help {
    font-size: 80%;
  }
</style>