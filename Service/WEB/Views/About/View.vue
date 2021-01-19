<template>
  <div class="row">
    <div class="col-12">
      <detail-tabs :tabs="['Help', 'License', 'Changelog', 'API', 'Updates', 'Logs']" storage="HelpTab" v-model="ActiveTab"></detail-tabs>
      <api v-if="ActiveTab=='API'"></api>
      <help v-if="ActiveTab=='Help'"></help>
      <license v-if="ActiveTab=='License'"></license>
      <changelog v-if="ActiveTab=='Changelog'"></changelog>
      <updates v-if="ActiveTab=='Updates'"></updates>
      <logs v-if="ActiveTab=='Logs'"></logs>
    </div>
  </div>
</template>

<script>
  import Api from './API.vue';
  import Help from './Help.vue';
  import License from './License.vue';
  import Changelog from './Changelog.vue';
  import Updates from './Updates.vue';
  import Logs from './Logs.vue';
  export default {
    data: function() {
      return { ActiveTab: 'Help' };
    },
    components: { Api, License, Changelog, Help, Updates, Logs }
  }
</script>

<style scoped>
  .row >>> .panel {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
</style>