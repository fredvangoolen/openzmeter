<template>
  <q-card v-show="Enabled">
    <q-toolbar-title class="bg-grey-9 text-white">
      <div class="q-ml-sm">Device manager</div>
    </q-toolbar-title>
    <q-table :loading="ShowSpinner" :hide-bottom="true" dense :columns="Columns" row-key="name" :data="Devices" class="q-ma-xs">
      <template v-slot:body-cell-enabled="props">
        <q-td  :props="props">
          <q-checkbox dense disabled v-model="props.row.Enabled"></q-checkbox>
        </q-td>
      </template>
      <template v-slot:body-cell-actions="props">
        <q-td  :props="props">
          <q-btn size="xs" color="primary" @click="Configure(props.row)">Configure</q-btn>
        </q-td>
      </template>
    </q-table>
    <device-virtual v-if="Driver=='DeviceVirtual'" :settings="Settings" @close="LoadTable"></device-virtual>
    <device-v1 v-if="Driver=='DeviceV1'" :settings="Settings" @close="LoadTable"></device-v1>
    <device-v2 v-if="Driver=='DeviceV2'" :settings="Settings" @close="LoadTable"></device-v2>
    <device-v3 v-if="Driver=='DeviceV3'" :settings="Settings" @close="LoadTable"></device-v3>
    <device-v4 v-if="Driver=='DeviceV4'" ></device-v4>
    <device-remote v-if="Driver=='DeviceRemote'" :settings="Settings" @close="LoadTable"></device-remote>
  </q-card>
</template>

<script>
  import DeviceVirtual from './Devices/Virtual.vue';
  import DeviceV1 from './Devices/V1.vue';
  import DeviceV2 from './Devices/V2.vue';
  import DeviceV3 from './Devices/V3.vue';
  import DeviceV4 from './Devices/V4.vue';
  import DeviceRemote from './Devices/Remote.vue';
  export default {
    data() {
      return { Devices: [], ShowSpinner: true, Driver: null, Settings: null, Enabled: false,
        Columns: [
          {name: 'driver', label: 'Driver', field: row => row.Driver, align: 'center' },
          {name: 'name', label: 'Name', field: row => row.Name, align: 'left' },
          {name: 'serial', label: 'Serial', field: row => row.Serial, align: 'center'},
          {name: 'enabled', label: 'Enabled', align: 'center' },
          {name: 'analyzers', label: 'Analyzers', field: row => row.Analyzers, align: 'center' },
          {name: 'actions', label: '' }
        ]
      };
    },
    methods: {
      LoadTable: function (update) {
        this.Driver = null;
        if(update == false) return;
        this.ShowSpinner = true;
        this.Post('getDevices', {}).then(
          (result) => {
            this.Enabled = true;
            this.Devices = result;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      Configure: function (id) {
        this.ShowSpinner = true;
        this.Post('getDevice', { Serial: id.Serial }).then(
          (result) => {
            this.Settings = result;
            this.Settings.Serial = id.Serial;
            if(id.Driver === 'Virtual') this.Driver = 'DeviceVirtual';
            if(id.Driver === 'V1') this.Driver = 'DeviceV1';
            if(id.Driver === 'V2') this.Driver = 'DeviceV2';
            if(id.Driver === 'V3') this.Driver = 'DeviceV3';
            if(id.Driver === 'Remote') this.Driver = 'DeviceRemote';
            this.ShowSpinner = false;
          },
          (fail) => {
            this.ShowSpinner = false;
          }
        );
      }
    },
    mounted: function () {
      this.LoadTable();
    },
    components: { DeviceVirtual, DeviceV1, DeviceV2, DeviceV3, DeviceV4, DeviceRemote }
  }
</script>