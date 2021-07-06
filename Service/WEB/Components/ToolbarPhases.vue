<template>
  <div class="btn-group" v-if="analyzer.Mode != '1PHASE'">
    <button type="button" v-for="(item, index) in analyzer.PhaseNames" @click="Toggle(index)" :key="index" :class="{ active: (Active[index]==true) }" class="btn btn-default btn-xs" :title="'Show phase ' + item"><img :src="PhaseImgs[index]"></button>
    <button type="button" v-if="(analyzer.NeutralName!=null)&&(current==true)" @click="Toggle(analyzer.PhaseNames.length)" :class="{ active: (Active[analyzer.PhaseNames.length]==true) }" class="btn btn-default btn-xs" :title="'Show phase ' + analyzer.NeutralName"><img :src="NeutralImg"></button>
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
        var Colors = ['#d9534f', '#5cb85c', '#318ce7'];
        for(var n in this.analyzer.PhaseNames) Imgs.push(this.CreateImage(this.analyzer.PhaseNames[n], Colors[n]));
        return Imgs;
      },
      NeutralImg: function() {
        return this.CreateImage(this.analyzer.NeutralName, '#333333');
      }
    },
    methods: {
      CreateImage: function(name, color) {
        var elem = document.createElement('canvas');
        elem.width = 16;
        elem.height = 16;
        var ctx = elem.getContext('2d');
        ctx.globalAlpha = 0.5;
        if(this.BaseImg != null) ctx.drawImage(this.BaseImg, 0, 0, 16, 16);
        ctx.globalAlpha = 1.0;
        ctx.font = "bold 11px Titillium Web";
        ctx.shadowOffsetX = 0;
        ctx.shadowOffsetY = 0;
        ctx.shadowBlur = 2;
        ctx.shadowColor = "rgba(0,0,0,0.4)";
        ctx.fillText(name, 0, 15);
        if(this.multi == true) {
          ctx.beginPath();
          ctx.arc(13, 3, 2.5, 0, 2 * Math.PI, false);
          ctx.fillStyle = color;
          ctx.fill();
        }
        return ctx.canvas.toDataURL();
      },
      Configure: function() {
        this.Active = new Array(this.analyzer.PhaseNames.length).fill(false);
        if((this.analyzer.NeutralName != null) && (this.current == true)) this.Active.push(false);
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
  .btn-default:active:hover, .btn-default.active:hover {
    background-color: #ADC5D9;
  }
</style>