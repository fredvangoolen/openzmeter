<template>
  <div class="btn-group" v-if="Mode != '1PHASE'">
    <button type="button" @click="Toogle(0)" v-bind:class="{ active: PhasesR }" class="btn btn-default btn-xs" data-toggle="tooltip" title="Show phase R"><img src="Imgs/phase_r.png"></button>
    <button type="button" @click="Toogle(1)" v-bind:class="{ active: PhasesS }" class="btn btn-default btn-xs" data-toggle="tooltip" title="Show phase S"><img src="Imgs/phase_s.png"></button>
    <button type="button" @click="Toogle(2)" v-bind:class="{ active: PhasesT }" class="btn btn-default btn-xs" data-toggle="tooltip" title="Show phase T"><img src="Imgs/phase_t.png"></button>
    <button type="button" @click="Toogle(3)" v-bind:class="{ active: PhasesN }" v-if="(Mode == '3PHASE+N') && (type == 'CURRENT')" class="btn btn-default btn-xs" data-toggle="tooltip" title="Show phase N"><img src="Imgs/phase_n.png"></button>
  </div>
</template> 

<script>
  export default {
    props: { variable: String, multi: Boolean, type: String },
    data() {
      return { Phases: [], PhasesR: false, PhasesS: false, PhasesT: false, PhasesN: false, Mode: '1PHASE' };
    },
    methods: {
      Toogle(pos) {
        $(this.$el).children()[pos].blur();
        if(this.multi === false) this.Phases.fill(false);
        this.Phases[pos] = !this.Phases[pos];
        this.SelChanged();
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
      this.$root.$on('AnalyzerChanged', this.AnalyzerChanged);
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