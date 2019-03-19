<template>
  <div class='input-group'>
    <input type='text' class="form-control"/>
    <span class="input-group-addon"><span class="glyphicon-calendar"></span></span>
  </div>
</template>

<script>
  import moment from 'JS/moment.js';
  import datetimepicker from 'JS/bootstrap-datetimepicker.min.js';
  import 'CSS/bootstrap-datetimepicker.min.css'  
  export default {
    props: ['options'],
    data() {
      return { };
    },
    methods: {
      SetDate(date) {
        $(this.$el).data("DateTimePicker").options({ defaultDate: date });
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
  
</style>