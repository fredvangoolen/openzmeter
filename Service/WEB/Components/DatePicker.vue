<template>
  <div class='input-group' v-bind:class="{'input-group-xs': (big==false)}">
    <input type='text' class="form-control"/>
    <span class="input-group-addon"><span class="glyphicon-calendar"></span></span>
  </div>
</template>

<script>
  import moment from 'moment';
  import datetimepicker from 'eonasdan-bootstrap-datetimepicker';
  import 'eonasdan-bootstrap-datetimepicker/build/css/bootstrap-datetimepicker.css'
  export default {
    props: { options: Object, big: Boolean, disabled: Boolean, value: Number },
    data() {
      return { Options: {} };
    },
    watch: {
      disabled: function(val) {
        if(this.disabled === true)
          $(this.$el).data("DateTimePicker").disable();
        else
          $(this.$el).data("DateTimePicker").enable();
      },
      value: function() {
        this.Options = Object.assign(this.Options, { date: moment(this.value) });
        $(this.$el).data("DateTimePicker").options(this.Options);
      },
      options: function() {
        this.Options = Object.assign(this.Options, this.options);
        $(this.$el).data("DateTimePicker").options(this.Options);
      }
    },
    beforeDestroy: function() {
      $(this.$el).data("DateTimePicker").destroy();
    },
    mounted: function() {
      this.Options = Object.assign({ date: moment(this.value) }, this.options);
      $(this.$el).datetimepicker({});
      $(this.$el).data("DateTimePicker").options(this.Options);
      $(this.$el).on('dp.change', (e) => {
        this.$emit('input', e.date.valueOf());
      });
      if(this.disabled == true) $(this.$el).data("DateTimePicker").disable();
    }
  }
</script>

<style scoped>
  .input-group-addon {
    line-height: 0;
  }
</style>