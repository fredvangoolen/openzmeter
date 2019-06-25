<template>
  <div>
    <div class="sidenav-version"><div>Build: {{ BuildDate }}</div>
      <!--button type="button" class="btn btn-default btn-xs pull-right" v-on:click="show"><img :src="require('Imgs/newspaper.png')"></button-->
    </div>
    <!--div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false" ref="dialog">
      <div class="modal-backdrop in"></div>
      <div class="modal-dialog">
        <div class="modal-content">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">{{ Title }}</h4></div>
            <div class="pull-right btn-group">
              <div class="pull-right btn-group">
                <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Save changes" v-on:click="Prev"><img :src="require('Imgs/resultset_previous.png')"></button>
                <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Save changes" v-on:click="Next"><img :src="require('Imgs/resultset_next.png')"></button>
                <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Discard" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')"></button>
              </div>
            </div>
          </div>
          <div class="modal-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div v-html="Contents"></div>
          </div>
        </div>
      </div>
    </div-->
  </div>
</template>

<script>
  import Spinner from 'Components/Spinner.vue';
  export default {
    data() {
      return { ShowSpinner: true, BuildDate: 'Wait...', ShowDialog: false, Title: '', Contents: '', News: []};
    },
    mounted: function() {
      $.ajax({ url: "getNews", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
        success: function(result) {
          this.News = result.News;
          this.BuildDate = result.Version;
        }.bind(this)
      });
    },
    destroyed: function() {
      this.$root.$el.removeChild(this.$refs.dialog);
    },
    methods: {
      show: function() {
        this.ShowDialog = true;
        $.ajax({ url: "getNews", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }),
          success: function(result) {
//            this.UserName = localStorage.getItem('UserName');
//            this.Name = result.Name;
//            this.Address = result.Address;
//            this.Company = result.Company;
//            if(result.Photo == null) {
//              this.ClearImage();
//            } else {
//              this.ImageData = result.Photo;
//            }
//            this.EMail = result.EMail;
//            this.TelegramID = result.TelegramID;
//            this.Country = result.Country_Code;
            this.ShowSpinner = false;
          }.bind(this)
        });
        this.$nextTick(function() {
          this.$root.$el.append(this.$refs.dialog);
          $(this.$refs.dialog).modal('show');
        });
      },
      Prev: function() {},
      Next: function() {},
      Back: function() {
        $(this.$refs.dialog).modal('hide');
        this.ShowDialog = false;
      },
      ReadNews() {
//        $('#btnNewsNext').click(LoadNews);
        $.ajax({ url: "getNews", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ LastLogin: parseInt(localStorage.getItem('LastLogin')) }), 
          success: function(result) {
            if(result === null) {
              $('#dlgNews').modal('hide');
              return;
            }
            $('#dlgNews .modal-header h4').html(result.Header);
            $('#dlgNews .modal-body').html(result.Contents);
            $('#dlgNews').modal('show');
            localStorage.setItem('LastLogin', result.Timestamp + 1);
          }
        }); 
      }
    },
    components: {Spinner}
  }
</script>

<style scoped>
  .modal-body {
    min-height: 200px;
  }
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
