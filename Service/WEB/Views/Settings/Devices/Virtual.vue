<template>
  <q-dialog :value="true" no-backdrop-dismiss @hide="$emit('close')">
    <div>
      <q-card>
        <q-card-actions class="bg-grey-9 text-white">
          <div class="text-h6">Device &raquo; {{ Settings.Serial }}</div>
        </q-card-actions>
        <q-separator></q-separator>
        <q-list>
          <q-item tag="label">
            <q-item-section>
              <q-item-label>Enabled</q-item-label>
            </q-item-section>
            <q-item-section avatar>
              <q-toggle v-model="Settings.Enabled"></q-toggle>
            </q-item-section>
          </q-item>
        </q-list>
        <q-separator></q-separator>
        <div class="row q-px-sm q-col-gutter-x-sm">
          <q-input type="number" v-model="Settings.Voltage" label="Voltage" :rules="[ val => (Number.parseFloat(val) && (Number.parseFloat(val) >= 10.0) && (Number.parseFloat(val) <= 100000.0)) || 'Only numbers from 10.0 to 100000.0' ]" class="col-6">
            <template v-slot:append>V</template>
          </q-input>
          <q-input type="number" v-model="Settings.Frequency" label="Frequency" :rules="[ val => (Number.parseFloat(val) && (Number.parseFloat(val) >= 10.0) && (Number.parseFloat(val) <= 100.0)) || 'Only numbers from than 10.0 to 100.0' ]" class="col-6">
            <template v-slot:append>Hz</template>
          </q-input>
          <q-input type="number" v-model="Settings.Current" label="Current" :rules="[ val => (Number.parseFloat(val) && (Number.parseFloat(val) >= 0.1) && (Number.parseFloat(val) <= 10000.0)) || 'Only numbers from 0.1 to 10000.0' ]" class="col-6">
            <template v-slot:append>A</template>
          </q-input>
          <q-input type="number" v-model="Settings.CurrentPhase" label="Current phase" :rules="[ val => (Number.parseFloat(val) && (Number.parseFloat(val) >= -180.0) && (Number.parseFloat(val) <= 180.0)) || 'Only numbers from -180.0 to 180.0' ]" class="col-6">
            <template v-slot:append>°</template>
          </q-input>
          <q-select label="Mode" :options="[ '1PHASE', '2PHASE', '2PHASE+N', '3PHASE', '3PHASE+N' ]" v-model="Settings.Mode" class="col-6"></q-select>
          <q-input type="number" v-model="Settings.SampleRate" label="Samplerate" :rules="[ val => (Number.parseFloat(val) && (Number.parseFloat(val) >= 5000.0) && (Number.parseFloat(val) <= 1000000.0)) || 'Only numbers from 5000.0 to 1000000.0' ]" class="col-6">
            <template v-slot:append>Hz</template>
          </q-input>
        </div>
        <q-separator></q-separator>
        <q-card-actions align="right" class="bg-grey-2">
          <q-btn @click="Save">Save</q-btn>
          <q-btn @click="$emit('close')">Cancel</q-btn>
        </q-card-actions>
      </q-card>
      <q-inner-loading :showing="Loading"><q-spinner-dots size="50px" color="orange-10"></q-spinner-dots></q-inner-loading>
    </div>
  </q-dialog>
</template>

<script>
  export default {
    props: { settings: Object },
    data() {
      return { Loading: false, Error: null, Settings: null };
    },
    created: function() {
      this.Settings = Object.assign({}, this.settings);
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