<template>
  <div class="panel panel-other" v-show="Enabled">
    <div class="panel-heading">
      <div class="huge-small header-text">Device manager</div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :data="Devices" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" >
            <button class="btn btn-xs btn-default" @click="Configure(props.rowData)">Configure</button>
          </div>
        </template>
      </vuetable>
    </div>
    <device-virtual v-if="Driver=='DeviceVirtual'" :settings="Settings" @close="LoadTable"></device-virtual>
    <device-v1 v-if="Driver=='DeviceV1'" :settings="Settings" @close="LoadTable"></device-v1>
    <device-v2 v-if="Driver=='DeviceV2'" :settings="Settings" @close="LoadTable"></device-v2>
    <device-v3 v-if="Driver=='DeviceV3'" :settings="Settings" @close="LoadTable"></device-v3>
    <device-v4 v-if="Driver=='DeviceV4'" ></device-v4>
    <device-remote v-if="Driver=='DeviceRemote'" :settings="Settings" @close="LoadTable"></device-remote>
  </div>
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
          {name: 'Driver', title: 'Driver'},
          {name: 'Name', title: 'Name'},
          {name: 'Serial', title: 'Serial'},
          {name: 'Enabled', title: 'Enabled', callback: this.EnabledStatus},
          {name: 'Analyzers', title: 'Analyzers'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    methods: {
      EnabledStatus: function(value) { return (value === true) ? '&#9635;' : '&#9633;'; },
      LoadTable: function (update) {
        this.Driver = null;
        if(update == false) return;
        this.ShowSpinner = true;
        this.Post('getDevices', {}).then(
          (result) => {
            var DriverCol = result[0].indexOf("Driver");
            var NameCol = result[0].indexOf("Name");
            var SerialCol = result[0].indexOf("Serial");
            var EnabledCol = result[0].indexOf("Enabled");
            var AnalyzersCol = result[0].indexOf("Analyzers");
            this.Enabled = true;
            this.Devices = [];
            this.ShowSpinner = false;
            for(var n = 1; n < result.length; n++) {
              this.Devices.push({ Driver: result[n][DriverCol], Name: result[n][NameCol], Serial: result[n][SerialCol], Enabled: result[n][EnabledCol], Analyzers: result[n][AnalyzersCol] });
            }
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

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1) {
      width: 60px;
      text-align: center;
    }
    .table >>> td:nth-child(1) {
      text-align: center;
    }
    .table >>> th:nth-child(3) {
      width: 250px;
      text-align: center;
    }
    .table >>> td:nth-child(3) {
      text-align: center;
    }
    .table >>> th:nth-child(4) {
      width: 20px;
      text-align: center;
    }
    .table >>> td:nth-child(4) {
      text-align: center;
    }
    .table >>> th:nth-child(5) {
      width: 90px;
      text-align: center;
    }
    .table >>> td:nth-child(5) {
      text-align: center;
    }
    .table >>> th:nth-child(6) {
      width: 80px;
    }
  }
  .help {
    font-size: 80%;
  }
  textarea {
    resize: none;
    font-family: 'Fira Code';
    font-size: 12px;
  }
</style>