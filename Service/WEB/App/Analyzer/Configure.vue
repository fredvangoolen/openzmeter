<template>
  <dlg :loading="ShowSpinner">
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
        <input type="text" class="form-control" disabled="" :value="value.Analyzer">
      </div>
      <div class="col-xs-3">
        <label class="control-label">Mode</label>
        <input type="text" class="form-control" disabled="" :value="value.Mode">
      </div>
      <div class="col-xs-12">
        <label class="control-label">Display name</label>
        <input type="text" class="form-control" v-model="value.Name" required>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Nominal voltage</label>
        <input-spin prepend="V" :list="[{name: '120 V (L-N)', value: 120}, {name: '240 V (L-L)', value: 240}, {name: '230 V (L-N)', value: 230}]" :decimals="1" :min="10" :max="10000" :increment="1" v-model="value.NominalVoltage"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Nominal frequency</label>
        <input-spin prepend="Hz" :list="[{name: 'DC', value: 0}, {name: '50 Hz', value: 50}, {name: '60 Hz', value: 60}]" :decimals="1" :min="0" :max="500" :increment="1" v-model="value.NominalFreq"></input-spin>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Min current</label>
        <input-spin prepend="A" :decimals="1" :min="0" :max="500" :increment="1" v-model="value.MinCurrent"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Max current</label>
        <input-spin prepend="A" :decimals="1" :min="0.1" :max="500" :increment="1" v-model="value.MaxCurrent"></input-spin>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable event</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="value.EventsEnabled"></toggle>
      </div>
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable alarms</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="value.AlarmsEnabled"></toggle>
      </div>
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable rates</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="value.RatesEnabled"></toggle>
      </div>
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable remote</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="value.RemoteEnabled"></toggle>
      </div>
    </div>
    <div class="row" v-if="View=='Contract'">
      <div class="col-xs-12">
        <label class="control-label">Contract ID</label>
        <input type="text" class="form-control" v-model="value.Contract">
      </div>
      <div class="col-xs-12 col-sm-9">
        <label class="control-label">Time zone</label>
        <option-picker :options="TimeZones" v-model="value.TimeZone" :big="true"></option-picker>
      </div>
      <div class="col-xs-12 col-sm-3">
        <label class="control-label">ZIP Code</label>
        <input type="text" class="form-control" v-model="value.ZipCode">
      </div>
      <div class="col-xs-12 col-sm-12">
        <label class="control-label">Location</label>
        <div style="display: flex">
          <div class="input-group" style="flex-grow: 50">
            <input-spin ref="latitude" prepend="º Lat" :decimals="7" :min="-180" :max="180" :increment="0.01" v-model="value.GeoPosition[0]"></input-spin>
          </div>
          <div class="input-group" style="flex-grow: 50; margin: 0px 5px">
            <input-spin ref="longitude" prepend="º Lon" :decimals="7" :min="-180" :max="180" :increment="0.01" v-model="value.GeoPosition[1]"></input-spin>
          </div>
          <button class="btn btn-default" v-on:click="GetLocation" title="Get current location"><img :src="require('Imgs/pin.png')"></button>
        </div>
      </div>
    </div>
    <div class="row" v-if="View=='Remote'">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable remote access</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="value.RemoteEnabled"></toggle>
        <hr style="margin: 5px 0px 5px 0px"/>
      </div>
      <div class="col-xs-12">
        <label class="control-label">Remote URL</label>
        <input type="text" class="form-control" v-model="value.RemoteServer">
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">User name</label>
        <input type="text" class="form-control" v-model="value.RemoteUser">
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Password</label>
        <input type="text" class="form-control" v-model="value.RemotePass">
      </div>
    </div>
  </dlg>
</template>

<script>
  import InputSpin from 'Components/InputSpin.vue';
  import OptionPicker from 'Components/OptionPicker.vue';
  import Toggle from 'Components/Toggle.vue';
  import Dlg from 'Components/Dialog.vue';
  import moment from 'moment';
  export default {
    props: ['value'],
    data() {
      return { TimeZones: moment.tz.names().map((val) => { return { name: val, value: val }; }), View: 'Basic', ShowSpinner: false, Error: ''};
    },
    mounted: function() {
      this.View = 'Basic';      
    },
    methods: {
      GetLocation: function() {
        navigator.geolocation.getCurrentPosition((val) => {
          this.value.GeoPosition = [val.coords.latitude, val.coords.longitude];
        });
      },
      Save: function() {
        this.ShowSpinner = true;
        $.ajax({ url: "setAnalyzer", timeout: 20000, type: "POST", contentType: 'application/json', data: JSON.stringify(this.value), 
          success: (data) => {
            this.ShowSpinner = false;
            if(this.$root.SelectedAnalyzer.Analyzer === this.value.Analyzer) {
              $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: this.value.Analyzer }), 
                success: (result) => { 
                  this.$root.SelectedAnalyzer = result;  
                  this.$root.$emit('AnalyzerChanged', result);
                },
                error: () => {
                  this.$root.$emit('AnalyzerChanged', null);
                  this.View = 'Table';
                }
              });
            }
            this.Back();
          },
          error: () => {
            this.ShowSpinner = false;
            this.Error = 'Failed to save analyzer!';
            setTimeout(() => { this.Error = ''; }, 5000);
          }
        });
      },
      Back: function() {
        this.$parent.View = null;
        this.$nextTick(() => { this.$parent.View = 'Table'; });
      }
    },
    components: { Dlg, OptionPicker, InputSpin, Toggle }
  }
</script>