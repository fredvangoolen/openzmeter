<template>
  <dlg>
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Holidays &raquo; Holiday</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')" alt="Go back"></button>
      </div>
    </template>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label">Name</label>
        <input type="text" class="form-control" v-model="tariff.Template.Holidays[index].Name">
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">From</label>
        <date-picker :big="true" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: tariff.TimeZone }" v-model="tariff.Template.Holidays[index].StartDate"></date-picker>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">To</label>
        <date-picker :big="true" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: tariff.TimeZone }" v-model="tariff.Template.Holidays[index].EndDate"></date-picker>
      </div>
    </div>
  </dlg>
</template>

<script>
  import moment from 'moment';
  import DatePicker from 'Components/DatePicker.vue';
  export default {
    props: { tariff: Object, index: Number },
    data() {
      return { };
    },
    methods: {
      Load: function(tariff, index) {
        this.Index = index;
        this.Tariff = tariff;
        this.Show = true;
        this.$nextTick(function() {
          $(this.$refs.Name).val(tariff.Template.Holidays[index].Name);
          this.$refs.StartDate.SetDate(tariff.Template.Holidays[index].StartDate);
          this.$refs.EndDate.SetDate(tariff.Template.Holidays[index].EndDate);
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