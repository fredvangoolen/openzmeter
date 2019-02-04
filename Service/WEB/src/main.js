import Vue from 'vue'
import BootstrapVue from 'bootstrap-vue'
import Router from 'vue-router'
import VueTouch from 'vue-touch'
//import NavbarIndicators from './NavbarIndicators.vue'
import App from './App.vue'

import Dashboard from './Dashboard.vue'



Vue.use(Vue-Touch);
Vue.use(Router);
Vue.use(BootstrapVue);
//Vue.use(NavbarIndicators);

const router = new Router({
    routes:[
    {path:'/', name: 'Dashboard', component: Dashboard},
//    {path:'/post/:id',name:'post',component:Post,props:true}
//        
    ]})

new Vue({
  el: '#app',
  render: h => h(App),
        router
})
