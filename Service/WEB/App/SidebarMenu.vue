<template>
  <div class="text-subtitle1">
    <q-item clickable class="q-pa-xs" to="/Dashboard" active-class="text-white" v-if="Analyzer.Analyzer != null" style="width: 300px">
      <q-item-section side class="q-pr-xs"><img class="icon" :src="require('Imgs/dashboard.png')" /></q-item-section>
      <q-item-section>Dashboard</q-item-section>
    </q-item>
    <q-expansion-item header-class="q-pa-xs" group="menu" v-model="Expanded[0]" v-if="Analyzer.Analyzer != null" style="width: 300px">
      <template v-slot:header>
        <q-item-section side class="q-pr-xs"><img class="icon" :src="require('Imgs/draw_wave.png')" /></q-item-section>
        <q-item-section>Analysis</q-item-section>
      </template>
      <q-card class="bg-grey-8">
        <q-list>
          <q-item clickable class="q-pl-lg" to="/Analysis/Voltage"><q-item-section>Voltage</q-item-section></q-item>
          <q-item clickable class="q-pl-lg" to="/Analysis/Current"><q-item-section>Current</q-item-section></q-item>
          <q-item clickable class="q-pl-lg" to="/Analysis/Power"><q-item-section>Power</q-item-section></q-item>
          <q-item clickable class="q-pl-lg" to="/Analysis/Energy"><q-item-section>Energy</q-item-section></q-item>
          <q-item clickable class="q-pl-lg" to="/Analysis/Frequency" v-if="Analyzer.NominalFrequency != 0"><q-item-section>Frequency</q-item-section></q-item>
          <q-item clickable class="q-pl-lg" to="/Analysis/200MS" v-if="Analyzer.Save200MS == true"><q-item-section>200 MS</q-item-section></q-item>
        </q-list>
      </q-card>
    </q-expansion-item>
    <q-expansion-item header-class="q-pa-xs" group="menu" v-model="Expanded[1]" v-if="Analyzer.Analyzer != null" style="width: 300px">
      <template v-slot:header>
        <q-item-section side class="q-pr-xs"><img class="icon" :src="require('Imgs/warning.png')" /></q-item-section>
        <q-item-section>Events</q-item-section>
      </template>
      <q-card class="bg-grey-8">
        <q-list>
          <q-item clickable class="q-pl-lg" v-if="Analyzer.NominalFrequency != 0.0" to="/Events/ITIC"><q-item-section>ITIC</q-item-section></q-item>
          <!--q-item clickable class="q-pl-lg" v-if="Analyzers.Analyzer.NominalFrequency != 0.0" :to="{ path: '/Events/Disturbances', query: (this.UnreadEvents != 0) ? { From: this.status.Events.LastRead, To: this.status.Time } : null }"><q-item-section>Disturbances</q-item-section></q-item>
          <q-item clickable class="q-pl-lg" v-if="Analyzers.Analyzer.NominalFrequency != 0.0" to="/Events/EN50160"><q-item-section>EN50160</q-item-section></q-item>
          <q-item clickable class="q-pl-lg" :to="{ path: '/Events/Alarms', query: (this.UnreadAlarms != 0) ? { From: this.status.Alarms.LastRead, To: this.status.Time } : null }"><q-item-section>Alarms</q-item-section></q-item-->
        </q-list>
      </q-card>
    </q-expansion-item>

    <!--div class="item" @click="Submenu = (Submenu == 'Events') ? null : 'Events'" v-if="analyzer!=null">
      <img class="icon" :src="require('Imgs/warning.png')">
      <span class="badge badge-menu" v-if="(UnreadEvents+UnreadAlarms)>0">{{ UnreadEvents+UnreadAlarms }}</span>
      <div class="name">Events</div>
    </div>
    <div class="submenu" :class="{ show: (Submenu == 'Events') }" v-if="analyzer!=null">
      <router-link to="/Events/EN50160" v-if="analyzer.NominalFrequency!=0">EN50160</router-link>
      <router-link :to="{ path: '/Events/Alarms', query: (this.UnreadAlarms != 0) ? { From: this.status.Alarms.LastRead, To: this.status.Time } : null }">Alarms<span class="badge badge-submenu" v-if="UnreadAlarms>0">{{ UnreadAlarms }}</span></router-link>
    </div-->
    <q-expansion-item header-class="q-pa-xs" group="menu" v-model="Expanded[2]" v-if="Analyzer.Analyzer != null" style="width: 300px">
      <template v-slot:header>
        <q-item-section side class="q-pr-xs"><img class="icon" :src="require('Imgs/table_money.png')" /></q-item-section>
        <q-item-section>Costs</q-item-section>
      </template>
      <q-card class="bg-grey-8">
        <q-list>
          <q-item clickable class="q-pl-lg" to="/Stats/Costs"><q-item-section>Costs</q-item-section></q-item>
          <q-item clickable class="q-pl-lg" to="/Stats/Heatmap"><q-item-section>Heatmap</q-item-section></q-item>
          <q-item clickable class="q-pl-lg" to="/Stats/Distribution"><q-item-section>Distribution</q-item-section></q-item>
        </q-list>
      </q-card>
    </q-expansion-item>
    <q-item clickable class="q-pa-xs" to="/Settings" active-class="text-white" style="width: 300px">
      <q-item-section side class="q-pr-xs"><img class="icon" :src="require('Imgs/cog.png')"></q-item-section>
      <q-item-section>Settings</q-item-section>
    </q-item>
    <q-item clickable class="q-pa-xs" to="/About" active-class="text-white" style="width: 300px">
      <q-item-section side class="q-pr-xs"><img class="icon" :src="require('Imgs/help.png')"></q-item-section>
      <q-item-section>About</q-item-section>
    </q-item>
    <q-resize-observer @resize="onResize" />
  </div>
</template>

<script>
  export default {
    props: { status: Object },
    data() {
      return { Expanded: [ false ], TitleFrame: 0 }
    },
    created: function() {
//      setInterval(() => { this.SetTitle(); }, 2000)
    },
    computed: {
      UnreadEvents: function() {
//        if(this.status == null) return 0;
//        return this.status.Events.Count;
      },
      UnreadAlarms: function() {
//        if(this.status == null) return 0;
//        return this.status.Alarms.Count;
      }
    },
    methods: {
      onResize: function(val) {
        if(val.width < 100) this.Expanded.fill(false);
      },
      SetTitle: function() {
        if((this.TitleFrame == 0) || (this.analyzer == null)) {
          document.title = "openZmeter";
          this.TitleFrame = 1;
        } else if(this.TitleFrame == 1) {
          if((this.UnreadEvents != 0) && (this.UnreadAlarms != 0)) {
            document.title = '[' + this.UnreadEvents + ' new events and ' + this.UnreadAlarms + ' new alarms]';
          } else if(this.UnreadEvents != 0) {
            document.title = '[' + this.UnreadEvents + ' new events]';
          } else if(this.UnreadAlarms != 0) {
            document.title = '[' + this.UnreadAlarms + ' new alarms]';
          } else {
            document.title = "openZmeter";
          }
          this.TitleFrame = 0;
        }
      }
    }
  }
</script>

<style scoped>
  ::v-deep(.expandable) {
    justify-content: start !important;
    width: 300px;
  }
  .icon {
    height: 24px;
    width: 24px;
    margin: 0px 3px;
  }
</style>