//Libs -----------------------------------------
import Vue from './JS/vue.js';
import Router from './JS/vue-router.js';
//import BootstrapVue from 'bootstrap-vue'

//Vue.use(BootstrapVue)



import jQuery from './JS/jquery.js';
window.$ = window.jQuery = jQuery;
//import 'bootstrap/dist/css/bootstrap.css'
//import 'bootstrap-vue/dist/bootstrap-vue.css'

import './JS/bootstrap.min.js';
import './CSS/bootstrap.min.css';
//import './JS/dygraph.js';

Vue.use(Router);
import './CSS/openZmeter.css';

//Views ----------------------------------------
import App from 'App/App.vue';
import Dashboard from 'Views/Dashboard/View.vue';
import AnalysisVoltage from 'Views/Analysis_Voltage/View.vue';
import AnalysisCurrent from 'Views/Analysis_Current/View.vue';
import AnalysisPower from 'Views/Analysis_Power/View.vue';
import AnalysisEnergy from 'Views/Analysis_Energy/View.vue';
import AnalysisFrequency from 'Views/Analysis_Frequency/View.vue';
import EventsITIC from 'Views/Events_ITIC/View.vue';
import EventsDisturbances from 'Views/Events_Disturbances/View.vue';
import EventsEN50160 from 'Views/Events_EN50160/View.vue';
import StatsCosts from 'Views/Stats_Costs/View.vue';
import StatsHeatmap from 'Views/Stats_Heatmap/View.vue';
import StatsDistribution from 'Views/Stats_Distribution/View.vue';
import Settings from 'Views/Settings/View.vue';
import Help from 'Views/Help/Help.vue';
import PageNotFound from 'Views/PageNotFound.vue';

new Vue({
  el: '#app',
  render: h => h(App),
  router: new Router({
    linkActiveClass: 'active',
    mode: 'history',
    routes: [
      { path: '/', redirect: '/Dashboard' },
      { path: '/Dashboard', component: Dashboard },
      { path: '/Analysis/Voltage', component: AnalysisVoltage },
      { path: '/Analysis/Current', component: AnalysisCurrent },
      { path: '/Analysis/Power', component: AnalysisPower },
      { path: '/Analysis/Energy', component: AnalysisEnergy },
      { path: '/Analysis/Frequency', component: AnalysisFrequency },
      { path: '/Events/ITIC', component: EventsITIC },
      { path: '/Events/Disturbances', component: EventsDisturbances },
      { path: '/Events/EN50160', component: EventsEN50160 },
      { path: '/Stats/Costs', component: StatsCosts },
      { path:'/Stats/Heatmap', component: StatsHeatmap },
      { path:'/Stats/Distribution', component: StatsDistribution },
      { path:'/Settings', component: Settings },
      { path:'/Help', component: Help },
      { path: '*', component: PageNotFound }
    ]})
})
