<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Configuration</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" :class="{ disabled: ShowSpinner }" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', null)"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null && !ShowSpinner">{{ Error }}</div>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable remote access</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Analyzer.RemoteEnabled"></toggle>
        <hr style="margin: 5px 0px 5px 0px"/>
      </div>
      <div class="col-xs-12">
        <label class="control-label">Remote URL</label>
        <input type="text" class="form-control" v-model="Analyzer.RemoteServer">
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">User name</label>
        <input type="text" class="form-control" v-model="Analyzer.RemoteUser">
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Password</label>
        <password v-model="Analyzer.RemotePass"></password>
      </div>
    </div>
  </dlg>
</template>

<script>
  export default {
    props: { analyzer: Object },
    data: function() {
      return { ShowSpinner: false, Error: null, Analyzer: null };
    },
    created: function() {
      this.Analyzer = Object.assign({}, this.analyzer);
    },
    methods: {
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post('setAnalyzer', this.Analyzer).then(
          (result) => { this.$emit('close', this.Analyzer.Analyzer); },
          (fail) => {
            this.ShowSpinner = false;
            this.Error = 'Failed to save analyzer!';
            setTimeout(() => { this.Error = null; }, 5000);
          }
        );
      }
    }
  }
</script>

