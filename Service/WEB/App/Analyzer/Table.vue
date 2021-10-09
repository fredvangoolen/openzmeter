<template>
  <q-dialog :modelValue="true" no-backdrop-dismiss @escape-key="$emit('close')">
    <q-card v-if="View==null">
      <q-toolbar class="text-white q-pr-none bg-grey-9">
        <q-toolbar-title>Analyzers</q-toolbar-title>
        <q-tabs stretch v-model="Tab" active-bg-color="primary">
          <q-tab name="all" label="All" />
          <q-tab name="selected" label="Selected" />
        </q-tabs>
      </q-toolbar>
      <q-card-section v-if="Tab == 'all'" class="q-pa-none">
        <q-toolbar class="bg-grey-2 q-pa-none">
          <q-space />
          <q-input dense debounce="300" label="Search" v-model="Filter" class="q-mr-sm">
            <template v-slot:prepend><q-icon name="search" /></template>
          </q-input>
        </q-toolbar>
        <q-separator />
        <div class="text-center col-12" v-if="Analyzers.Objects.length == 0">
          <p>There are no analyzers available.</p>
          <p>If you are an administrator, check device settings, otherwise request permission.</p>
        </div>
        <q-table v-else flat dense :columns="Columns" :rows="Analyzers.Objects" :filter="Filter" row-key="analyzer" class="q-ma-xs">
          <template v-slot:no-data>
            <div class="full-width row flex-center q-gutter-sm">There are no analyzers available.<br />If you are an administrator, check device settings, otherwise request permission.</div>
          </template>
          <template v-slot:body-cell-state="props">
            <q-td :props="props">
              <img :src="require('Imgs/bullet_green.png')" v-if="props.row.State=='ONLINE'">
              <img :src="require('Imgs/bullet_yellow.png')" v-if="props.row.State=='SYNCING'">
              <img :src="require('Imgs/bullet_red.png')" v-if="props.row.State=='OFFLINE'">
            </q-td>
          </template>
            <template v-slot:body-cell-actions="props">
              <q-td :props="props">
                <q-btn-dropdown size="sm" color="primary" split label="Select" @click="Select(props.row.Analyzer, true)" :disable-main-btn="((analyzer != null) && (analyzer.Analyzer == props.row.Analyzer))">
                <q-list dense>
                  <q-item clickable @click="LoadView('Alarms', props.row.Analyzer)">Alarms and events</q-item>
                  <q-item clickable @click="LoadView('Configure', props.row.Analyzer)">Configure</q-item>
                  <q-item clickable @click="LoadView('Permissions', props.row.Analyzer)">Permissions</q-item>
                  <q-item clickable @click="LoadView('Rates', props.row.Analyzer)">Rates</q-item>
                  <q-item clickable @click="LoadView('Remote', props.row.Analyzer)">Remote access</q-item>
                  <q-separator />
                  <q-item clickable @click="LoadView('Maintenance', props.row.Analyzer)">Maintenance</q-item>
                </q-list>
                </q-btn-dropdown>
              <!--div class="btn-group">
            <button type="button" class="btn btn-success btn-xs" :disabled="((analyzer != null) && (analyzer.Analyzer == props.rowData.Analyzer))" >Select</button>
            <button type="button" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown"><span class="caret"></span></button>
            <ul class="dropdown-menu pull-right">
              <li><a @click="LoadView('Alarms', props.rowData.Analyzer)">Alarms and events</a></li>
              <li><a @click="LoadView('Configure', props.rowData.Analyzer)">Configure</a></li>
              <li><a @click="LoadView('Permissions', props.rowData.Analyzer)">Permissions</a></li>
              <li><a @click="LoadView('Rates', props.rowData.Analyzer)">Rates</a></li>
              <li><a @click="LoadView('Remote', props.rowData.Analyzer)">Remote access</a></li>
              <li role="separator" class="divider"></li>
              <li><a @click="LoadView('Maintenance', props.rowData.Analyzer)">Maintenance</a></li>
            </ul>
          </div-->
              </q-td>
            </template>
          </q-table>
        </q-card-section>
        <q-separator color="grey-3"></q-separator>
      <q-card-actions align="right" class="bg-grey-3">
          <q-btn unelevated color="red-9" @click="$emit('close')">Close</q-btn>
      </q-card-actions>
      <q-inner-loading :showing="Analyzers.Loading || ShowSpinner"><q-spinner-dots size="50px" color="orange-10"></q-spinner-dots></q-inner-loading>
    </q-card>
    <analyzer-configure v-if="View=='Configure'" :analyzer="Analyzer" @close="CloseSelect($event)"></analyzer-configure>
    <analyzer-alarms v-if="View=='Alarms'" :analyzer="Analyzer" @close="CloseSelect($event)"></analyzer-alarms>
    <analyzer-permissions v-if="View=='Permissions'" :analyzer="Analyzer" @close="CloseSelect($event)"></analyzer-permissions>
    <analyzer-remote v-if="View=='Remote'" :analyzer="Analyzer" @close="CloseSelect($event)"></analyzer-remote>
    <analyzer-rates v-if="View=='Rates'" :analyzer="Analyzer" @close="CloseSelect($event)"></analyzer-rates>
    <analyzer-maintenance v-if="View=='Maintenance'" :analyzer="Analyzer" @close="CloseSelect($event)"></analyzer-maintenance>
        <!--analyzer-info v-if="Dialog=='Info'" :analyzer="analyzer" @close="Dialog=null"></analyzer-info-->

  </q-dialog>
