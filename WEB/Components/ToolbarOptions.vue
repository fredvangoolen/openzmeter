<template>
  <div class="btn-group">
    <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: Conf[name] }" v-for="(name, index) in options" @click="Toogle(index, name)">{{ name }}</button>
  </div>
</template> 

<script>
  export default {
    props: { options: Array, variable: String },
    data() {
      return { Conf: {} };
    },
    methods: {
      Toogle(index, name) {
        this.Conf[name] = !this.Conf[name];
        this.Conf = Object.assign({}, this.Conf);
        localStorage.setItem(this.variable, JSON.stringify(this.Conf));
        this.$emit('OptionChanged', name, this.Conf[name]);
        $(this.$el).children('button').blur();
      },
      AnalyzerChanged() {
        var Len = 0;
        if(this.$root.SelectedAnalyzer === null) return;
        this.Mode = (this.$root.SelectedAnalyzer === null) ? '1PHASE' : this.$root.SelectedAnalyzer.Mode;
        this.Phases = JSON.parse(localStorage.getItem(this.variable + '_' + this.Mode));
        if(Array.isArray(this.Phases) === false) this.Phases = [];
        if(this.Mode === '1PHASE') {
          Len = 1;
        } else if(this.Mode === '3PHASE') {
          Len = 3;
        } else if((this.Mode === '3PHASE+N') && (type === 'CURRENT')) {
          Len = 4;
        }
        while(this.Phases.length < Len) this.Phases.push(false);
        this.Phases = this.Phases.slice(0, Len);
        for(var n = 0; n < this.Phases.length; n++) {
          if(typeof this.Phases[n] !== "boolean") this.Phases[n] = false;
        }
        this.SelChanged();
      },
      SelChanged() {
        var Set = 0;
        for(var n = 0; n < this.Phases.length; n++) {
          if((Set > 0) && (this.multi === false)) this.Phases[n] = false;
          if(this.Phases[n] === true) Set++;
        }
        if(Set === 0) this.Phases[0] = true;
        this.PhasesR = (this.Phases[0] === true);
        this.PhasesS = (this.Phases[1] === true);
        this.PhasesT = (this.Phases[2] === true);
        this.PhasesN = (this.Phases[3] === true);
        localStorage.setItem(this.variable + '_' + this.Mode, JSON.stringify(this.Phases));
        this.$emit('PhasesChanged', this.Phases);
      }
    },
    created: function() {
      var Conf = JSON.parse(localStorage.getItem(this.variable));
      if(typeof(Conf) !== 'object') Conf = {};
      for(var n = 0; n < this.options.length; n++) {
        this.Conf[this.options[n]] = (Conf[this.options[n]] === true) ? true : false;
      }
    },
    mounted: function() {
      for(var n = 0; n < this.options.length; n++) {  
        this.$emit('OptionChanged', name, this.Conf[name]);
      }
    },
    mounted: function() {
      this.AnalyzerChanged();
    }
  }
</script>
  
<style>
  .btn-default:active:hover, 
  .btn-default.active:hover, 
  .btn-default:active:focus, 
  .btn-default.active:focus {
    background-color: #ADC5D9;
  }
</style>
