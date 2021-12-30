<template>
  <q-btn stretch dense flat @click="$emit('input', !value)" :title="value?'Minimize graph' : 'Maximize graph'" :icon="mdiCloudUpload">
    <!--img v-if="!value" :src="require('Imgs/arrow_out.png')">
    <img v-if="value" :src="require('Imgs/arrow_in.png')"-->

  </q-btn>
</template>

<script>
  import { mdiCloudUpload } from '@quasar/extras/mdi-v6';
  export default {
    props: { value: Boolean },
    data() {
      return { };
    },
    created: function() {
      this.mdiCloudUpload = mdiCloudUpload;
    }
  }
</script>