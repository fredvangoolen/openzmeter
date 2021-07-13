<template>
  <div>
    <input ref="check" class="checkbox" type="checkbox" data-width="52">
  </div>
</template>

<script>
  import Toggle from 'JS/bootstrap-toggle.js';
  export default {
    props: {onText: { type: String, default: 'On'}, offText: {type: String, default: 'Off'}, onStyle: {type: String, default: 'success'}, offStyle: {type: String, default: 'danger'}, size: { type: String, default: 'mini'}, value: { type: Boolean, default: false}},
    data: function() {
      return {};
    },
    mounted: function() {
      $(this.$refs.check).bootstrapToggle({on: this.onText, off: this.offText, onstyle: this.onStyle, offstyle: this.offStyle, size: this.size});
      this.SetState(this.value);
    },
    watch: {
      value: function(val) {
        this.SetState(val);
      }
    },
    methods: {
      SetState: function(checked) {
        $(this.$refs.check).off('change');
        $(this.$refs.check).bootstrapToggle((checked) ? 'on' : 'off');
        $(this.$refs.check).on('change', function(e) {
          this.$emit('change', $(this.$refs.check).prop('checked'));
          this.$emit('input', $(this.$refs.check).prop('checked'));
        }.bind(this));
      }
    },
    components: { Toggle }
  }
</script>

<style>
  .checkbox label .toggle,.checkbox-inline .toggle{margin-left:-20px;margin-right:5px}
  .toggle{position:relative;overflow:hidden}
  .toggle input[type=checkbox]{display:none}
  .toggle-group{position:absolute;width:200%;top:0;bottom:0;left:0;transition:left .35s;-webkit-transition:left .35s;-moz-user-select:none;-webkit-user-select:none}
  .toggle.off .toggle-group{left:-100%}
  .toggle-on{position:absolute;top:0;bottom:0;left:0;right:50%;margin:0;border:0;border-radius:0}
  .toggle-off{position:absolute;top:0;bottom:0;left:50%;right:0;margin:0;border:0;border-radius:0}
  .toggle-handle{position:relative;margin:0 auto;padding-top:0;padding-bottom:0;height:100%;width:0;border-width:0 1px}
  .toggle.btn{min-width:59px;min-height:34px}
  .toggle-on.btn{padding-right:24px}
  .toggle-off.btn{padding-left:24px}
  .toggle.btn-lg{min-width:79px;min-height:45px}
  .toggle-on.btn-lg{padding-right:31px}
  .toggle-off.btn-lg{padding-left:31px}
  .toggle-handle.btn-lg{width:40px}
  .toggle.btn-sm{min-width:50px;min-height:30px}
  .toggle-on.btn-sm{padding-right:20px}
  .toggle-off.btn-sm{padding-left:20px}
  .toggle.btn-xs{min-width:35px;min-height:22px}
  .toggle-on.btn-xs{padding-right:12px}
  .toggle-off.btn-xs{padding-left:12px}
  checkbox .active{ background-color: red};
</style>