import { createApp } from 'vue';
import { Quasar, Notify } from 'quasar';
import { createWebHistory, createRouter } from 'vue-router';
import Main from 'App/App.vue';
import openZmeter from 'JS/openZmeter.js';
import IconSet from 'quasar/icon-set/svg-mdi-v6';
let App = createApp(Main);
let QuasarConf = {
  iconSet: IconSet,
  plugins: { Notify },
  animations: 'all',
  config: { }
};
App.use(Quasar, QuasarConf);
App.use(openZmeter);
//Main libs and style --------------------------------------------------------------------------------------------------
import 'quasar/dist/quasar.css';
//import '@mdi/font/css/materialdesignicons.css';
import 'CSS/openZmeter.css';
//Custom components ----------------------------------------------------------------------------------------------------
import ButtonExpand from 'Components/ButtonExpand.vue';
App.component('ButtonExpand', ButtonExpand);
import ToolbarAggreg from 'Components/ToolbarAggreg.vue';
App.component('ToolbarAggreg', ToolbarAggreg);
import ToolbarExport from 'Components/ToolbarExport';
App.component('ToolbarExport', ToolbarExport);
import ToolbarPhasesMulti from 'Components/ToolbarPhasesMulti.vue';
App.component('ToolbarPhasesMulti', ToolbarPhasesMulti);
import ToolbarPhasesSingle from 'Components/ToolbarPhasesSingle.vue';
App.component('ToolbarPhasesSingle', ToolbarPhasesSingle);
//import ToolbarSelect from 'Components/ToolbarSelect.vue';
//App.component('ToolbarSelect', ToolbarSelect);
import ToolbarYear from 'Components/ToolbarYear.vue';
App.component('ToolbarYear', ToolbarYear);
import ColorBar from 'Components/ColorBar.vue';
App.component('ColorBar', ColorBar);
//Define routes --------------------------------------------------------------------------------------------------------
import Config from 'Views/Config.vue';
import Dashboard from 'Views/Dashboard/View.vue';
import Analysis_Voltage from 'Views/Analysis_Voltage/View.vue';
import Analysis_Current from 'Views/Analysis_Current/View.vue';
import Analysis_Power from 'Views/Analysis_Power/View.vue';
import Analysis_Energy from 'Views/Analysis_Energy/View.vue';
import Analysis_Frequency from 'Views/Analysis_Frequency/View.vue';
import Settings from 'Views/Settings/View.vue';
import About from 'Views/About/View.vue';
const ViewRouter = createRouter({
  history: createWebHistory(),
  routes: [
    { path: '/',                    redirect: '/Dashboard' },
    { path: '/Config',              component: Config },
    { path: '/Dashboard',           component: Dashboard },
    { path: '/Analysis/Voltage',    component: Analysis_Voltage },
    { path: '/Analysis/Current',    component: Analysis_Current },
    { path: '/Analysis/Power',      component: Analysis_Power },
    { path: '/Analysis/Energy',     component: Analysis_Energy },
    { path: '/Analysis/Frequency',  component: Analysis_Frequency },
    { path: '/Analysis/200MS',      component: () => import('Views/Analysis_200MS/View.vue') },
    { path: '/Events/ITIC',         component: () => import('Views/Events_ITIC/View.vue') },
    { path: '/Events/Disturbances', component: () => import('Views/Events_Disturbances/View.vue') },
    { path: '/Events/EN50160',      component: () => import('Views/Events_EN50160/View.vue') },
    { path: '/Events/Alarms',       component: () => import('Views/Events_Alarms/View.vue') },
    { path: '/Stats/Costs',         component: () => import('Views/Stats_Costs/View.vue') },
    { path: '/Stats/Heatmap',       component: () => import('Views/Stats_Heatmap/View.vue') },
    { path: '/Stats/Distribution',  component: () => import('Views/Stats_Distribution/View.vue') },
    { path: '/Settings',            component: Settings },
    { path: '/About',               component: About },
    { path: '/:catchAll(.*)',       redirect: '/Dashboard' },
  ],
});
App.use(ViewRouter);
App.mount('#app');