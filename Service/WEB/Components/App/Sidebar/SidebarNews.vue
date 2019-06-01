<template>
  <div class="sidenav-version"><span>{{ BuildDate }}</span>
    <button type="button" class="btn btn-default btn-xs pull-right" @click="ReadNews"><img :src="require('Imgs/newspaper.png')"></button>
  </div>
</template>

<script>
  export default {
    data() {
      return { BuildDate: 'TestVersion'};
    },
    methods: {
      ReadNews() {
        ///* Load News and show dialog if any */
//function LoadNews() {
//  $('#btnNewsNext').click(LoadNews);
//  $.ajax({ url: "getNews", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ LastLogin: parseInt(localStorage.getItem('LastLogin')) }), success: 
//    function(result) {
//      if(result === null) {
//        $('#dlgNews').modal('hide');
//        return;
//      }
//      $('#dlgNews .modal-header h4').html(result.Header);
//      $('#dlgNews .modal-body').html(result.Contents);
//      $('#dlgNews').modal('show');
//      localStorage.setItem('LastLogin', result.Timestamp + 1);
//    }
//  }); 
//}
      }
    }
  }
</script>

<style>
  .sidenav-version {
    color: white;
    width: 100%;
    padding: 0px 10px;
    font-size: 12px;
  }
  .sidenav-version > div {
    width: 210px;
    line-height: 26px;
    font-size: 14px;
    display: inline-block;
  }
  .sidenav-version button {
    padding: 4px;
    background-color: transparent;
    border: 1px solid #333;
  }
  .sidenav-version > span {
    vertical-align: middle;
  }
</style>
