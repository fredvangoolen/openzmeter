<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Events &raquo; Add notes</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" :disabled="ShowSpinner" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', null)"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null">{{ Error }}</div>
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label">Notes</label>
          <textarea type="text" class="form-control" v-model="Event.Notes"></textarea>
        </div>
      </div>
    </div>
  </dlg>
</template>

<script>
  export default {
    props: { analyzer: Object, event: Object },
    data: function() {
      return { ShowSpinner: false, Error: null, Event: null };
    },
    created: function() {
      this.Event = Object.assign({}, this.event);
    },
    methods: {
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post('setEvent', { Analyzer: this.analyzer.Analyzer, Notes: this.Event.Notes, StartTime: this.Event.StartTime, Type: this.Event.Type } ).then(
          (result) => { this.$emit('close', this.Event); },
          (fail) =>  {
            this.Error = 'Failed to save analyzer!';
            this.ShowSpinner = false;
            setTimeout(function() { this.Error = null; }.bind(this), 2000);
          }
        );
      }
    }
  };  
</script>

<style scoped>
  textarea {
    resize: none;
  }
</style>