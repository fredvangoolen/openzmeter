<!-- This file is part of openZmeter, licensed under the GNU General Public License, version 3 or (at your option) any later
     version, without any warranty. See the LICENSE file. SPDX-License-Identifier: GPL-3.0-or-later -->
<template>
  <dlg :loading="Loading">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Device &raquo; {{ Settings.Serial }}</h4></div>
      <div class="pull-right btn-group">
        <z-button @click="Save" :image="require('Imgs/diskette.png')" />
        <z-button @click="$emit('close', false)" :image="require('Imgs/arrow_undo.png')" />
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null">{{ Error }}</div>
    <z-row>
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enabled</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Settings.Enabled"></toggle>
        <hr style="margin: 5px 0px 5px 0px"/>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Serial port (RS-422 adapter, 2 Mbaud)</label>
        <input-text v-model="Settings.Port"></input-text>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Mode</label>
        <option-picker :options="[ {name: 'Auto detect', value: 'AUTO'}, {name: '1PHASE', value: '1PHASE'}, {name: '3PHASE', value: '3PHASE'}, {name: '3PHASE+N', value: '3PHASE+N'} ]" v-model="Settings.Mode"></option-picker>
      </div>
      <div class="col-xs-12" v-if="Status">
        <hr style="margin: 10px 0px 5px 0px"/>
        <table class="table table-condensed" style="margin-bottom: 0px">
          <tr><td>Port</td><td>{{ Status.Connected ? 'Open' : 'Closed' }}</td></tr>
          <tr><td>Data</td><td>{{ Status.Receiving ? 'Receiving' : 'No frames (S1 port activated by Fluvius?)' }}</td></tr>
          <tr v-if="Status.MeterID !== undefined"><td>Meter ID</td><td>{{ Status.MeterID || '-' }}</td></tr>
          <tr v-if="Status.MeterType !== undefined"><td>Meter type</td><td>{{ Status.MeterType }}{{ Status.Neutral ? ', neutral current' : '' }}</td></tr>
          <tr v-if="Status.Sampling !== undefined"><td>Sampling</td><td>{{ Status.Sampling }}</td></tr>
          <tr v-if="Status.Frequency !== undefined"><td>Frequency</td><td>{{ Status.Frequency.toFixed(3) }} Hz</td></tr>
          <tr v-if="Status.Analyzer !== undefined"><td>Analyzer</td><td>{{ Status.Analyzer }} ({{ Status.AnalyzerMode }})</td></tr>
          <tr><td>Frames OK / CRC errors / lost</td><td>{{ Status.FramesOK }} / {{ Status.CRCErrors }} / {{ Status.LostFrames }}</td></tr>
          <tr><td>Invalid samples</td><td>{{ Status.InvalidSamples }}</td></tr>
        </table>
      </div>
    </z-row>
  </dlg>
</template>

<script>
  export default {
    props: { settings: Object },
    data() {
      return { Loading: false, Error: null, Settings: null, Status: null };
    },
    created: function() {
      this.Settings = Object.assign({}, this.settings);
      this.Status = this.Settings.Status || null;
      delete this.Settings.Status;
    },
    methods: {
      Save: function () {
        this.Loading = true;
        this.Post('setDevice', this.Settings).then(
          (result) => { this.$emit('close', true); },
          (fail) => {
            this.Loading = false;
            this.Error = 'Failed to save device settings!';
            setTimeout(() => { this.Error = null; }, 1000);
          }
        );
      }
    }
  }
</script>
