<template>
  <div class="btn-group">
    <button type="button" v-for="(i, idx) in names" :key="idx" @click="$emit('input', idx)" :class="{ active: (idx==value) }" class="btn btn-default btn-xs" data-toggle="tooltip" :title="i"><img :src="PhaseImgs[idx]"></button>
  </div>
</template>

<script>
  export default {
    props: { value: Number, names: Array, colors: Array, prefix: String },
    data() {
      return { };
    },
    computed: {
      PhaseImgs: function() {
        var Imgs = [];
        var Colors = ['#d9534f', '#5cb85c', '#318ce7', '#333333'];
        for(var n = 0; n < this.colors.length; n++) {
          var elem = document.createElement('canvas');
          elem.width = 16;
          elem.height = 16;
          var ctx = elem.getContext('2d');
          ctx.fillStyle = this.colors[n];
          ctx.arc(8, 8, 8, 0, 2 * Math.PI, false);
          ctx.fill();
          ctx.fillStyle = '#000';
          ctx.font = "bold 11px Titillium Web";
          ctx.shadowOffsetX = 0;
          ctx.shadowOffsetY = 0;
          ctx.shadowBlur = 2;
          ctx.shadowColor = "rgba(0,0,0,0.4)";
          ctx.fillText(this.prefix + (n + 1), 0, 15);
          Imgs.push(ctx.canvas.toDataURL());
        }
        return Imgs;
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