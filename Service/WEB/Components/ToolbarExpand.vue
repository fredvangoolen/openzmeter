<template>
  <q-btn stretch dense flat @click="$emit('update:modelValue', !modelValue)" :title="modelValue ? 'Minimize' : 'Maximize'" :icon="modelValue ? mdiArrowCollapseAll : mdiArrowExpandAll" />
</template>

<script>
  import { mdiArrowCollapseAll, mdiArrowExpandAll } from '@quasar/extras/mdi-v6';
  export default {
    props: { modelValue: Boolean },
    data() {
      return { };
    },
    created: function() {
      this.mdiArrowCollapseAll = mdiArrowCollapseAll;
      this.mdiArrowExpandAll = mdiArrowExpandAll;
    }
  }
</script>