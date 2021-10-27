import { createApp } from 'vue';
import { Quasar, Notify } from 'quasar';
import { createWebHistory, createRouter } from 'vue-router';
import Main from 'App/App.vue';
import openZmeter from 'JS/openZmeter.js';
let App = createApp(Main);
let QuasarConf = {
  plugins: { Notify },
  animations: 'all',
  config: { }
};
App.use(Quasar, QuasarConf);
App.use(openZmeter);
//Main libs and style --------------------------------------------------------------------------------------------------
import 'quasar/dist/quasar.css';
import 'material-icons/iconfont/material-icons.css';
import 'CSS/openZmeter.css';
//Custom components ----------------------------------------------------------------------------------------------------
import ToolbarSelect from 'Components/ToolbarSelect.vue';
App.component('ToolbarSelect', ToolbarSelect);
//Define routes --------------------------------------------------------------------------------------------------------
import About from 'Views/About/View.vue';
const ViewRouter = createRouter({
  history: createWebHistory(),
  routes: [
    { path: '/',                    redirect: '/Dashboard' },
//    { path: '/Login',               component: require('Views/Login.vue') },
    { path: '/Config',              component: () => import('Views/Config.vue') },
    { path: '/Dashboard',           component: () => import('Views/Dashboard/View.vue') },
    { path: '/Analysis/Voltage',    component: () => import('Views/Analysis_Voltage/View.vue') },
    { path: '/Analysis/Current',    component: () => import('Views/Analysis_Current/View.vue') },
    { path: '/Analysis/Power',      component: () => import('Views/Analysis_Power/View.vue') },
    { path: '/Analysis/Energy',     component: () => import('Views/Analysis_Energy/View.vue') },
    { path: '/Analysis/Frequency',  component: () => import('Views/Analysis_Frequency/View.vue') },
    { path: '/Analysis/200MS',      component: () => import('Views/Analysis_200MS/View.vue') },
    { path: '/Events/ITIC',         component: () => import('Views/Events_ITIC/View.vue') },
    { path: '/Events/Disturbances', component: () => import('Views/Events_Disturbances/View.vue') },
    { path: '/Events/EN50160',      component: () => import('Views/Events_EN50160/View.vue') },
    { path: '/Events/Alarms',       component: () => import('Views/Events_Alarms/View.vue') },
    { path: '/Stats/Costs',         component: () => import('Views/Stats_Costs/View.vue') },
    { path: '/Stats/Heatmap',       component: () => import('Views/Stats_Heatmap/View.vue') },
    { path: '/Stats/Distribution',  component: () => import('Views/Stats_Distribution/View.vue') },
    { path: '/Settings',            component: () => import('Views/Settings/View.vue') },
    { path: '/About',               component: About },
    { path: '/:catchAll(.*)',       redirect: '/Dashboard' },
  ],
});
App.use(ViewRouter);
App.mount('#app');