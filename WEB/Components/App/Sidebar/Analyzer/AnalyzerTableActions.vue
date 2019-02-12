<template>
  <div class="btn-group">
    <button type="button" class="btn btn-success btn-xs" v-bind:class="{ disabled: active }" v-on:click="Select">Select</button>
    <button type="button" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown"><span class="caret"></span></button>
    <ul class="dropdown-menu pull-right">
      <li><a href="#" v-on:click="Configure" >Configure</a></li>
      <li><a href="#" v-on:click="ACL">ACL</a></li>
      <li><a href="#" v-on:click="Tariffs">Tariff</a></li>
      <li><a href="#" v-on:click="Alerts">Alerts</a></li>
      <li role="separator" class="divider"></li>
      <li><a href="#" v-on:click="Maintenance">Maintenance</a></li>
    </ul>
  </div>
</template>

<script>
  export default {
    props: ['row-data'],
    data() {
      return {};
    },
    methods: {
      Select() {
        localStorage.setItem("SelAnalyzer", this.rowData.ID);
        this.$root.SelectedAnalyzer = this.rowData;  
        this.$root.$emit('AnalyzerChanged');
      },
      Configure() {alert("Configure not yet implemented");},
      ACL() {alert("ACL not yet implemented");},
      Tariffs() { this.$parent.$parent.$parent.showTariffs(this.rowData.ID); },
      Alerts() {alert("Alerts not yet implemented");},
      Maintenance() {alert("Maintenance not yet implemented");}
    },
    computed: {
      active: function() { return (this.rowData.ID === localStorage.getItem("SelAnalyzer")); }
    }
  }  
</script>