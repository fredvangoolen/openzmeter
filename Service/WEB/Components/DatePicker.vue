<template>
  <div class='input-group' v-bind:class="{'input-group-xs': (big==false)}">
    <input type='text' class="form-control"/>
    <span class="input-group-addon"><span class="glyphicon-calendar"></span></span>
  </div>
</template>

<script>
  import moment from 'JS/moment.js';
  import datetimepicker from 'JS/bootstrap-datetimepicker.min.js';
  import 'CSS/bootstrap-datetimepicker.min.css'  
  export default {
    props: {options: { type: Object, default: {}}, big: {type: Boolean, default: true}, timezone: {type: String, default: 'UTC'}},
    data() {
      return { };
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
      $(this.$el).on('dp.change', function(e) {
        this.$emit('DateChanged', e.date);
      }.bind(this));
    }
  }
</script>

<style scoped>
  .input-group-addon {
    line-height: 0;
  }
</style>