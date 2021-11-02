<template>
  <q-btn stretch flat size="sm">
    <div>{{ name }}<br><strong>{{ Selected }}</strong></div>
    <q-menu auto-close>
      <q-list dense>
        <q-item clickable v-for="(option, idx) in options" :active="modelValue == option.value" :key="idx" @click="Select(option.value)">{{ option.name }}</q-item>
      </q-list>
    </q-menu>
      <!--span>
    <div class="btn-group">
      <div class="input-group input-group-xs">
        <span class="input-group-addon"><img :src="require('Imgs/clock_15.png')"></span>
        <div class="input-group-btn">
          <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ Selected }}<span class="caret"></span></button>
          <ul class="dropdown-menu pull-left">
            <li v-for="(option, idx) in Options" :class="{active: (value == option.value)}" :key="idx"><a v-on:click="$emit('input', option.value)">{{ option.name }}</a></li>
          </ul>
        </div>
      </div>
    </div>
    <dlg v-if="showDialog">
      <template v-slot:header>
        <h4 class="modal-title">Aggregation change</h4>
        <div class="pull-right">
          <button type="button" class="btn btn-default btn-xs" @click="showDialog=false"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
      </template>
      <p>Requested aggregation returns too much points.<br/>Zoom in and try again.</p><p>Aggregation period has been changed to the lowest allowed.</p>
    </dlg>
  </span-->

  </q-btn>
</template>

<script>
  export default {
    props: { name: String, modelValue: [String, Number], options: Array, storage: String },
    data() {
      return { showDialog: false };
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
        console.log("bliu");
      }
    },
    created: function() {
      this.CheckOptions();
    }
  }
</script>