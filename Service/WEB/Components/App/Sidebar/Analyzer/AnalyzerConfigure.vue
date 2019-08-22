<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Configuration</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: ShowSpinner, active: View=='Basic' }" @click="View='Basic'">Basic</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: ShowSpinner, active: View=='Contract' }" @click="View='Contract'">Contract</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: ShowSpinner, active: View=='Remote' }" @click="View='Remote'">Remote</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: ShowSpinner }" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </div>
    <div class="modal-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != '' && !ShowSpinner">{{ Error }}</div>
      <div class="row" v-if="View=='Basic'">
        <div class="col-xs-9">
          <label class="control-label">Serial number</label>
          <input type="text" class="form-control" disabled :value="Analyzer.Analyzer">
        </div>
        <div class="col-xs-3">
          <label class="control-label">Mode</label>
          <input type="text" class="form-control" disabled :value="Analyzer.Mode">
        </div>
        <div class="col-xs-12">
          <label class="control-label">Display name</label>
          <input type="text" class="form-control" v-model="Analyzer.Name" required>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Nominal voltage</label>
          <input-spin prepend="V" :list="[{Text: '120 V (L-N)', Value: 120}, {Text: '240 V (L-L)', Value: 240}, {Text: '230 V (L-N)', Value: 230}]" :decimals="1" :min="10" :max="10000" :increment="1" :value="Analyzer.NominalVoltage" v-on:change="SetVoltage"></input-spin>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Nominal frequency</label>
          <input-spin prepend="Hz" :list="[{Text: 'DC', Value: 0}, {Text: '50 Hz', Value: 50}, {Text: '60 Hz', Value: 60}]" :decimals="1" :min="0" :max="500" :increment="1" :value="Analyzer.NominalFreq" v-on:change="SetFrequency"></input-spin>
        </div>
        <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Min current</label>
          <input-spin prepend="A" :decimals="1" :min="10" :max="500" :increment="1" :value="Analyzer.MinCurrent" v-on:change="SetMinCurrent"></input-spin>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Max current</label>
          <input-spin prepend="A" :decimals="1" :min="0.1" :max="500" :increment="1" :value="Analyzer.MaxCurrent" v-on:change="SetMaxCurrent"></input-spin>
        </div>      
      </div>
      <div class="row" v-if="View=='Contract'">
        <div class="col-xs-12">
          <label class="control-label">Contract ID</label>
          <input type="text" class="form-control" v-model="Analyzer.Contract">
        </div>
        <div class="col-xs-12 col-sm-9">
          <label class="control-label">Time zone</label>
          <option-picker :options="TimeZones" :selected="Analyzer.TimeZone" v-on:change="Analyzer.TimeZone=$event" :big="true"></option-picker>
        </div>
        <div class="col-xs-12 col-sm-3">
          <label class="control-label">ZIP Code</label>
          <input type="text" class="form-control" v-model="Analyzer.ZipCode">
        </div>
        <div class="col-xs-12 col-sm-12">
          <label class="control-label">Location</label>
          <div style="display: flex">
            <div class="input-group" style="flex-grow: 50">
              <input-spin ref="latitude" prepend="º Lat" :decimals="7" :min="-180" :max="180" :increment="0.01" :value="Analyzer.GeoPosition[0]" v-on:change="SetLatitude"></input-spin>
            </div>
            <div class="input-group" style="flex-grow: 50; margin: 0px 5px">
              <input-spin ref="longitude" prepend="º Lon" :decimals="7" :min="-180" :max="180" :increment="0.01" :value="Analyzer.GeoPosition[1]" v-on:change="SetLongitude"></input-spin>
            </div>
            <button class="btn btn-default" v-on:click="GetLocation" title="Get current location"><img :src="require('Imgs/pin.png')"></button>
          </div>
        </div>
      </div>
      <div class="row" v-if="View=='Remote'">
        <div class="col-xs-12">
          <label class="control-label" style="line-height: 22px">Enable remote access</label>
          <toggle class="float-right" onText="Yes" offText="No" :value="Analyzer.RemoteEnabled" v-on:change="SetRemoteEnabled"></toggle>
          <hr style="margin: 5px 0px 5px 0px"/>
        </div>
        <div class="col-xs-12">
          <label class="control-label">Remote URL</label>
          <input type="text" class="form-control" v-model="Analyzer.RemoteServer">
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">User name</label>
          <input type="text" class="form-control" v-model="Analyzer.RemoteUser">
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Password</label>
          <input type="text" class="form-control" v-model="Analyzer.RemotePass">
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Spinner from 'Components/Spinner.vue';
  import InputSpin from 'Components/InputSpin.vue';
  import OptionPicker from 'Components/OptionPicker.vue';
  import Toggle from 'Components/Toggle.vue';
  import moment from 'JS/moment.js';
  export default {
    data() {
      return { TimeZones: moment.tz.names(), View: 'Basic', Show: false, ShowSpinner: true, Analyzer: {}, Error: ''};
    },
    methods: {
      Load: function(analyzer) {
        this.Analyzer = analyzer;
        this.Show = true;
        this.View = 'Basic';
        this.ShowSpinner = false;
      },
      SetVoltage: function(val) {
        this.Analyzer.NominalVoltage = val;
      },
      SetFrequency: function(val) {
        this.Analyzer.NominalFreq = val;
      },
      SetMinCurrent: function(val) {
        this.Analyzer.MinCurrent = val;
      },
      SetMaxCurrent: function(val) {
        this.Analyzer.MaxCurrent = val;
      },
      GetLocation: function() {
        navigator.geolocation.getCurrentPosition(this.SetLocation);
      },
      SetLocation: function(val) {
        this.$refs.latitude.SetValue(val.coords.latitude);
        this.$refs.longitude.SetValue(val.coords.longitude);
      },
      SetLongitude: function(val) {
        this.Analyzer.GeoPosition[1] = val;
      },
      SetLatitude: function(val) {
        this.Analyzer.GeoPosition[0] = val;
      },
      SetRemoteEnabled: function(val) {
        this.Analyzer.RemoteEnabled = val;
      },
      Save: function() {
        this.ShowSpinner = true;
        $.ajax({ url: "setAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify(this.Analyzer), 
          success: function(data) {
            this.ShowSpinner = false;
            if(data.Result === true) {
              this.Show = false;
              this.$parent.$refs.Table.Load();
              if(this.$root.SelectedAnalyzer.Analyzer === this.Analyzer.Analyzer) {
                this.$root.SelectedAnalyzer = this.Analyzer;
                this.$root.$emit('AnalyzerChanged', this.Analyzer);
              }
            } else {
              this.Error = 'Form values are incorrect. Please, review.';
              setTimeout(function() { this.Error = ''; }.bind(this), 5000); 
            }
          }.bind(this),
          error: function() {
            this.ShowSpinner = false;
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
    components: { Spinner, OptionPicker, InputSpin, Toggle }
  }
</script>