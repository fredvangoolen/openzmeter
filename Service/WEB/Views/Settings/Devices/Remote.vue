<template>
  <q-card>
    <q-card-actions class="bg-grey-9 text-white">
      <q-toolbar-title>Device &raquo; {{ Settings.Device }}</q-toolbar-title>
    </q-card-actions>
    <q-toolbar class="bg-grey-3 q-pa-none">
      <q-input v-model="Filter" debounce="300" label="Add/Search analyzer" dense class="q-px-sm q-py-xs full-width" />
      <q-btn stretch unelevated color="primary" :disable="!CanAdd" label="Add" />
    </q-toolbar>
    <q-separator />
    <q-card-section class="row q-px-sm q-col-gutter-x-sm">
      <q-item class="col-12">
        <q-item-section>
          <q-item-label>Enabled</q-item-label>
          <q-item-label caption>While enabled authorized analyzers can connect to the service. Each new shared analyzer will be added to the analyzer list on first connection. Users an permissions on this machine will be applied.</q-item-label>
        </q-item-section>
        <q-item-section avatar>
          <q-toggle v-model="Settings.Enabled"></q-toggle>
        </q-item-section>
      </q-item>
      <q-table flat dense :columns="Columns" :data="Settings.Analyzers" :filter="Filter" row-key="serial" class="col-12">
        <template v-slot:no-data="data">
          <div class="full-width row flex-center q-gutter-sm">{{ data.message }}</div>
        </template>
        <template v-slot:body-cell-actions="props">
          <q-td :props="props">
            <q-btn-group>
              <q-btn size="sm" icon="delete" dense @click="Delete(props.row.Analyzer)"><q-tooltip>Delete analyzer</q-tooltip></q-btn>
              <q-btn size="sm" icon="manage_accounts" dense @click="Delete(props.row.Analyzer)"><q-tooltip>Set analyzer owner</q-tooltip></q-btn>
            </q-btn-group>
          </q-td>
        </template>
      </q-table>
    </q-card-section>
    <q-separator />
    <q-card-actions align="right" class="bg-grey-3">
      <q-btn unelevated color="primary" @click="Save" label="Save" />
      <q-btn unelevated color="primary" @click="$emit('update:modelValue', null)" label="Cancel" />
    </q-card-actions>
    <q-inner-loading :showing="ShowSpinner"><q-spinner-dots size="50px" color="orange-10"></q-spinner-dots></q-inner-loading>
  </q-card>
</template>

<script>
  export default {
    props: { settings: Object },
    data() {
      return { ShowSpinner: false, Settings: null, Filter: '',
        Columns: [
          { name: 'analyzer', label: 'Analyzer', field: row => row.Analyzer, align: 'left' },
          { name: 'user', label: 'Owner', field: row => row.User, align: 'left' },
          { name: 'Size', label: 'Size', field: row => this.BestUnitsBin(row.Size) },
          { name: 'actions', label: '' }
        ]
      };
    },
    created: function() {
      this.Settings = Object.assign({}, this.settings);
    },
    computed: {
      CanAdd: function() {
        return (this.Filter.match("^[0-9A-F]{24}(_CAL|_CH1|_CH2|_CH3|_CH4)?(_1P|_3P|_3PN)$") != null);
      }
    },
    methods: {
      Add: function() {
        for(var n = 0; n < this.Settings.Analyzers.length; n++) {
          if(this.Settings.Analyzers[n].Analyzer == this.NewAnalyzer) {
            this.NewAnalyzer = '';
            return;
          }
        }
        this.Settings.Analyzers.push({ User: 'admin', Analyzer: this.NewAnalyzer, Size: 0 });
        this.NewUser = '';
        this.NewAnalyzer = '';
      },
      Delete: function(row) {
        if(confirm("Delete '" + row + "' analyzer?") == false) return;
        for(var n = 0; n < this.Settings.Analyzers.length; n++) {
          if(this.Settings.Analyzers[n].Analyzer != row) continue;
          this.Settings.Analyzers.splice(n, 1);
          return;
        }
      },
      Save: function () {
        this.ShowSpinner = true;
        this.API.SetDevice(this.Settings).then(
          () => {
            this.Visible = false;
            this.$q.notify({ message: 'Device settings saved', timeout: 3000, color: 'green', position: 'bottom-right' });
          },
          () => {
            this.ShowSpinner = false;
            this.$q.notify({ message: 'Failed to save device settings!', timeout: 3000, color: 'red', position: 'bottom-right' });
          }
        );
      }
    }
  }
</script>