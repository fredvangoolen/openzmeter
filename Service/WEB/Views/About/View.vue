<template>
  <q-page>
    <q-card>
      <q-toolbar class="bg-grey-9 text-white">
        <q-tabs v-model="ActiveTab" shrink stretch active-bg-color="primary" align="left">
          <q-tab name="help" label="Help" />
          <q-tab name="license" label="License" />
          <q-tab name="changelog" label="Changelog" />
          <q-tab name="api" label="API" />
          <q-tab name="system" label="System" />
        </q-tabs>
      </q-toolbar>
      <q-tab-panels v-model="ActiveTab">
        <q-tab-panel name="api" class="q-pa-sm"><api /></q-tab-panel>
        <q-tab-panel name="license" class="q-pa-sm"><license /></q-tab-panel>
        <q-tab-panel name="help" class="q-pa-sm"><help /></q-tab-panel>
        <q-tab-panel name="changelog" class="q-pa-sm"><changelog /></q-tab-panel>
        <q-tab-panel name="system" class="q-pa-none"><system /></q-tab-panel>
      </q-tab-panels>
    </q-card>
  </q-page>
</template>

<script>
  import { defineAsyncComponent } from 'vue';
  let Api = defineAsyncComponent(() => import('./API.vue'));
  let Help = defineAsyncComponent(() => import('./Help.vue'));
  let License = defineAsyncComponent(() => import('./License.vue'));
  let Changelog = defineAsyncComponent(() => import('./Changelog.vue'));
  let System = defineAsyncComponent(() => import('./System.vue'));
  export default {
    data: function() {
      return { ActiveTab: 'help' };
    },
    components: { Api, License, Changelog, Help, System }
  }
</script>