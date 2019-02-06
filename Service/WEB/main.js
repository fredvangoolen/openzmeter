//Libs -----------------------------------------
import Vue from 'vue'
import BootstrapVue from 'bootstrap-vue'
import 'bootstrap/dist/css/bootstrap.css';
import 'bootstrap-vue/dist/bootstrap-vue.css';
Vue.use(BootstrapVue);
import Router from 'vue-router'
Vue.use(Router);
import VueTouch from 'vue-touch'
import jQuery from "jquery";
window.$ = window.jQuery = jQuery;

//import VueDygraphs from 'vue-dygraphs'


import * as oZm from './JS/openZmeter.js'
import './CSS/openZmeter.css';

//Views ----------------------------------------
import App from './Components/App.vue'
import Dashboard from './Components/Dashboard.vue'
import AnalysisVoltage from './Components/AnalysisVoltage.vue'
import AnalysisCurrent from './Components/AnalysisCurrent.vue'
import AnalysisPower from './Components/AnalysisPower.vue'
import AnalysisEnergy from './Components/AnalysisEnergy.vue'
import AnalysisFrequency from './Components/AnalysisFrequency.vue'

import Settings from './Components/Settings.vue'





//Vue.use(Vue-Touch);
//Vue.use(VueDygraphs);

const router = new Router({
  linkActiveClass: 'active',
  routes: [
    { path:'/Dashboard', component: Dashboard },
    { path:'/Analysis/Voltage', component: AnalysisVoltage },
    { path:'/Analysis/Current', component: AnalysisCurrent },
    { path:'/Analysis/Power', component: AnalysisPower },
    { path:'/Analysis/Energy', component: AnalysisEnergy },
    { path:'/Analysis/Frequency', component: AnalysisFrequency },
    { path:'/Settings', component: Settings },
    ]})

new Vue({
  el: '#app',
  render: h => h(App),
        router
})
