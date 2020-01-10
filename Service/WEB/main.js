//Main libs and style --------------------------------------------------------------------------------------------------
import jQuery from 'jquery';
import Bootstrap from 'bootstrap';
import 'bootstrap/dist/css/bootstrap.css';
import 'CSS/openZmeter.css';
//VUE components -------------------------------------------------------------------------------------------------------
import Vue from 'vue';
import Router from 'vue-router';
Vue.use(Router);
import VueResource from 'vue-resource';
Vue.use(VueResource);
import PortalVue from 'portal-vue';
Vue.use(PortalVue);
//OZM tools ------------------------------------------------------------------------------------------------------------
import openZmeter from 'JS/openZmeter.js';
Vue.use(openZmeter);
// Custom components ---------------------------------------------------------------------------------------------------
import Avatar from 'Components/Avatar.vue';
Vue.component('Avatar', Avatar);
import ButtonExpand from 'Components/ButtonExpand.vue';
Vue.component('ButtonExpand', ButtonExpand);
import ButtonExport from 'Components/ButtonExport.vue';
Vue.component('ButtonExport', ButtonExport);
import ButtonPause from 'Components/ButtonPause.vue';
Vue.component('ButtonPause', ButtonPause);
import ColorBar from 'Components/ColorBar.vue';
Vue.component('ColorBar', ColorBar);
import Spinner from 'Components/Spinner.vue';

import Cubism from 'Components/Cubism.vue';
Vue.component('Cubism', Cubism);

Vue.component('Spinner', Spinner);
import Dlg from 'Components/Dialog.vue';
Vue.component('Dlg', Dlg);
import Graph from 'Components/Graph.vue';
Vue.component('Graph', Graph);
import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
Vue.component('ToolbarAggregation', ToolbarAggregation);
//import ToolbarTools from 'Components/ToolbarTools.vue';
//Vue.component('ToolbarTools', ToolbarTools);
import ToolbarYears from 'Components/ToolbarYears.vue';
Vue.component('ToolbarYears', ToolbarYears);
import ToolbarPeriod from 'Components/ToolbarPeriod.vue';
Vue.component('ToolbarPeriod', ToolbarPeriod);
import ToolbarPhases from 'Components/ToolbarPhases.vue';
Vue.component('ToolbarPhases', ToolbarPhases);
import ToolbarOptions from 'Components/ToolbarOptions.vue';
Vue.component('ToolbarOptions', ToolbarOptions);
import DetailTabs from 'Components/Tabs.vue';
Vue.component('DetailTabs', DetailTabs);

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
import EventsAlarms from 'Views/Events_Alarms/View.vue';
import StatsCosts from 'Views/Stats_Costs/View.vue';
import StatsHeatmap from 'Views/Stats_Heatmap/View.vue';
import StatsDistribution from 'Views/Stats_Distribution/View.vue';
import Settings from 'Views/Settings/View.vue';
import Help from 'Views/Help/Help.vue';
import PageNotFound from 'Views/PageNotFound.vue';

const AppRouter = new Router({
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
    { path: '/Events/Alarms', component: EventsAlarms },
    { path: '/Stats/Costs', component: StatsCosts },
    { path: '/Stats/Heatmap', component: StatsHeatmap },
    { path: '/Stats/Distribution', component: StatsDistribution },
    { path: '/Settings', component: Settings },
    { path: '/Help', component: Help },
    { path: '*', component: PageNotFound }
  ]
});

new Vue( { el: '#app', render: h => h(App), router: AppRouter } );