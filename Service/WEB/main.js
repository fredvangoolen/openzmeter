//Libs -----------------------------------------
import Vue from './JS/vue.js';
import Router from './JS/vue-router.js';
import jQuery from './JS/jquery.js';
window.$ = window.jQuery = jQuery;
import './JS/bootstrap.min.js';
import './CSS/bootstrap.min.css';
import './JS/dygraph.js';

Vue.use(Router);
import './CSS/openZmeter.css';

//Views ----------------------------------------
import App from 'Components/App/App.vue';
import Dashboard from 'Components/Views/Dashboard/View.vue';
import AnalysisVoltage from 'Components/Views/Analysis_Voltage/View.vue';
import AnalysisCurrent from 'Components/Views/Analysis_Current/View.vue';
import AnalysisPower from 'Components/Views/Analysis_Power/View.vue';
import AnalysisEnergy from 'Components/Views/Analysis_Energy/View.vue';
import AnalysisFrequency from 'Components/Views/Analysis_Frequency/View.vue';
import EventsITIC from 'Components/Views/Events_ITIC/View.vue';
import EventsDisturbances from 'Components/Views/Events_Disturbances/View.vue';
import EventsEN50160 from 'Components/Views/Events_EN50160/View.vue';
import StatsCosts from 'Components/Views/Stats_Costs/View.vue';
import StatsHeatmap from 'Components/Views/Stats_Heatmap/View.vue';
import StatsDistribution from 'Components/Views/Stats_Distribution/View.vue';
import Settings from './Components/Views/Settings/View.vue';

//import PageNotFound from './Components/Views/PageNotFound.vue';

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
//      { path: '*', component: PageNotFound }
    ]})
})
