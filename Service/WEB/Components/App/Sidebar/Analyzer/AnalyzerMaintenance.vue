<template>
  <div class="modal-content" v-bind:class="{ hide: !Show }">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Maintenance</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </div>
    <div class="modal-body">
      <input id="file-input" type="file" name="name" style="display: none;" />
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Only stored samples and events are affected by this next actions.<hr />Analyzer data take currently <strong>{{ DBSize }}</strong></div>
      <div class="row">
        <div class="col-xs-12 col-sm-6"><label class="control-label">From</label><date-picker ref="StartDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" v-on:DateChanged="SetStartDate"></date-picker></div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">To</label><date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" v-on:DateChanged="SetEndDate"></date-picker></div>
      </div>
      <hr style="margin-top: 10px; margin-bottom: 10px;"/>
      <div class="row" style="margin-top:5px; margin-bottom: 5px">
        <div class="col-xs-6"><button type="button" class="btn btn-default btn-block btn-xs" @click="BackupRange">Backup range</button></div>
        <div class="col-xs-6"><button type="button" class="btn btn-default btn-block btn-xs" @click="BackupAll">Backup all</button></div>
      </div>
      <hr style="margin-top: 10px; margin-bottom: 10px;"/>
      <div class="row" style="margin-top:5px; margin-bottom: 5px">
        <div class="col-xs-6"><button type="button" class="btn btn-default btn-block btn-danger btn-xs" @click="ClearRange">Clear range</button></div>
        <div class="col-xs-6"><button type="button" class="btn btn-default btn-block btn-danger btn-xs" @click="ClearAll">Clear all</button></div>
      </div>
      <hr style="margin-top: 10px; margin-bottom: 10px;"/>
      <div class="row" style="margin-top:5px; margin-bottom: 5px">
        <div class="col-xs-12"><button type="button" class="btn btn-default btn-block btn-xs" @click="Restore">Restore backup</button></div>
      </div>
    </div>
  </div>
</template>

<script>
  import bsSuggest from 'JS/bootstrap-suggest.js';
  import moment from 'JS/moment.js';
  import DatePicker from 'Components/DatePicker.vue';
  import Spinner from 'Components/Spinner.vue';
  import * as oZm from 'JS/openZmeter.js';
  import Oboe from 'JS/oboe.js';
  import Pako from 'JS/pako.js';
  export default {
    data() {
      return { Show: false, DBSize: '', StartDate: null, EndDate: null };
    },
    methods: {
      Load: function(analyzer, id) {
        this.Analyzer = analyzer;
        this.Show = true;
        this.DBSize = oZm.GetBestUnitsBin(analyzer.Size);
        this.StartDate = moment().startOf('year');
        this.EndDate = moment().endOf('year');
        this.$nextTick(function() {
          this.$refs.StartDate.SetDate(this.StartDate);
          this.$refs.EndDate.SetDate(this.EndDate);
        });
      },
      SetStartDate: function(date) {
        this.StartDate = date.startOf('day').valueOf();
      },
      SetEndDate: function(date) {
        this.EndDate = date.endOf('day').valueOf();
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Table.Load(this.Analyzer);
      },
      BackupRange: function() {
        var input = document.createElement('input');
        input.type = 'file';
        input.click();
        input.onchange = e => {
          var file = e.target.files[0];
        }

        
      },
      ClearRange: function() {},
      Restore: function() {},
      BackupAll: function() {},
      ClearAll: function() {}
    },
    created: function() {
      this.Analyzer = null;
      this.Tariffs = [];
      this.Initial = null;
    },
    beforeDestroy: function() {
      $(this.$refs.Base).off('onSetSelectValue', this.SelectBase.bind(this));
      $(this.$refs.Base).off('onUnsetSelectValue', this.DeselectBase.bind(this));
      $(this.$refs.Base).bsSuggest("destroy");
    },
    components: { DatePicker, Spinner }
  }
</script>

<style scoped>

</style>