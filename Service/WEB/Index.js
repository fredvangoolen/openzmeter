import { createApp } from 'vue';
import { Quasar, Notify, AddressbarColor } from 'quasar';
import { createWebHistory, createRouter } from 'vue-router';
import Main from 'App/App.vue';
import openZmeter from 'JS/openZmeter.js';
import IconSet from 'quasar/icon-set/svg-mdi-v6';
import PortalVuePlugin from 'portal-vue';
let App = createApp(Main);
let QuasarConf = {
  iconSet: IconSet,
  plugins: { Notify, AddressbarColor },
  animations: 'all',
  config: { }
};
App.use(Quasar, QuasarConf);
App.use(openZmeter);
App.use(PortalVuePlugin);
//Main libs and style --------------------------------------------------------------------------------------------------
import 'quasar/dist/quasar.css';
//import '@mdi/font/css/materialdesignicons.css';
import 'CSS/openZmeter.css';
//Custom components ----------------------------------------------------------------------------------------------------
import DialogFullscreen from 'Components/DialogFullscreen.vue';
App.component('DialogFullscreen', DialogFullscreen);
//import Graph from 'Components/Graph.vue';
//App.component('Graph', Graph);
import ToolbarAggreg from 'Components/ToolbarAggreg.vue';
App.component('ToolbarAggreg', ToolbarAggreg);
import ToolbarExpand from 'Components/ToolbarExpand.vue';
App.component('ToolbarExpand', ToolbarExpand);
import ToolbarExport from 'Components/ToolbarExport.vue';
App.component('ToolbarExport', ToolbarExport);
import ToolbarPause from 'Components/ToolbarPause.vue';
App.component('ToolbarPause', ToolbarPause);
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
import Analysis_200MS from 'Views/Analysis_200MS/View.vue';
import Events_ITIC from 'Views/Events_ITIC/View.vue';
import Events_Disturbances from 'Views/Events_Disturbances/View.vue';
import Events_EN50160 from 'Views/Events_EN50160/View.vue';
import Events_Alarms from 'Views/Events_Alarms/View.vue';
import Stats_Costs from 'Views/Stats_Costs/View.vue';
import Stats_Heatmap from'Views/Stats_Heatmap/View.vue';
import Stats_Distribution from 'Views/Stats_Distribution/View.vue';

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
    { path: '/Analysis/200MS',      component: Analysis_200MS },
    { path: '/Events/ITIC',         component: Events_ITIC },
    { path: '/Events/Disturbances', component: Events_Disturbances },
    { path: '/Events/EN50160',      component: Events_EN50160 },
    { path: '/Events/Alarms',       component: Events_Alarms },
    { path: '/Stats/Costs',         component: Stats_Costs },
    { path: '/Stats/Heatmap',       component: Stats_Heatmap },
    { path: '/Stats/Distribution',  component: Stats_Distribution },
    { path: '/Settings',            component: Settings },
    { path: '/About',               component: About },
    { path: '/:catchAll(.*)',       redirect: '/Dashboard' },
  ],
});
App.use(ViewRouter);
App.mount('#app');