<template>
  <q-btn stretch flat dense size="sm">
    <div>
      <div class="block">Aggregation</div>
      <div class="block select-value">{{ Selected }}</div>
    </div>
    <q-menu auto-close anchor="bottom right" self="top right">
      <q-list dense>
        <q-item clickable v-for="(option, idx) in Options" :active="modelValue == option.value" :key="idx" @click="Select(option.value)">{{ option.name }}</q-item>
      </q-list>
    </q-menu>
  </q-btn>
</template>

<script>
  export default {
    props: { modelValue: String, options: Array, storage: String },
    data() {
      return { Options: [] };
    },
    computed: {
      Selected: function() {
        for(var idx in this.Options) {
          if(this.Options[idx].value == this.modelValue) return this.Options[idx].name;
        }
      }
    },
    watch: {
      modelValue: function() {
        if((this.storage != null) && (this.modelValue != null)) localStorage.setItem(this.storage, this.modelValue);
      }
    },
    created: function() {
      this.CheckOptions();
    },
    methods: {
      Select: function(val) {
        if(this.modelValue != val) this.$emit('update:modelValue', val);
      },
      CheckOptions: function() {
        let Options = [];
        let All = [{value: '3S',  name: '3 seconds'}, {value: '1M',  name: '1 minute'}, {value: '10M', name: '10 minutes'}, {value: '15M', name: '15 minutes'}, {value: '1H',  name: '1 hour'}, {value: '1D',  name: '1 day'}];
        if(this.options != null) All.forEach(val => { if(this.options.includes(val.value)) Options.push(val); } );
        this.Options = Options;
        this.Select(this.GetConfig(this.storage, this.options[0], this.options));
      }
    }
  }
</script>

<style scoped>
  .select-value {
    font-weight: bold;
    border-radius: 2px;
    background: rgba(255, 255, 255, 0.2);
    padding: 3px;
    margin: 0px;
  }
</style>