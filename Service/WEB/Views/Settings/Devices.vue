<template>
  <q-card v-show="(Devices.Loading==false)&&(User.User=='admin')">
    <q-toolbar class="bg-grey-9 text-white">
      <q-toolbar-title>Device manager</q-toolbar-title>
    </q-toolbar>
    <q-table :hide-bottom="true" dense :columns="Columns" row-key="name" :rows="Devices.Rows" class="q-ma-xs">
      <template v-slot:no-data="data">
        <div class="full-width row flex-center q-gutter-sm">{{ data.message }}</div>
      </template>
      <template v-slot:body-cell-enabled="props">
        <q-td :props="props">
          <q-checkbox dense size="sm" disable v-model="props.row.Enabled" />
        </q-td>
      </template>
      <template v-slot:body-cell-actions="props">
        <q-td :props="props">
          <q-btn size="sm" @click="Configure(props.row)" unelevated color="primary" label="Configure" />
        </q-td>
      </template>
    </q-table>
    <q-dialog :modelValue="Device!=null" no-backdrop-dismiss @hide="Device=null" position="top">
      <component :is="Device.Driver" v-model="Device" />
    </q-dialog>
    <q-inner-loading :showing="ShowSpinner"><q-spinner-dots size="50px" color="orange-10" /></q-inner-loading>
  </q-card>
</template>

<script>
  import Virtual from './Devices/Virtual.vue';
  import DeviceV1 from './Devices/V1.vue';
  import DeviceV2 from './Devices/V2.vue';
  import DeviceV3 from './Devices/V3.vue';
  import DeviceV4 from './Devices/V4.vue';
  import Remote from './Devices/Remote.vue';
  export default {
    data: function() {
      return { ShowSpinner: false, Device: null, Devices: this.APIList(this.API.GetDevices),
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
          this.$emit('done');
      }
    },
    mounted: function () {
      this.$emit('queries', 1);
    },
    unmounted: function() {
      this.Devices.Stop();
    },
    methods: {
      Configure: function(id) {
        this.ShowSpinner = true;
        this.API.GetDevice(id.Serial).then(
          (result) => {
            this.Device = result;
            this.Device.Driver = id.Driver;
            this.ShowSpinner = false;
          },
          () => {
            this.ShowSpinner = false;
          }
        );
      }
    },
    components: { Virtual, DeviceV1, DeviceV2, DeviceV3, DeviceV4, Remote }
  }
</script>