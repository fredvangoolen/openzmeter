<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Configuration</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: ShowSpinner }" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </div>
    <div class="modal-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != '' && !ShowSpinner">{{ Error }}</div>
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label">Serial number</label>
          <input type="text" class="form-control" disabled v-model="analyzer.ID">
        </div>
        <div class="col-xs-12">
          <label class="control-label">Display name</label>
          <input type="text" class="form-control" v-model="analyzer.Name" required>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Voltage</label>
          <!-- div class="input-group">
            <input type="number" class="form-control" id="inputAnalyzerVoltage" v-model="analyzer.NominalVoltage">
            <span class="input-group-addon" id="basic-addon1">V</span>
          </div -->
          <select class="form-control" v-model="analyzer.NominalVoltage">
            <option value="120">120 V</option>
            <option value="230">230 V</option>
            <option value="400">400 V</option>            
          </select>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Frequency</label>
          <!-- div class="input-group">
            <input type="number" class="form-control" id="inputAnalyzerFrequency" v-model="analyzer.NominalFreq">
            <span class="input-group-addon" id="basic-addon1">Hz</span>
          </div -->
          <select class="form-control" v-model="analyzer.NominalFreq">
            <option value="50">50 Hz</option>
            <option value="60">60 Hz</option>
          </select>
        </div>
        <!-- div class="col-xs-12 col-sm-6">
          <label class="control-label">CUPS</label>
          <input type="text" class="form-control" id="inputAnalyzerCUPS">
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Postal code</label>
          <input type="text" class="form-control" id="inputAnalyzerZIP">
        </div>
        <div class="col-xs-12 col-sm-12">
          <label class="control-label">Location</label>
          <div class="input-group">
            <span class="input-group-addon">Lat</span>
            <input type="text" class="form-control" id="inputAnalyzerLat">
            <span class="input-group-addon">Long</span>
            <input type="text" class="form-control" id="inputAnalyzerLong">
            <span class="input-group-btn"><button class="btn btn-default"><img :src="require('Imgs/globe_africa.png')"></button></span>
          </div>
        </div -->
      </div>
    </div>
  </div>
</template>

<script>
  import Spinner from 'Components/Spinner.vue';
  import OptionPicker from 'Components/OptionPicker.vue';
  export default {
    data() {
      return { Show: false, ShowSpinner: true, analyzer: null, Error: ''};
    },
    methods: {
      Load: function(analyzer) {
        this.Show = true;
        this.ShowSpinner = true;
        $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: analyzer }), 
          success: function(result) { 
            this.ShowSpinner = false;
            this.analyzer = Object.assign({}, result);
            this.Error = '';
          }.bind(this)
        });    
      },
      Save: function() {
        $.ajax({ url: "setAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify(this.analyzer), 
          success: function(data) {
            if(data.Result === true) {
              this.Show = false;
              this.$parent.$refs.Table.Load();
              if(this.$root.SelectedAnalyzer.ID === this.analyzer.ID) {
                this.$root.SelectedAnalyzer = this.analyzer;
                this.$root.$emit('AnalyzerChanged');
              }
            } else {
              this.Error = 'Form values are incorrect. Please, review.';
              setTimeout(function() { this.Error = ''; }.bind(this), 5000); 
            }
          }.bind(this),
          error: function() {
            this.Error = 'Failed to save analyzer!';
            setTimeout(function() { this.Error = ''; }.bind(this), 5000); 
          }.bind(this)
        });
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Table.Load(); 
      }
    },
    components: { Spinner, OptionPicker }
  }
</script>
  
<style>

</style>