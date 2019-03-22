<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Holidays &raquo; Holiday</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Go back"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label">Name</label><input type="text" class="form-control" ref="Name" @change="SetName">
        </div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">From</label><date-picker ref="StartDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" v-on:DateChanged="SetStartDate"></date-picker></div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">To</label><date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" v-on:DateChanged="SetEndDate"></date-picker></div>
      </div>
    </div>
  </div> 
</template>

<script>
  import moment from 'JS/moment.js';
  import DatePicker from 'Components/DatePicker.vue';
  export default {
    data() {
      return { Show: false, Tariff: {}, Index: 0 };
    },
    methods: {
      Load: function(tariff, index) {
        this.Index = index;
        this.Tariff = tariff;
        this.Show = true;
        this.$nextTick(function() {
          $(this.$refs.Name).val(tariff.Template.Holidays[index].Name);
          this.$refs.StartDate.SetDate(moment(tariff.Template.Holidays[index].StartDate));
          this.$refs.EndDate.SetDate(moment(tariff.Template.Holidays[index].EndDate));
        });
      },
      SetName: function(evt) {
        this.Tariff.Template.Holidays[this.Index].Name = evt.target.value;
      },
      SetStartDate: function(date) {
        this.Tariff.Template.Holidays[this.Index].StartDate = date.startOf('day').valueOf();
      },
      SetEndDate: function(date) {
        this.Tariff.Template.Holidays[this.Index].EndDate = date.endOf('day').valueOf();
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Holidays.Load(this.Tariff);
      }
    },
    components: { DatePicker }
  }
</script>