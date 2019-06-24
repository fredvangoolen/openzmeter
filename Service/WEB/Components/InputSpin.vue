<template>
  <div class="input-group inputspin">
    <input type="text" class="form-control" :value="value" :min="min" :max="max">
    <div class="input-group-btn-vertical">
      <button class="btn btn-default" type="button">&#9650;</button>
      <button class="btn btn-default" type="button">&#9660;</button>
    </div>
  </div>
</template>

<script>
  export default {
    data: function() {
      return {};
    },
    methods: {
//        $('.spinner .btn:first-of-type').on('click', function() {
//      var btn = $(this);
//      var input = btn.closest('.spinner').find('input');
//      if (input.attr('max') == undefined || parseInt(input.val()) < parseInt(input.attr('max'))) {    
//        input.val(parseInt(input.val(), 10) + 1);
//      } else {
//        btn.next("disabled", true);
//      }
//    });
//    $('.spinner .btn:last-of-type').on('click', function() {
//      var btn = $(this);
//      var input = btn.closest('.spinner').find('input');
//      if (input.attr('min') == undefined || parseInt(input.val()) > parseInt(input.attr('min'))) {    
//        input.val(parseInt(input.val(), 10) - 1);
//      } else {
//        btn.prev("disabled", true);
//      }
//    });
    
    }
    
  }
</script>

<style scoped>
  .inputspin input {
    text-align: right;
  }
  .input-group-btn-vertical {
    position: relative;
    white-space: nowrap;
    width: 2%;
    vertical-align: middle;
    display: table-cell;
  }
  .input-group-btn-vertical > .btn {
    display: block;
    float: none;
    width: 100%;
    max-width: 100%;
    padding: 2px 4px;
    margin-left: -1px;
    position: relative;
    border-radius: 0;
    font-size: 10px;
    line-height: 12px;
  }
  .input-group-btn-vertical > .btn:first-child {
    border-top-right-radius: 4px;
  }
  .input-group-btn-vertical > .btn:last-child {
    margin-top: -2px;
    border-bottom-right-radius: 4px;
  }
  .input-group-btn-vertical i {
    position: absolute;
    top: 0;
    left: 4px;
  }
</style>