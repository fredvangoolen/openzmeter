import { createApp } from 'vue';
import Main from 'App/App.vue';
let App = createApp(Main);

//import VueAxios from 'vue-axios';

//App.use(VueAxios, Axios);

//window.EventBus = new Vue;
//import Router from 'vue-router';
//Vue.use(Router);
//import VueResource from 'vue-resource';
//App.use(VueResource);
//import PortalVue from 'portal-vue';
//Vue.use(PortalVue);
//import VueToast from 'vue-toast-notification';
//Vue.use(VueToast);
//Import Quasar and icons ----------------------------------------------------------------------------------------------
import { Quasar, Notify, Loading } from 'quasar';
let QuasarConf = {
  plugins: { Notify, Loading },
  animations: 'all',
  config: { }
};
App.use(Quasar, QuasarConf);
//Main libs and style --------------------------------------------------------------------------------------------------
//import 'jquery';
//import 'bootstrap';
//import 'bootstrap/dist/css/bootstrap.css';
import 'quasar/dist/quasar.css';
import 'material-icons/iconfont/material-icons.css';
//import 'quasar-extras/material-icons';
//import 'vue-toast-notification/dist/theme-sugar.css';
import 'CSS/openZmeter.css';
//OZM tools ------------------------------------------------------------------------------------------------------------
import openZmeter from 'JS/openZmeter.js';
App.use(openZmeter);
// Custom components ---------------------------------------------------------------------------------------------------
//import Vuetable from 'vuetable-2';
//Vue.component('Vuetable', Vuetable);
//import Avatar from 'Components/Avatar.vue';
//Vue.component('Avatar', Avatar);
//import ButtonExpand from 'Components/ButtonExpand.vue';
//Vue.component('ButtonExpand', ButtonExpand);
//import ButtonExport from 'Components/ButtonExport.vue';
//Vue.component('ButtonExport', ButtonExport);
//import ButtonPause from 'Components/ButtonPause.vue';
//Vue.component('ButtonPause', ButtonPause);
//import ColorBar from 'Components/ColorBar.vue';
//Vue.component('ColorBar', ColorBar);
//import ColorPicker from 'Components/ColorPicker.vue';
//Vue.component('ColorPicker', ColorPicker);
//import Cubism from 'Components/Cubism.vue';
//Vue.component('Cubism', Cubism);
//import Dlg from 'Components/Dialog.vue';
//Vue.component('Dlg', Dlg);
//import DatePicker from 'Components/DatePicker.vue';
//Vue.component('DatePicker', DatePicker);
//import DlgFs from 'Components/DialogFullscreen.vue';
//Vue.component('DlgFs', DlgFs);
//import Graph from 'Components/Graph.vue';
//Vue.component('Graph', Graph);
//import InputMath from 'Components/InputMath.vue';
//Vue.component('InputMath', InputMath);
//import InputText from 'Components/InputText.vue';
//Vue.component('InputText', InputText);
//import InputSpin from 'Components/InputSpin.vue';
//Vue.component('InputSpin', InputSpin);
//import OptionAdder from 'Components/OptionAdder.vue';
//Vue.component('OptionAdder', OptionAdder);
//import OptionPicker from 'Components/OptionPicker.vue';
//Vue.component('OptionPicker', OptionPicker);
//import Password from 'Components/Password.vue';
//Vue.component('Password', Password);
//import ProgressBar from 'Components/ProgressBar.vue';
//Vue.component('ProgressBar', ProgressBar);
//import Spinner from 'Components/Spinner.vue';
//Vue.component('Spinner', Spinner);
//import YearConfig from 'Components/YearConfig.vue';
//Vue.component('YearConfig', YearConfig);
//import DayConfig from 'Components/DayConfig.vue';
//Vue.component('DayConfig', DayConfig);
//import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
//Vue.component('ToolbarAggregation', ToolbarAggregation);
//import ToolbarYears from 'Components/ToolbarYears.vue';
//Vue.component('ToolbarYears', ToolbarYears);
//import ToolbarPager from 'Components/ToolbarPager.vue';
//Vue.component('ToolbarPager', ToolbarPager);
//import ToolbarPeriod from 'Components/ToolbarPeriod.vue';
//Vue.component('ToolbarPeriod', ToolbarPeriod);
//import ToolbarColorItems from 'Components/ToolbarColorItems.vue';
//Vue.component('ToolbarColorItems', ToolbarColorItems);
//import ToolbarPeriodB from 'Components/ToolbarPeriodB.vue';
//Vue.component('ToolbarPeriodB', ToolbarPeriodB);
//import ToolbarPhases from 'Components/ToolbarPhases.vue';
//Vue.component('ToolbarPhases', ToolbarPhases);
//import ToolbarOptions from 'Components/ToolbarOptions.vue';
//Vue.component('ToolbarOptions', ToolbarOptions);
//import ToolbarWeeks from 'Components/ToolbarWeeks.vue';
//Vue.component('ToolbarWeeks', ToolbarWeeks);
//import DetailTabs from 'Components/Tabs.vue';
//Vue.component('DetailTabs', DetailTabs);
//Views ----------------------------------------

import { createWebHistory, createRouter } from 'vue-router';
const ViewRouter = createRouter({
  history: createWebHistory(),
  routes: [
    { path: '/',                    redirect: '/Dashboard' },
    { path: '/Login',               component: () => import('Views/Login.vue') },
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
    { path: '/About',               component: () => import('Views/About/View.vue') },
    { path: '/:catchAll(.*)',       redirect: '/Dashboard' },
  ],
});
ViewRouter.beforeEach((to, from, next) => {
  Loading.show();
  App.config.globalProperties.Users.getCurrentUser().then(
    () => {
      if(to.path == '/Login') next('/Dashboard');
      else next();
    },
    () => {
      if(to.path != '/Login') next('/Login');
      else next();
    }
  );
});
ViewRouter.afterEach(() => {
  Loading.hide();
});
App.use(ViewRouter);
App.mount('#app');