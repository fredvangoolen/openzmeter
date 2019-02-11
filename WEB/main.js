//Libs -----------------------------------------
import Vue from 'vue';
import Router from 'vue-router'
//import jQuery from './JS/jquery.min.js';
//window.$ = window.jQuery = jQuery;
import 'bootstrap3';
import './CSS/bootstrap.min.css'


Vue.use(Router);


//import * as oZm from './JS/openZmeter.js';
import './CSS/openZmeter.css';

//Views ----------------------------------------
import App from 'Components/App/App.vue';
import Dashboard from 'Components/Views/Dashboard/View.vue';
import AnalysisVoltage from 'Components/Views/Analysis_Voltage/View.vue';
import AnalysisCurrent from 'Components/Views/Analysis_Current/View.vue';
import AnalysisPower from 'Components/Views/Analysis_Power/View.vue';
import AnalysisEnergy from 'Components/Views/Analysis_Energy/View.vue';
import AnalysisFrequency from 'Components/Views/Analysis_Frequency/View.vue';
import StatsHeatmap from 'Components/Views/Stats_Heatmap/View.vue';
import Settings from './Components/Views/Settings/View.vue';

new Vue({
  el: '#app',
  render: h => h(App),
  router: new Router({
    linkActiveClass: 'active',
    mode: 'history',
    routes: [
      { path:'/Dashboard', component: Dashboard },
      { path:'/Analysis/Voltage', component: AnalysisVoltage },
      { path:'/Analysis/Current', component: AnalysisCurrent },
      { path:'/Analysis/Power', component: AnalysisPower },
      { path:'/Analysis/Energy', component: AnalysisEnergy },
      { path:'/Analysis/Frequency', component: AnalysisFrequency },
      { path:'/Stats/Heatmap', component: StatsHeatmap },
      { path:'/Settings', component: Settings },
    ]})
})
