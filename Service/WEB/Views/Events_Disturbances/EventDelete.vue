<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Events &raquo; Delete</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs btn-danger" :disabled="ShowSpinner" @click="Delete">Delete</button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', false)"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px">Delete {{ event.length }} events?<br>Select event deletion mode</div>
    <div class="row">
      <div class="col-xs-12">
        <label>Mode:</label>
        <option-picker :options="[{name: 'All', value: 'ALL'}, {name: 'Only samples', value: 'SAMPLES_ONLY'}, {name: 'RMS samples only', value: 'SAMPLES_RMS'}, {name: 'RAW samples only', value: 'SAMPLES_RAW'}]" v-model="Mode" :big="true"></option-picker>
      </div>
    </div>
  </dlg>
</template>

<script>
  export default {
    props: { analyzer: Object, event: Array },
    data: function() {
      return { ShowSpinner: false, Mode: 'ALL' };
    },
    methods: {
      Delete: function() {
        this.ShowSpinner = true;
        var Promises = [];
        for(var n = 0; n < this.event.length; n++) Promises.push(this.Post('delEvent', { StartTime: this.event[n], Analyzer: this.analyzer.Analyzer, Mode: this.Mode }));
        Promise.all(Promises).then(
          () => { this.$emit('close', true); },
          () => { this.ShowSpinner = false; }
        );
      }
    }
  };
</script>