<template>
  <div class="btn-group">
    <button type="button" class="btn btn-default btn-xs" v-if="options.includes('EXPORT')" @click="$emit('ExportClick')" data-toggle="tooltip" title="Export data to CSV file"><img src="Imgs/table_export.png"></button>
    <button type="button" class="btn btn-default btn-xs" v-if="options.includes('MAXIMIZE')" @click="$emit('MaximizeClick')" data-toggle="tooltip" title="Maximize graph"><img src="Imgs/arrow_out.png"></button>
    <button type="button" class="btn btn-default btn-xs" v-if="options.includes('NOW')" v-bind:class="{ active: Now }" @click="NowClick" data-toggle="tooltip" title="Show most recent samples and update in real time"><img src="Imgs/time_go.png"></button>
    <button type="button" class="btn btn-default btn-xs" v-if="options.includes('PLAY') && !Pause" @click="PlayClick" data-toggle="tooltip" title="Resume live update"><img src="Imgs/play.png"></button>
    <button type="button" class="btn btn-default btn-xs" v-if="options.includes('PLAY') && Pause" @click="PauseClick" data-toggle="tooltip" title="Pause"><img src="Imgs/pause.png"></button>
  </div>
</template>

<script>
  export default {
    props: { options: Array },
    data() {
      return { Now: true, Pause: false, Conf: {} };
    },
    methods: {
      NowClick() {
        this.Now = !this.Now;
        $(this.$el).children('button').blur();
        localStorage.setItem('NowRefresh', JSON.stringify(this.Now));
      },
      PlayClick() {
        Pause = false;
        $(this.$el).children('button').blur();
        this.$emit('PlayStatus', false);
      },
      PauseClick() {
        Pause = true;
        $(this.$el).children('button').blur();
        this.$emit('PlayStatus', false);
      }
    },
    created: function() {
      this.Conf = JSON.parse(localStorage.getItem('NowRefresh'));
      this.Now = (this.Conf === true) ? true : false;
    }
  }
</script>
  
<style>

</style>