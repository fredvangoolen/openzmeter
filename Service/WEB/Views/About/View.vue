<template>
  <z-row>
    <div class="col-12">
      <detail-tabs :tabs="['Help', 'License', 'Changelog', 'API', /*'Updates', */'System']" storage="HelpTab" v-model="ActiveTab"></detail-tabs>
      <api v-if="ActiveTab=='API'"></api>
      <help v-if="ActiveTab=='Help'"></help>
      <license v-if="ActiveTab=='License'"></license>
      <changelog v-if="ActiveTab=='Changelog'"></changelog>
      <!--updates v-if="ActiveTab=='Updates'"></updates-->
      <system v-if="ActiveTab=='System'"></system>
    </div>
  </z-row>
</template>

<script>
  import Api from './API.vue';
  import Help from './Help.vue';
  import License from './License.vue';
  import Changelog from './Changelog.vue';
  //import Updates from './Updates.vue';
  import System from './System.vue';
  export default {
    data: function() {
      return { ActiveTab: 'Help' };
    },
    components: { Api, License, Changelog, Help, /*Updates,*/ System }
  }
</script>