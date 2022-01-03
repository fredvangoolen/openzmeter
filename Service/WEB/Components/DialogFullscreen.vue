<template>
  <q-dialog :modelValue="modelValue" no-backdrop-dismiss maximized>
    <q-card>
      <q-toolbar class="text-white q-pr-none" :class="color">
        <portal-target name="dlg-toolbar" />
      </q-toolbar>
      <q-card-section>
        <portal-target name="dlg-body" />
      </q-card-section>
    </q-card>
  </q-dialog>
</template>

<script>
  import { mdiDockWindow } from '@quasar/extras/mdi-v6';
  export default {
    props: { modelValue: Boolean, color: String },
    data: function() {
      return { };
    },
    created: function() {
      this.mdiDockWindow = mdiDockWindow;
    }
  }
</script>