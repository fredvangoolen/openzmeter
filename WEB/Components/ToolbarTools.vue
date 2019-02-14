<template>
  <div class="btn-group">
    <button type="button" class="btn btn-default btn-xs" data-function="Export" data-toggle="tooltip" title="Export data to CSV file"><img src="Imgs/table_export.png"></button>
    <button type="button" class="btn btn-default btn-xs" data-function="Maximize" data-toggle="tooltip" title="Maximize graph"><img src="Imgs/arrow_out.png"></button>
    <button type="button" class="btn btn-default btn-xs active" data-function="Update" data-toggle="tooltip" title="Show most recent samples and update in real time"><img src="Imgs/time_go.png"></button>
    <button type="button" class="btn btn-default btn-xs" data-function="Maximize" data-toggle="tooltip" title="Resume"><img src="Imgs/play.png"></button>
    <button type="button" class="btn btn-default btn-xs" data-function="Maximize" data-toggle="tooltip" title="Pause"><img src="Imgs/pause.png"></button>
  </div>
</template>

<script>
  export default {
    data() {
      return {};
    }
  }
</script>
  
<style>

</style>