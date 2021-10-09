<template>
  <q-card v-show="Devices.Loading == false">
    <q-toolbar class="bg-grey-9 text-white">
      <q-toolbar-title>Device manager</q-toolbar-title>
    </q-toolbar>
    <q-table :hide-bottom="true" dense :columns="Columns" row-key="name" :rows="Devices.Objects" class="q-ma-xs">
      <template v-slot:no-data="data">
        <div class="full-width row flex-center q-gutter-sm">{{ data.message }}</div>
      </template>
      <template v-slot:body-cell-enabled="props">
        <q-td :props="props">
          <q-checkbox size="sm" disable v-model="props.row.Enabled" />
        </q-td>
      </template>
      <template v-slot:body-cell-actions="props">
        <q-td :props="props">
          <q-btn size="sm" @click="Configure(props.row)" unelevated color="primary" label="Configure" />
        </q-td>
      </template>
    </q-table>
    <q-dialog :modelValue="Driver != null" no-backdrop-dismiss @escape-key="Driver = null">
      <component :is="Driver" :settings="Settings" @close="Driver = null" />
    </q-dialog>
    <q-inner-loading :showing="ShowSpinner"><q-spinner-dots size="50px" color="orange-10" /></q-inner-loading>
  </q-card>
</template>

<script>
  import { defineAsyncComponent } from 'vue';
  let DeviceVirtual = defineAsyncComponent(() => import('./Devices/Virtual.vue'));
  let DeviceV1 = defineAsyncComponent(() => import('./Devices/V1.vue'));
  let DeviceV2 = defineAsyncComponent(() => import('./Devices/V2.vue'));
  let DeviceV3 = defineAsyncComponent(() => import('./Devices/V3.vue'));
  let DeviceV4 = defineAsyncComponent(() => import('./Devices/V4.vue'));
  let DeviceRemote = defineAsyncComponent(() => import('./Devices/Remote.vue'));
  export default {
    data() {
      return { FirstLoad: true, ShowSpinner: false, Driver: null, Settings: null, Enabled: false,
        Columns: [
          { name: 'driver', label: 'Driver', field: row => row.Driver, align: 'center', style: 'width: 80px' },
          { name: 'name', label: 'Name', field: row => row.Name, align: 'left' },
          { name: 'serial', label: 'Serial', field: row => row.Serial, align: 'center'},
          { name: 'enabled', label: 'Enabled', align: 'center', style: 'width: 60px' },
          { name: 'analyzers', label: 'Analyzers', field: row => row.Analyzers, align: 'center', style: 'width: 70px' },
          { name: 'actions', label: '', style: 'width: 80px' }
        ]
      };
    },
    watch: {
      'Devices.Loading': function(val) {
        if((val == false) && this.FirstLoad) {
          this.$emit('done');
          this.FirstLoad = false;
        }
      }
    },
    mounted: function () {
      this.Devices.RegisterListener();
      this.FirstLoad = true;
      this.$emit('queries', 1);
      if((this.Devices.Loading == false) && this.FirstLoad) {
        this.$emit('done');
        this.FirstLoad = false;
      }
    },
    unmounted: function() {
      this.Devices.UnregisterListener();
    },
    methods: {
      Configure: function(id) {
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
          () => {
            this.ShowSpinner = false;
          }
        );
      }
    },
    components: { DeviceVirtual, DeviceV1, DeviceV2, DeviceV3, DeviceV4, DeviceRemote }
  }
</script>