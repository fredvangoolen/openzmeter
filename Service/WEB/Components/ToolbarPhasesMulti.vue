<template>
  <q-btn stretch flat dense size="sm">
    <div>
      <div class="block">Phases</div>
      <div class="block">
        <div class="select-value" v-for="(option, idx) in Selected" :key="idx"><q-icon :name="mdiCircle" :color="option.color" />{{ option.name }}</div>
      </div>
    </div>
    <q-menu auto-close anchor="bottom middle" self="top middle">
      <q-list dense>
        <q-item v-for="(option, idx) in Options" :key="idx">
          <q-item-section side><q-icon :name="mdiSineWave" :color="option.color"/></q-item-section>
          <q-item-section>Phase {{ option.name }}</q-item-section>
          <q-item-section side><q-toggle :modelValue="modelValue[idx]" :disable="option.disabled" @update:modelValue="Select($event, idx)" /></q-item-section>
        </q-item>
      </q-list>
    </q-menu>
  </q-btn>
</template>

<script>
  import { mdiCircle, mdiSineWave } from '@quasar/extras/mdi-v6';
  export default {
    props: { modelValue: Array, storage: String, current: Boolean },
    data() {
      return { ValidOptions: [] };
    },
    created: function() {
      this.mdiCircle = mdiCircle;
      this.mdiSineWave = mdiSineWave;
    },
    computed: {
      Selected: function() {
        let Sel = [];
        for(let phase = 0; phase < this.Options.length; phase++) {
          if(this.modelValue[phase] == true) Sel.push({ color: this.Options[phase].color, name: this.Options[phase].name });
        }
        return Sel;
      },
      Options: function() {
        let Options = [];
        let Colors = ['red-9', 'green-5', 'blue-5'];
        let Default = [];
        if(this.Analyzer.Analyzer != null) {
          for(let phase = 0; phase < this.Analyzer.PhaseNames.length; phase++) {
            Options.push({ color: Colors[phase], name: this.Analyzer.PhaseNames[phase], disabled: false });
            Default.push((phase == 0));
          }
          if((this.Analyzer.NeutralName != null) && (this.current == true)) {
            Options.push({ color: 'black', name: this.Analyzer.NeutralName, disabled: false });
            this.Active.push(false);
          }
        }
        this.ValidOptions = Options;
        let Conf = this.GetConfig(Default);
        let Change = false;
        if(this.modelValue.length == Conf.length) {
          for(let n = 0; n < Conf.length; n++) {
            if(this.modelValue[n] != Conf[n]) Change = true;
          }
        }
        if((Change == true) || (this.modelValue.length != Conf.length)) this.$emit('update:modelValue', Conf);
        return Options;
      }
    },
    watch: {
      modelValue: function() {
        if((this.storage != null) && (this.modelValue.length > 0)) localStorage.setItem(this.storage + ((this.current == true) ? '.C' : '.V') + 'M' + this.ValidOptions.length, JSON.stringify(this.modelValue));
      }
    },
    methods: {
      Select: function(val, idx) {
        if((this.modelValue.length == this.ValidOptions.length) && (this.modelValue[idx] != val)) {
          let Sel = [];
          let Active = 0;
          for(let phase = 0; phase < this.ValidOptions.length; phase++) {
            Sel.push((phase == idx) ? val : this.modelValue[phase]);
            if(Sel[phase] == true) Active++;
          }
          for(let phase = 0; phase < this.Options.length; phase++) this.ValidOptions[phase].disabled = ((Active == 1) && (Sel[phase] == true)) ? true : false;
          this.$emit('update:modelValue', Sel);
        }
      },
      GetConfig: function(def) {
        if(this.storage == null) return def;
        let Key = this.storage + ((this.current == true) ? '.C' : '.V') + 'M' + this.ValidOptions.length;
        let Stored = null;
        try {
          Stored = JSON.parse(localStorage.getItem(Key));
        } catch(error) {}
        if(Stored == null) {
          localStorage.setItem(Key, JSON.stringify(def));
          return def;
        } else {
          if(Stored.length != this.ValidOptions.length) {
            return def;
          } else {
            let Active = 0;
            for(let phase = 0; phase < this.ValidOptions.length; phase++) {
              if(typeof Stored[phase] != "boolean") return def;
              if(Stored[phase] == true) Active++;
            }
            if(Active == 0) return def;
            return Stored;
          }
        }
      }
    }
  }
</script>

<style scoped>
  .select-value {
    display: inline-block;
    font-weight: bold;
    border-radius: 2px;
    background: rgba(255, 255, 255, 0.2);
    padding: 3px;
    margin-right: 1px;
  }
  .select-value:last-child {
    margin-right: 0px;
  }
  .select-value > .q-icon {
    margin-right: 2px;
  }
</style>