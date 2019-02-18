<template>
  <div class="btn-group">
    <button type="button" class="btn btn-default btn-xs" v-if="options.includes('EXPORT')" @click="$emit('ExportClick')" data-toggle="tooltip" title="Export data to CSV file"><img src="Imgs/table_export.png"></button>
    <button type="button" class="btn btn-default btn-xs" v-if="options.includes('MAXIMIZE')" @click="$emit('MaximizeClick')" data-toggle="tooltip" title="Maximize graph"><img src="Imgs/arrow_out.png"></button>
    <button type="button" class="btn btn-default btn-xs" v-if="options.includes('NOW')" v-bind:class="{ active: Now, disabled: UpdateInProgress }" @click="NowClick" data-toggle="tooltip" title="Show most recent samples and update in real time"><img src="Imgs/time_go.png"></button>
    <button type="button" class="btn btn-default btn-xs" v-if="options.includes('PLAY') && !Pause" @click="PlayClick" data-toggle="tooltip" title="Resume live update"><img src="Imgs/play.png"></button>
    <button type="button" class="btn btn-default btn-xs" v-if="options.includes('PLAY') && Pause" @click="PauseClick" data-toggle="tooltip" title="Pause"><img src="Imgs/pause.png"></button>
  </div>
</template>

<script>
  export default {
    props: { options: Array, variable: String },
    data() {
      return { UpdateInProgress: false, Now: false, Pause: false };
    },
    methods: {
      NowClick() {
        this.Now = !this.Now;
        $(this.$el).children('button').blur();
        if(this.Now === true) this.WriteTo(0);
        this.$emit('NowStatus', this.Now);
      },
      ReadTo() {
        var Time = parseInt(localStorage.getItem(this.variable));
        if(isNaN(Time) === true) Time = 0;
        this.Now = (Time === 0);
        return Time;
      },
      WriteTo(time, force) {
        if((this.Now === true) && (force !== true)) time = 0;
        localStorage.setItem(this.variable, time);
        this.Now = (time === 0);
      },
      BeginUpdate() {
        this.UpdateInProgress = true;
      },
      EndUpdate() {
        this.UpdateInProgress = false;
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
      this.ReadTo();
      if(this.options.includes('NOW')) this.$emit('NowStatus', this.Now);
    }
  }
</script>
  
<style>

</style>