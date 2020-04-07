<template>
  <div class='input-group' v-bind:class="{'input-group-xs': (big==false)}">
    <input type='text' class="form-control"/>
    <span class="input-group-addon"><span class="glyphicon-calendar"></span></span>
  </div>
</template>

<script>
  import moment from 'moment';
  import datetimepicker from 'JS/bootstrap-datetimepicker.min.js';
  import 'CSS/bootstrap-datetimepicker.min.css'
  export default {
    props: { options: Object, big: Boolean, disabled: Boolean, value: Number },
    data() {
      return { };
    },
    watch: {
      disabled: function(val) {
        if(this.disabled === true)
          this.Disable();
        else
          this.Enable();
      },
      date: function(val) {
        this.SetDate(val);
      },
    },
    methods: {
      SetTimeZone: function(val) {
        $(this.$el).data("DateTimePicker").options({ timeZone: val });
      },
      SetDate(date) {
        $(this.$el).data("DateTimePicker").options({ date: moment(date) });
      },
      Disable() {
        $(this.$el).data("DateTimePicker").disable();
      },
      Enable() {
        $(this.$el).data("DateTimePicker").enable();
      }
    },
    mounted: function() {
      $(this.$el).datetimepicker(this.options);
      if(this.disabled == true) $(this.$el).data("DateTimePicker").disable();
      if(this.value != null) $(this.$el).data("DateTimePicker").options({ date: moment(this.value) });
      $(this.$el).on('dp.change', (e) => {
        if(this.Date != e.date.valueOf()) this.$emit('input', e.date.valueOf());
      });
    }
  }
</script>

<style scoped>
  .input-group-addon {
    line-height: 0;
  }
</style>