<template>
  <q-dialog v-model="Visible" no-backdrop-dismiss @hide="$emit('close')" position="top">

    <analyzers-table @close="Visible = false" @goto="SetView" />
  </q-dialog>
</template>

<script>
  import AnalyzersTable from './Table.vue';

//  import AnalyzerInfo from './Analyzer/Info.vue';
//    import AnalyzerConfigure from './Configure.vue';
//  import AnalyzerPermissions from './Permissions.vue';
//  import AnalyzerRates from './Rates.vue';
//  import AnalyzerMaintenance from './Maintenance.vue';
//  import AnalyzerAlarms from './Alarms.vue';
//  import AnalyzerRemote from './Remote.vue';
  export default {
    data() {
      return { Visible: true };
    },
    methods: {
      SetView: function(view) {
        if(view == null) {
          this.Visible = false;
        }
      }
    },
    components: { AnalyzersTable }
  }
</script>
