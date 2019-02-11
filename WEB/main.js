//Libs -----------------------------------------
import Vue from 'vue';
//import BlockUI from 'vue-blockui';
import Router from 'vue-router'
//import VueTransferDom from 'vue-transfer-dom';
import jQuery from './JS/jquery.min.js';
window.$ = window.jQuery = jQuery;
import 'bootstrap3';
import './CSS/bootstrap.min.css'


//Vue.use(BlockUI);
Vue.use(Router);
//Vue.use(VueTransferDom);
//import VueTouch from 'vue-touch'

//import VueDygraphs from 'vue-dygraphs'
//import 'bootstrap3/dist/fonts/glyphicons-halflings-regular.ttf'
//import 'bootstrap3/dist/css/bootstrap.css'

import * as oZm from './JS/openZmeter.js';
import './CSS/openZmeter.css';

//Views ----------------------------------------
import App from 'Components/App/View.vue';
import Dashboard from 'Components/Views/Dashboard/View.vue';
import AnalysisVoltage from 'Components/Views/Analysis_Voltage/View.vue';
import AnalysisCurrent from 'Components/AnalysisCurrent.vue';
import AnalysisPower from 'Components/AnalysisPower.vue';
import AnalysisEnergy from 'Components/AnalysisEnergy.vue';
import AnalysisFrequency from 'Components/AnalysisFrequency.vue';

import StatsHeatmap from 'Components/Views/Stats_Heatmap/View.vue';
import Settings from './Components/Settings.vue';





//Vue.use(Vue-Touch);
//Vue.use(VueDygraphs);

const router = new Router({
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

new Vue({
  el: '#app',
  render: h => h(App),
        router
})
