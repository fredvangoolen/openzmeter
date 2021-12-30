<template>
  <q-btn stretch flat size="sm">
    <div>{{ name }}<br><strong>{{ Selected }}</strong></div>
    <q-menu auto-close>
      <q-list dense>
        <q-item clickable v-for="(option, idx) in options" :active="modelValue == option.value" :key="idx" @click="Select(option.value)">{{ option.name }}</q-item>
      </q-list>
    </q-menu>
  </q-btn>
</template>

<script>
  export default {
    props: { name: String, modelValue: [String, Number], options: Array, storage: String },
    data() {
      return { };
    },
    computed: {
      Selected: function() {
        for(var idx in this.options) {
          if(this.options[idx].value == this.modelValue) return this.options[idx].name;
        }
      }
    },
    watch: {
      options: function() {
        this.CheckOptions();
      },
      modelValue: function() {
        if((this.storage != null) && (this.modelValue != null)) localStorage.setItem(this.storage, this.modelValue);
      }
    },
    methods: {
      Select: function(val) {
        if(this.modelValue != val) this.$emit('update:modelValue', val);
      },
      CheckOptions: function() {
        if(this.options == null) return;
        let Values = [];
        this.options.forEach(val => Values.push(val.value));
        this.Select(this.GetConfig(this.storage, Values[0], Values));
        console.log("select checkoptions");
      }
    },
    created: function() {
      this.CheckOptions();
    }
  }
</script>