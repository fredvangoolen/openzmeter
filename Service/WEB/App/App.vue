<template>
  <div id="app">
    <topbar :analyzer="Analyzer" :value="Values"></topbar>
    <sidebar :analyzer="Analyzer"></sidebar>
    <router-view :analyzer="Analyzer" id="contents"></router-view>
  </div>
</template>

<script>
  import Topbar from './Topbar.vue';
  import Sidebar from './Sidebar.vue';
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  import Graph from 'Components/Graph.vue';
  export default {
    data: function() {
      return { Analyzer: null, Values: {} };
    },
    components: { Topbar,  Sidebar }
  }
</script>

<style>
  #contents {
    position: fixed;
    z-index: 0;
    padding: 10px;
    overflow-x: hidden;
    overflow-y: auto;
    left: 0px;
    top: 116px;
    bottom: 0px;
    right: 0px;
  }
/*  .modal-dialog.full-screen-dialog{
    width: 100%; 
    height: 100%; 
    padding: 0px; 
    margin: 0px !important;
  }*/
  @media (min-width: 640px) {
    #contents {
      top: 56px;
    }
/*    .modal-dialog.full-screen-dialog{
      padding: 10px; 
    }*/
  }
  @media (min-width: 1280px) {
    #contents {
      left: 48px;
    }
  }
</style>
