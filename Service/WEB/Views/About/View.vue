<template>
 <q-page>
  <q-card>
    <q-tabs v-model="ActiveTab" dense class="text-grey" active-color="primary" indicator-color="primary" align="left" narrow-indicator>
      <q-tab name="help" label="Help"></q-tab>
      <q-tab name="license" label="License"></q-tab>
      <q-tab name="changelog" label="Changelog"></q-tab>
      <q-tab name="api" label="API"></q-tab>
      <q-tab name="system" label="System"></q-tab>
    </q-tabs>
    <q-separator></q-separator>
    <q-tab-panels v-model="ActiveTab">
      <q-tab-panel name="api" class="q-pa-sm"><api></api></q-tab-panel>
      <q-tab-panel name="license" class="q-pa-sm"><license></license></q-tab-panel>
      <q-tab-panel name="help" class="q-pa-sm"><help></help></q-tab-panel>
      <q-tab-panel name="changelog" class="q-pa-sm"><changelog></changelog></q-tab-panel>
      <q-tab-panel name="system" class="q-pa-none"><system></system></q-tab-panel>
    </q-tab-panels>
  </q-card>
 </q-page>
</template>

<script>
  import Api from './API.vue';
  import Help from './Help.vue';
  import License from './License.vue';
  import Changelog from './Changelog.vue';
  import System from './System.vue';
  export default {
    data: function() {
      return { ActiveTab: 'help' };
    },
    components: { Api, License, Changelog, Help, /*Updates,*/ System }
  }
</script>