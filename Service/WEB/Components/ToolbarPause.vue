<template>
  <q-btn stretch dense flat @click="$emit('update:modelValue', !modelValue)" :title="modelValue ? 'Resume' : 'Pause'" :icon="modelValue ? mdiMotionPlayOutline : mdiMotionPauseOutline" />
</template>

<script>
  import { mdiMotionPauseOutline, mdiMotionPlayOutline } from '@quasar/extras/mdi-v6';
  export default {
    props: { modelValue: Boolean },
    data() {
      return { };
    },
    created: function() {
      this.mdiMotionPlayOutline = mdiMotionPlayOutline;
      this.mdiMotionPauseOutline = mdiMotionPauseOutline;
    }
  }
</script>