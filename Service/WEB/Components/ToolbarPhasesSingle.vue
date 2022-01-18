<template>
  <q-btn stretch flat dense size="sm">
    <div>
      <div class="block">Phase</div>
      <div class="select-value"><q-icon :name="mdiCircle" :color="Options[modelValue].color" />{{ Options[modelValue].name }}</div>
    </div>
    <q-menu auto-close anchor="bottom right" self="top right">
      <q-list dense>
        <q-item v-for="(option, idx) in Options" :key="idx" clickable @click="Select(option.id)">
          <q-item-section side><q-icon :name="mdiSineWave" :color="option.color"/></q-item-section>
          <q-item-section>Phase {{ option.name }}</q-item-section>
        </q-item>
      </q-list>
    </q-menu>
  </q-btn>
</template>

<script>
  import { mdiCircle, mdiSineWave } from '@quasar/extras/mdi-v6';
  export default {
    props: { modelValue: Number, current: Boolean, storage: String },
    data() {
      return { Options: [] };
    },
    watch: {
      modelValue: function() {
        if((this.storage != null) && (this.modelValue != null)) localStorage.setItem(this.storage, JSON.stringify(this.modelValue));
      },
      'Analyzer.PhaseNames': function() { this.CheckOptions(); },
      'Analyzer.NeutralName': function() { this.CheckOptions(); }
    },
    created: function() {
      this.CheckOptions();
      this.mdiCircle = mdiCircle;
      this.mdiSineWave = mdiSineWave;
    },
    methods: {
      Select: function(val) {
        console.log(val)
        if(this.modelValue != this.Options[val].id) this.$emit('update:modelValue', this.Options[val].id);
      },
      GetConfig: function(key, def) {
        if(key == null) return 0;
        let Stored = localStorage.getItem(key);
        if(Stored == null) {
          localStorage.setItem(key, JSON.stringify(0));
          return 0;
        } else {
          Stored = Number.parseInt(Stored);
          if(Stored >= this.Options.length) {
            return Stored;
          } else {
            localStorage.setItem(key, JSON.stringify(0));
            return 0;
          }
        }
      },
      CheckOptions: function() {
        let Options = [];
        let Colors = ['red-9', 'green-5', 'blue-5'];
        if(this.Analyzer.Analyzer != null) {
          for(let phase = 0; phase < this.Analyzer.PhaseNames.length; phase++) Options.push({ color: Colors[phase], name: this.Analyzer.PhaseNames[phase], id: phase });
          if((this.Analyzer.NeutralName != null) && (this.current == true)) Options.push({ color: 'black', name: this.Analyzer.NeutralName, id: Options.length });
        }
        this.Options = Options;
        this.Select(this.GetConfig(this.storage, 0));
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
  .select-value > .q-icon {
    margin-right: 2px;
  }
</style>


