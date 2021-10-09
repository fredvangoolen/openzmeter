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
    <q-separator />
    <div class="row q-px-sm q-col-gutter-x-sm">
      <q-input type="number" v-model.number="Settings.Voltage" label="Voltage" :rules="[ val => (val >= 10.0) && (val <= 100000.0) || 'Only numbers from 10.0 to 100000.0' ]" class="col-6">
        <template v-slot:append>V</template>
      </q-input>
      <q-input type="number" v-model="Settings.Frequency" label="Frequency" :rules="[ val => ((Number.parseFloat(val) >= 10.0) && (Number.parseFloat(val) <= 100.0)) || 'Only numbers from than 10.0 to 100.0' ]" class="col-6">
        <template v-slot:append>Hz</template>
      </q-input>
      <q-input type="number" v-model="Settings.Current" label="Current" :rules="[ val => ((Number.parseFloat(val) >= 0.1) && (Number.parseFloat(val) <= 10000.0)) || 'Only numbers from 0.1 to 10000.0' ]" class="col-6">
        <template v-slot:append>A</template>
      </q-input>
      <q-input type="number" v-model="Settings.CurrentPhase" label="Current phase" :rules="[ val => ((Number.parseFloat(val) >= -180.0) && (Number.parseFloat(val) <= 180.0)) || 'Only numbers from -180.0 to 180.0' ]" class="col-6">
        <template v-slot:append>°</template>
      </q-input>
      <q-select label="Mode" :options="[ '1PHASE', '2PHASE', '2PHASE+N', '3PHASE', '3PHASE+N' ]" v-model="Settings.Mode" class="col-6"></q-select>
      <q-input type="number" v-model="Settings.SampleRate" label="Samplerate" :rules="[ val => (Number.parseFloat(val) && (Number.parseFloat(val) >= 5000.0) && (Number.parseFloat(val) <= 1000000.0)) || 'Only numbers from 5000.0 to 1000000.0' ]" class="col-6">
        <template v-slot:append>Hz</template>
      </q-input>
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
      return { ShowSpinner: false, Error: null, Settings: null };
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
      }
    }
  }
</script>