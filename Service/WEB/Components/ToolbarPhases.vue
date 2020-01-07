<template>
  <div class="btn-group" v-if="analyzer.Mode != '1PHASE'">
    <button type="button" @click="Toggle(0)" :class="{ active: (Active[0]==true) }" class="btn btn-default btn-xs" data-toggle="tooltip" title="Show phase R"><img :src="require('Imgs/phase_r.png')"></button>
    <button type="button" @click="Toggle(1)" :class="{ active: (Active[1]==true) }" class="btn btn-default btn-xs" data-toggle="tooltip" title="Show phase S"><img :src="require('Imgs/phase_s.png')"></button>
    <button type="button" @click="Toggle(2)" :class="{ active: (Active[2]==true) }" class="btn btn-default btn-xs" data-toggle="tooltip" title="Show phase T"><img :src="require('Imgs/phase_t.png')"></button>
    <button type="button" @click="Toggle(3)" :class="{ active: (Active[3]==true) }" v-if="(analyzer.Mode=='3PHASE+N') && (current==true)" class="btn btn-default btn-xs" data-toggle="tooltip" title="Show phase N"><img :src="require('Imgs/phase_n.png')"></button>
  </div>
</template>

<script>
  export default {
    props: { analyzer: { type: Object, required: true }, storage: { type: String, default: null }, multi: { type: Boolean, default: false }, current: { type: Boolean, dafault: false }, value: { type: Array, required: true } },
    data() {
      return { Active: [] };
    },
    created: function() {
      this.Active = Array((this.analyzer.Mode == '1PHASE') ? 1 : (this.analyzer.Mode == '3PHASE') ? 3 : (this.current == true) ? 4 : 3). fill(false);
      if(this.storage != null) {
        var Mode = this.analyzer.Mode + '_' + ((this.multi == true) ? 'Multi' : 'Single') + '_' + ((this.current == true) ? 'Current' : 'Voltage');
        var Stored = JSON.parse(localStorage.getItem(this.storage));
        if((Stored != undefined) && (Stored[Mode] != undefined)) {
          if(Array.isArray(Stored[Mode]) == true) {
           this.Active = Stored[Mode];
          } else {
            this.Active[Stored[Mode]] = true;
          }
        } else {
          this.Active[0] = true;
        }
        this.$emit('input', this.Active.slice(0));
      }
    },
    watch: {
      value: function(val) {
        var Mode = this.analyzer.Mode + '_' + ((this.multi == true) ? 'Multi' : 'Single') + '_' + ((this.current == true) ? 'Current' : 'Voltage');
        var Stored = JSON.parse(localStorage.getItem(this.storage));
        if(Stored == null) Stored = {};
        Stored[Mode] = val;
        if(this.storage != null) localStorage.setItem(this.storage, JSON.stringify(Stored));
        if(this.multi == false) {
          this.Active.fill(false);
          this.Active[val] = true;
        } else {
          this.Active = val;
        }
      },
      analyzer: function(val, old) {
        this.Active = Array((this.analyzer.Mode == '1PHASE') ? 1 : (this.analyzer.Mode == '3PHASE') ? 3 : (this.current == true) ? 4 : 3). fill(false);
      }
    },
    methods: {
      Toggle: function(val) {
        this.Active[val] = !this.Active[val];
        var Set = 0;
        for(var n in this.Active) {
          if((Set > 0) && (this.multi == false)) this.Active[n] = false;
          if(this.Active[n] == true) Set++;
        }
        if(Set == 0) this.Active[0] = true;
        if(this.multi === true) {
          this.$emit('input', this.Active.slice(0));
        } else {
          for(var n in this.Active) {
            if(this.Active[n] == true) this.$emit('input', n);
          }
        }
      },
    }
  }
</script>

<style>
  .btn-default:active:hover,
  .btn-default.active:hover {
    background-color: #ADC5D9;
  }
</style>