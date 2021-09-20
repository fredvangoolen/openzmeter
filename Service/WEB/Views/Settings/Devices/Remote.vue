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
              <q-item-label caption>While enabled authorized analyzers can connect to the service. Each new shared analyzer will be added to the analyzer list on first connection. Users an permissions on this machine will be applied.</q-item-label>
            </q-item-section>
            <q-item-section avatar>
              <q-toggle v-model="Settings.Enabled"></q-toggle>
            </q-item-section>
          </q-item>
        </q-list>
        <q-separator></q-separator>
        <div class="row q-px-sm">
          <q-input v-model="Filter" debounce="300" label="Add/Search analyzer" dense class="col-12">
            <template v-slot:append>
              <q-btn dense flat>Add</q-btn>
            </template>
          </q-input>
          <q-table flat dense :columns="Columns" :data="Settings.Analyzers" :filter="Filter" row-key="serial" class="col-12">
            <template v-slot:body-cell-actions="props">
              <q-td :props="props">
                <q-btn-group>
                  <q-btn size="sm" icon="delete" dense @click="Delete(props.row.Analyzer)"><q-tooltip>Delete analyzer</q-tooltip></q-btn>
                  <q-btn size="sm" icon="manage_accounts" dense @click="Delete(props.row.Analyzer)"><q-tooltip>Set analyzer owner</q-tooltip></q-btn>
                </q-btn-group>
              </q-td>
            </template>
          </q-table>
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
      return { Loading: false, Settings: null, Error: null, Filter: '',
        Columns: [
          { name: 'analyzer', label: 'Analyzer', field: row => row.Analyzer, align: 'left' },
          { name: 'user', label: 'Owner', field: row => row.User, align: 'left' },
          { name: 'Size', label: 'Size', field: row => this.BestUnitsBin(row.Size) },
          { name: 'actions', label: '' }
        ]
      };
    },
    computed: {
      CanAdd: function() {
        return (this.NewAnalyzer.match("^[0-9A-F]{24}(_CAL|_CH1|_CH2|_CH3|_CH4)?(_1P|_3P|_3PN)$") != null);
      }
    },
    created: function() {
      this.Settings = Object.assign({}, this.settings);
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
        this.Loading = true;
        this.Post('setDevice', this.Settings, { headers: { 'Accept': 'application/json', 'Content-Type': 'application/json' } }).then(
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

<style scoped>
  .table >>> th:nth-child(2) {
    width: 120px;
  }
  .table >>> th:nth-child(3) {
    width: 100px;
    text-align: right;
  }
  .table >>> td:nth-child(3) {
    text-align: right;
  }
  .table >>> th:nth-child(4) {
    width: 62px;
  }
  .table >>> td:nth-child(4) {
    text-align: right;
  }
</style>