</template>

<script>
  import AnalyzerConfigure from './Configure.vue';
  import AnalyzerPermissions from './Permissions.vue';
  import AnalyzerRates from './Rates.vue';
  import AnalyzerMaintenance from './Maintenance.vue';
  import AnalyzerAlarms from './Alarms.vue';
  import AnalyzerRemote from './Remote.vue';
  export default {
    data() {
      return { Tab: 'all', View: null, Filter: '', ShowSpinner: false, Analyzer: null,
        Columns: [
          { name: 'state', label: '', width: '20px', titleClass: 'text-center', dataClass: 'text-center' },
          { name: 'user', label: 'Owner', field: row => row.User },
          { name: 'name', label: 'Description', field: row => row.Name },
          { name: 'analyzer', label: 'Serial', field: row => row.Analyzer },
          { name: 'actions', title: '', width: '96px', dataClass: 'text-right' }
        ]
      };
    },
    mounted: function() {
      this.Analyzers.RegisterListener();
    },
    unmounted: function() {
      this.Analyzers.UnregisterListener();
    },
    methods: {

      Select: function(analyzer, close) {
        this.$emit('update:analyzer', null);
        this.ShowSpinner = true;
        localStorage.setItem('Analyzer', analyzer);
        this.Post('getAnalyzer', { Analyzer: analyzer }).then(
          (result) => {
            if(this.Timer != null) clearTimeout(this.Timer);
            result.Analyzer = analyzer;
            this.$emit('update:analyzer', result);
            this.ShowSpinner = false;
            if(close == true) this.$emit('close');
          },
          () => {}
        );
      },
      CloseSelect: function(analyzer) {
        this.View = null;
        this.GetAnalyzers();
        if((analyzer != null) && (analyzer == this.analyzer.Analyzer)) this.Select(analyzer, false);
      },
      LoadView: function(view, analyzer) {
        this.ShowSpinner = true;
        this.Post('getAnalyzer', { Analyzer: analyzer }).then(
          (result) => {
            if(this.Timer != null) clearTimeout(this.Timer);
            result.Analyzer = analyzer;
            this.Analyzer = result;
            this.View = view;
            this.ShowSpinner = false;
          },
          () => {}
        );
      },
      Back: function() {
        if(this.Timer != null) clearTimeout(this.Timer);
        this.$emit('close');
      }
    },
    components: { AnalyzerRemote, AnalyzerConfigure, AnalyzerRates, AnalyzerMaintenance, AnalyzerAlarms, AnalyzerPermissions }
  }
</script>