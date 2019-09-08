<template>
  <dlg ref="Dialog">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Configuration</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: ShowSpinner }" v-on:click="Save"><img :src="require('Imgs/diskette.png')" alt="Save"></button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: ShowSpinner, active: View=='Basic' }" v-on:click="View='Basic'">Basic</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: ShowSpinner, active: View=='Contract' }" v-on:click="View='Contract'">Contract</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: ShowSpinner, active: View=='Remote' }" v-on:click="View='Remote'">Remote</button>
        <button type="button" class="btn btn-default btn-xs" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != '' && !ShowSpinner">{{ Error }}</div>
    <div class="row" v-if="View=='Basic'">
      <div class="col-xs-9">
        <label class="control-label">Serial number</label>
        <input type="text" class="form-control" disabled="" :value="Analyzer.Analyzer">
      </div>
      <div class="col-xs-3">
        <label class="control-label">Mode</label>
        <input type="text" class="form-control" disabled="" :value="Analyzer.Mode">
      </div>
      <div class="col-xs-12">
        <label class="control-label">Display name</label>
        <input type="text" class="form-control" v-model="Analyzer.Name" required>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Nominal voltage</label>
        <input-spin prepend="V" :list="[{Text: '120 V (L-N)', Value: 120}, {Text: '240 V (L-L)', Value: 240}, {Text: '230 V (L-N)', Value: 230}]" :decimals="1" :min="10" :max="10000" :increment="1" :value="Analyzer.NominalVoltage" v-on:change="Analyzer.NominalVoltage=$event"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Nominal frequency</label>
        <input-spin prepend="Hz" :list="[{Text: 'DC', Value: 0}, {Text: '50 Hz', Value: 50}, {Text: '60 Hz', Value: 60}]" :decimals="1" :min="0" :max="500" :increment="1" :value="Analyzer.NominalFreq" v-on:change="Analyzer.NominalFreq=$event"></input-spin>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Min current</label>
        <input-spin prepend="A" :decimals="1" :min="0" :max="500" :increment="1" :value="Analyzer.MinCurrent" v-on:change="Analyzer.MinCurrent=$event"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Max current</label>
        <input-spin prepend="A" :decimals="1" :min="0.1" :max="500" :increment="1" :value="Analyzer.MaxCurrent" v-on:change="Analyzer.MaxCurrent=$event"></input-spin>
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
            <input-spin ref="latitude" prepend="º Lat" :decimals="7" :min="-180" :max="180" :increment="0.01" :value="Analyzer.GeoPosition[0]" v-on:change="Analyzer.GeoPosition[0]=$event"></input-spin>
          </div>
          <div class="input-group" style="flex-grow: 50; margin: 0px 5px">
            <input-spin ref="longitude" prepend="º Lon" :decimals="7" :min="-180" :max="180" :increment="0.01" :value="Analyzer.GeoPosition[1]" v-on:change="Analyzer.GeoPosition[1]=$event"></input-spin>
          </div>
          <button class="btn btn-default" v-on:click="GetLocation" title="Get current location"><img :src="require('Imgs/pin.png')"></button>
        </div>
      </div>
    </div>
    <div class="row" v-if="View=='Remote'">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable remote access</label>
        <toggle class="float-right" onText="Yes" offText="No" :value="Analyzer.RemoteEnabled" v-on:change="Analyzer.RemoteEnabled=$event"></toggle>
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
  </dlg>
</template>

<script>
  import InputSpin from 'Components/InputSpin.vue';
  import OptionPicker from 'Components/OptionPicker.vue';
  import Toggle from 'Components/Toggle.vue';
  import Dlg from 'Components/Dialog.vue';
  import moment from 'JS/moment.js';
  export default {
    data() {
      return { TimeZones: moment.tz.names(), View: 'Basic', ShowSpinner: false, Analyzer: {}, Error: ''};
    },
    methods: {
      Show: function(analyzer) {
        this.Analyzer = analyzer;
        this.$refs.Dialog.Show();
      },
      GetLocation: function() {
        navigator.geolocation.getCurrentPosition(this.SetLocation);
      },
      SetLocation: function(val) {
        this.$refs.latitude.SetValue(val.coords.latitude);
        this.$refs.longitude.SetValue(val.coords.longitude);
      },
      Save: function() {
        this.$refs.Dialog.ShowSpinner = true;
        $.ajax({ url: "setAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify(this.Analyzer), 
          success: (data) => {
            this.$refs.Dialog.ShowSpinner = false;
            if(data.Result === true) {
              if(this.$root.SelectedAnalyzer.Analyzer === this.Analyzer.Analyzer) {
                this.$root.SelectedAnalyzer = this.Analyzer;
                this.$root.$emit('AnalyzerChanged', this.Analyzer);
              }
              this.Back();
            } else {
              this.Error = 'Form values are incorrect. Please, review.';
              setTimeout(() => { this.Error = ''; }, 5000); 
            }
          },
          error: () => {
            this.$refs.Dialog.ShowSpinner = false;
            this.Error = 'Failed to save analyzer!';
            setTimeout(() => { this.Error = ''; }, 5000);
          }
        });
      },
      Back: function() {
        this.$refs.Dialog.Close();
        this.$parent.$refs.Dialog.Show();
      }
    },
    components: { Dlg, OptionPicker, InputSpin, Toggle }
  }
</script>