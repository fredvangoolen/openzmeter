<template>
  <div class="modal" tabindex="2" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content" style="height: 100%">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">Events &raquo; Add notes</h4></div>
          <div class="pull-right btn-group">
            <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: ShowSpinner }" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save"></button>
            <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
          </div>
        </div>
        <div class="modal-body">
          <spinner v-if="ShowSpinner"></spinner>
          <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != ''">{{ Error }}</div>
          <div class="row">
            <div class="col-xs-12">
              <label class="control-label">Notes</label>
              <textarea type="text" class="form-control" v-model="Event.Notes"></textarea>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Spinner from 'Components/Spinner.vue';
  export default {
    data: function() {
      return { ShowDialog: false, ShowSpinner: false, Error: '', Event: { Notes: '' }, EventOrig: null };
    },
    created: function() {
      this.Event = null;      
    },
    mounted: function() {
      this.$root.$el.append(this.$el);
    },
    beforeDestroy: function() {
      this.$root.$el.removeChild(this.$el);
    },
    methods: {
      Close: function() {
        $(this.$el).modal('hide');
        this.ShowDialog = false;
      },
      Show: function(evt) {
        this.Event = Object.assign({}, evt);
        this.EventOrig = evt;
        this.ShowDialog = true;
        this.$nextTick(function() {
          $(this.$el).modal('show');
        });
      },
      Save: function() {
        this.ShowSpinner = true;
        var Analyzer = this.$root.SelectedAnalyzer;
        $.ajax({ url: 'SetEvent', timeout: 20000, type: 'POST', dataType: 'text', contentType: 'application/json', data: JSON.stringify( { Analyzer: Analyzer.ID, Notes: this.Event.Notes, StartTime: this.Event.StartTime, Type: this.Event.Type } ), 
          success: function(data) {
            this.ShowDialog = false;
            $(this.$el).modal('hide');
            this.EventOrig.Notes = this.Event.Notes;
          }.bind(this),
          error: function() {
            this.Error = 'Failed to save analyzer!';
            setTimeout(function() { this.Error = ''; }.bind(this), 5000); 
          }.bind(this),
          complete: function() {
            this.ShowSpinner = false;
          }.bind(this)
        });
      },
      Back: function() {
        $(this.$el).modal('hide');
        this.ShowDialog = false;
      }
    },
    components: { Spinner }
  };  
</script>

<style scoped>
  textarea {
    resize: none;
  }
</style>