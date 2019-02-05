//Libs -----------------------------------------
import Vue from 'vue'
import BootstrapVue from 'bootstrap-vue'
import Router from 'vue-router'
import VueTouch from 'vue-touch'
//Views ----------------------------------------
import App from './Components/App.vue'
import Dashboard from './Components/Dashboard.vue'


import 'bootstrap/dist/css/bootstrap.css';
import 'bootstrap-vue/dist/bootstrap-vue.css';
import './CSS/openZmeter.css';



//Vue.use(Vue-Touch);
Vue.use(Router);
Vue.use(BootstrapVue);

const router = new Router({
  routes: [
    {path:'/', name: 'Dashboard', component: Dashboard},
//    {path:'/post/:id',name:'post',component:Post,props:true}
//        
    ]})

new Vue({
  el: '#app',
  render: h => h(App),
        router
})
