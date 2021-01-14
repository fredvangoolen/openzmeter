__webpack_public_path__ = window.location.pathname;
//Main libs and style --------------------------------------------------------------------------------------------------
import jQuery from 'jquery';
import Bootstrap from 'bootstrap';
import Vue from 'vue';
import 'bootstrap/dist/css/bootstrap.css';
import 'CSS/openZmeter.css';
//VUE components -------------------------------------------------------------------------------------------------------
import VueI18n from 'vue-i18n';
Vue.use(VueI18n);
import Router from 'vue-router';
Vue.use(Router);
import VueResource from 'vue-resource';
Vue.use(VueResource);
import PortalVue from 'portal-vue';
Vue.use(PortalVue);
//Other tools ----------------------------------------------------------------------------------------------------------
import { blur } from 'auto-blur';
addEventListener('click', evt => blur(evt));
//OZM tools ------------------------------------------------------------------------------------------------------------
import openZmeter from 'JS/openZmeter.js';
Vue.use(openZmeter);
// Custom components ---------------------------------------------------------------------------------------------------
Vue.component('Avatar', (resolve) => { import('Components/Avatar.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ButtonExpand', (resolve) => { import('Components/ButtonExpand.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ButtonExport', (resolve) => { import('Components/ButtonExport.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ButtonPause', (resolve) => { import('Components/ButtonPause.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ColorBar', (resolve) => { import('Components/ColorBar.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ColorPicker', (resolve) => { import('Components/ColorPicker.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('Cubism', (resolve) => { import('Components/Cubism.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('Dlg', (resolve) => { import('Components/Dialog.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('DatePicker', (resolve) => { import('Components/DatePicker.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('DlgFs', (resolve) => { import('Components/DialogFullscreen.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('Graph', (resolve) => { import('Components/Graph.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('InputMath', (resolve) => { import('Components/InputMath.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('InputSpin', (resolve) => { import('Components/InputSpin.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('OptionAdder', (resolve) => { import('Components/OptionAdder.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('OptionPicker', (resolve) => { import('Components/OptionPicker.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('Password', (resolve) => { import('Components/Password.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ProgressBar', (resolve) => { import('Components/ProgressBar.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('Spinner', (resolve) => { import('Components/Spinner.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('YearConfig', (resolve) => { import('Components/YearConfig.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('DayConfig', (resolve) => { import('Components/DayConfig.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ToolbarAggregation', (resolve) => { import('Components/ToolbarAggregation.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ToolbarYears', (resolve) => { import('Components/ToolbarYears.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ToolbarPager', (resolve) => { import('Components/ToolbarPager.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ToolbarPeriod', (resolve) => { import('Components/ToolbarPeriod.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ToolbarColorItems', (resolve) => { import('Components/ToolbarColorItems.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ToolbarPeriodB', (resolve) => { import('Components/ToolbarPeriodB.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ToolbarPhases', (resolve) => { import('Components/ToolbarPhases.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ToolbarOptions', (resolve) => { import('Components/ToolbarOptions.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ToolbarWeeks', (resolve) => { import('Components/ToolbarWeeks.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('DetailTabs', (resolve) => { import('Components/Tabs.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('Toggle', (resolve) => { import('Components/Toggle.vue').then((Comp) => { resolve(Comp.default); }); } );
Vue.component('ViewSpinner', (resolve) => { import('Components/ViewSpinner.vue').then((Comp) => { resolve(Comp.default); }); } );
//Views ----------------------------------------
import App from 'App/App.vue';
var Routes = new Router({
  linkActiveClass: 'active',
  mode: 'history',
  routes: [
    { path: '/', redirect: '/Dashboard' },
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
    { path: '*',                    component: () => import('Views/PageNotFound.vue') }
  ]
});
var Main = new Vue({
  i18n: new VueI18n( { locale: 'es', fallbackLocale: 'en'} ),
  el: '#app',
  data: { Loading: false },
  render: h => h(App),
  router: Routes
});
Routes.beforeEach((to, from, next) => {
  Main.Loading = true;
  next();
});
Routes.afterEach(() => {
  Main.Loading = false;
});
