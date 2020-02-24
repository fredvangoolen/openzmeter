<template>
  <div class="btn-group" v-if="analyzer.Mode != '1PHASE'">
    <button type="button" @click="Toggle(0)" :class="{ active: (Active[0]==true) }" class="btn btn-default btn-xs" data-toggle="tooltip" :title="'Show phase ' + analyzer.PhaseNames[0]"><img :src="PhaseImgs[0]"></button>
    <button type="button" @click="Toggle(1)" :class="{ active: (Active[1]==true) }" class="btn btn-default btn-xs" data-toggle="tooltip" :title="'Show phase ' + analyzer.PhaseNames[1]"><img :src="PhaseImgs[1]"></button>
    <button type="button" @click="Toggle(2)" :class="{ active: (Active[2]==true) }" class="btn btn-default btn-xs" data-toggle="tooltip" :title="'Show phase ' + analyzer.PhaseNames[2]"><img :src="PhaseImgs[2]"></button>
    <button type="button" @click="Toggle(3)" :class="{ active: (Active[3]==true) }" v-if="(analyzer.Mode=='3PHASE+N') && (current==true)" class="btn btn-default btn-xs" data-toggle="tooltip" :title="'Show phase ' + analyzer.PhaseNames[3]"><img :src="PhaseImgs[3]"></button>
  </div>
</template>

<script>
  export default {
    props: { analyzer: Object, storage: String, multi: Boolean, current: Boolean, value: [Number, Array] },
    data() {
      return { Active: [], BaseImg: null };
    },
    created: function() {
      this.Configure();
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
          this.$set(this.Active, val, true);
        } else {
          this.Active = val;
        }
      },
      analyzer: function(val, old) {
        this.Configure();
      }
    },
    computed: {
      PhaseImgs: function() {
        var Imgs = [];
        for(var n in this.analyzer.PhaseNames) {
          var elem = document.createElement('canvas');
          elem.width = 16;
          elem.height = 16;
          var ctx = elem.getContext('2d');
          if(this.BaseImg != null) ctx.drawImage(this.BaseImg, 0, 0, 16, 16);
          ctx.font = "bold 12px Titillium Web";
          ctx.shadowOffsetX = 0;
          ctx.shadowOffsetY = 0;
          ctx.shadowBlur = 2;
          ctx.shadowColor = "rgba(0,0,0,0.4)";
          ctx.fillText(this.analyzer.PhaseNames[n], 0, 15);
          Imgs.push(ctx.canvas.toDataURL());
        }
        return Imgs;
      }
    },
    methods: {
      Configure: function() {
        this.Active = Array((this.analyzer.Mode == '1PHASE') ? 1 : (this.analyzer.Mode == '3PHASE') ? 3 : (this.current == true) ? 4 : 3). fill(false);
        if(this.storage != null) {
          var Mode = this.analyzer.Mode + '_' + ((this.multi == true) ? 'Multi' : 'Single') + '_' + ((this.current == true) ? 'Current' : 'Voltage');
          var Stored = JSON.parse(localStorage.getItem(this.storage));
          if((Stored != null) && (Stored[Mode] != null)) {
            if(Array.isArray(Stored[Mode]) == true) {
              this.Active = Stored[Mode];
            } else {
              this.Active[Stored[Mode]] = true;
            }
          } else {
            this.Active[0] = true;
          }
          if(this.multi == true) {
            this.$emit('input', this.Active.slice(0));
          } else {
            for(var n in this.Active) {
              if(this.Active[n] == true) this.$emit('input', parseInt(n));
            }
          }
        }
        var Img = new Image();
        Img.onload = () => { this.BaseImg = Img; }
        Img.src = require('Imgs/draw_wave16.png');
      },
      Toggle: function(val) {
        if(this.multi == false) {
          this.Active.fill(false);
          this.Active[val] = true;
          this.$emit('input', parseInt(val));
        } else {
          this.Active[val] = !this.Active[val];
          var Set = 0;
          for(var n in this.Active) {
            if(this.Active[n] == true) Set++;
          }
          if(Set == 0) this.Active[0] = true;
          this.$emit('input', this.Active.slice(0));
        }
      }
    }
  }
</script>

<style>
  .btn-default:active:hover,
  .btn-default.active:hover {
    background-color: #ADC5D9;
  }
</style>