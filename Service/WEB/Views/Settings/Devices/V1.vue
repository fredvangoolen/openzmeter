<template>
  <q-card>
    <q-card-actions class="bg-grey-9 text-white">
      <div class="text-h6">Device &raquo; {{ Settings.Serial }}</div>
    </q-card-actions>
    <q-item>
      <q-item-section>
        <q-item-label>Enabled</q-item-label>
      </q-item-section>
      <q-item-section avatar>
        <q-toggle v-model="Settings.Enabled" />
      </q-item-section>
    </q-item>
    <q-item>
      <q-item-section>
        <q-item-label>4KHz low pass filter</q-item-label>
        <q-item-label caption>Reduce noise on all input channels, input bandwidth is also reduced</q-item-label>
      </q-item-section>
      <q-item-section avatar>
        <q-toggle v-model="Settings.LowPass" />
      </q-item-section>
    </q-item>
    <q-separator />
    <div class="row q-px-sm q-col-gutter-x-sm">
      <q-input type="number" v-model.number="Settings.VoltageGain" label="Voltage channel correction" :rules="[ val => (val >= -10.0) && (val <= 10.0) || 'Only numbers from -10.0 to 10.0' ]" class="col-6">
        <template v-slot:append>%</template>
      </q-input>
    </div>

    <div class="row">
      <div class="col-xs-12">


      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Voltage channel correction</label>
        <input-spin :min="-10" :max="10" v-model="Settings.VoltageGain" :increment="0.001" :decimals="3" prepend="%"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Voltage channel offset</label>
        <div style="display: flex">
          <input-spin :min="-20" :max="20" v-model="Settings.VoltageOffset" :increment="0.01" :decimals="2" style="flex-grow: 100"></input-spin>
          <button type="button" class="btn btn-default btn-xs" @click="SetVoltageOffset" :disabled="Settings.Wave_Average.Voltage == 0.0" :title="'Last second offset: ' + Round(Settings.Wave_Average.Voltage, 2, false)"><img :src="require('Imgs/wand.png')"></button>
        </div>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Current channel correction</label>
        <div style="display: flex">
          <div style="min-width: 134px">
            <option-picker :options="[ {name: 'NO INVERT', value: false}, {name: 'INVERT', value: true}]" v-model="Settings.CurrentInvert"></option-picker>
          </div>
          <div>
            <input-spin :min="-10" :max="10" v-model="Settings.CurrentGain" :increment="0.001" :decimals="3" prepend="%"></input-spin>
          </div>
        </div>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Current channel offset</label>
        <div style="display: flex">
          <input-spin :min="-1" :max="1" v-model="Settings.CurrentOffset" :increment="0.01" :decimals="2" style="flex-grow: 100"></input-spin>
          <button type="button" class="btn btn-default btn-xs" @click="SetCurrentOffset" :disabled="Settings.Wave_Average.Current == 0.0" :title="'Last second offset: ' + Round(Settings.Wave_Average.Current, 2, false)"><img :src="require('Imgs/wand.png')"></button>
        </div>
      </div>
    </div>
    <q-separator />
    <q-card-actions align="right" class="bg-grey-2">
      <q-btn unelevated color="primary" @click="Save" label="Save" />
      <q-btn unelevated color="primary" @click="$emit('close')" label="Cancel" />
    </q-card-actions>
    <q-inner-loading :showing="ShowSpinner"><q-spinner-dots size="50px" color="orange-10"></q-spinner-dots></q-inner-loading>
  </q-card>
</template>

<script>
  export default {
    props: { settings: Object },
    data() {
      return { Error: null, ShowSpinner: false, Settings: null };
    },
    created: function() {
      this.Settings = Object.assign({}, this.settings);
    },
    methods: {
      Save: function () {
        this.ShowSpinner = true;
        this.Post('setDevice', this.Settings).then(
          (result) => {
            this.$emit('close', true);
            this.$q.notify({ message: 'Device settings saved', timeout: 3000, color: 'green', position: 'bottom-right' });
          },
          (fail) => {
            this.ShowSpinner = false;
            this.$q.notify({ message: 'Failed to save device settings!', timeout: 3000, color: 'red', position: 'bottom-right' });
          }
        );
      },
      SetCurrentOffset: function() {
        this.Settings.CurrentOffset = -this.Settings.Wave_Average.Current + this.Settings.CurrentOffset;
        this.Settings.Wave_Average.Current = 0.0;
      },
      SetVoltageOffset: function() {
        this.Settings.VoltageOffset = -this.Settings.Wave_Average.Voltage + this.Settings.VoltageOffset;
        this.Settings.Wave_Average.Voltage = 0.0;
      }
    }
  }
</script>