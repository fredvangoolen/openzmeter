<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Permission &raquo; Edit</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Error != '') }" v-on:click="Save"><img :src="require('Imgs/diskette.png')" alt="Save permission"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', null)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != ''">{{ Error }}</div>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label">Analyzer serial number</label>
        <input type="text" class="form-control" disabled="" :value="analyzer.Analyzer">
      </div>
      <div class="col-xs-12">
        <label class="control-label">User name</label>
        <input type="text" class="form-control" v-model="Permission.User">
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Allow read</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Permission.Read"></toggle>
        <div class="help">View and export all analysis data. All configuration is also visible, excepts permissions.</div>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Allow write</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Permission.Write"></toggle>
        <div class="help">Allow to alter analysis data. (Currently only delete event and edit its descriptions is allowed)</div>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Allow configure</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Permission.Configure"></toggle>
        <div class="help">Allow to change all analyzer settings, modify rates, alarms, and restore database backups</div>
      </div>

    </div>
  </dlg>
</template>

<script>
  export default {
    props: { analyzer: Object, permission: Object },
    data: function() {
      return { ShowSpinner: false, Error: '', Permission: null };
    },
    created: function() {
      if(this.permission == null) {
        this.Permission = { Analyzer: this.analyzer.Analyzer, User: '', Read: false, Write: false, Configure: false };
      } else {
        this.Permission = Object.assign({ Analyzer: this.analyzer.Analyzer }, this.permission);
      }
    },
    methods: {
      Save: function() {
        if(this.permission == null) {
          this.$http.post('addPermission', this.Permission).then(
            (result) => {
              $emit('close');
            },
            (fail) => {
              this.Error = 'Can not add permission, maybe user does not exist or is already defined';
              setTimeout(() => { this.Error = ''; }, 5000);
            }
          );
        }
      }
    }
  }
</script>

<style scoped>
  .help {
    font-size: 80%;
  }
</style>