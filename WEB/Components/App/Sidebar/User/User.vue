<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">User</h4></div>
          <div class="pull-right btn-group">
            <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Save changes" v-on:click="Save"><img :src="require('Imgs/diskette.png')"></button>
            <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Discard" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')"></button>
          </div>
        </div>
        <div class="modal-body">
          <spinner v-if="ShowSpinner"></spinner>
          <div class="row">
            <div class="col-xs-12 col-lg-4 col-sm-6">
              <div class="row">
                <div class="col-xs-12">
                  <div class="rounded-panel">
                    <div class="popover-title"><strong>Profile image</strong>
                      <div class="pull-right btn-group">
                        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Browse image" v-on:click="ShowImageDialog" v-if="Image == false"><img :src="require('Imgs/folder_explorer.png')"></button>
                        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Remove image" v-on:click="ClearImage" v-if="Image == true"><img :src="require('Imgs/cross.png')"></button>
                      </div>
                    </div>
                    <div class="popover-content" style="text-align: center">
                      <input type="file" ref="ImageChooser" accept="image/*" style="display:none" v-on:change="LoadImage">
                      <img id="dynamic" width="128" height="128" :src="ImageData">
                    </div>
                  </div>
                </div>
              </div>
            </div>
            <div class="col-xs-12 col-lg-8 col-sm-6">
              <div class="row">
                <div class="col-xs-12">
                  <label class="control-label">Username</label>
                  <input type="text" class="form-control" :value="UserName" disabled>
                </div>
                <div class="col-xs-12">
                  <label class="control-label">Password</label>
                  <input type="password" class="form-control" id="inputPassword">
                </div>
                <div class="col-xs-12">
                  <label class="control-label">Full name</label>
                  <input type="text" class="form-control" ref="Name" :value="Name">
                </div>
              </div>
            </div>
          </div>
          <div class="row">
            <div class="col-xs-12 col-sm-6">
              <label class="control-label">Telegram ID</label>
              <input type="text" class="form-control" ref="TelegramID" :value="TelegramID">
            </div>
            <div class="col-xs-12 col-sm-6">
              <label class="control-label">E-Mail</label>
              <input type="text" class="form-control" ref="EMail" :value="EMail">
            </div>
            <div class="col-xs-12 col-sm-6">
              <label class="control-label">Country</label>
              <input type="text" class="form-control" ref="Country" :value="Country">
            </div>
            <div class="col-xs-12 col-sm-6">
              <label class="control-label">Company</label>
              <input type="text" class="form-control" ref="Company" :value="Company">
            </div>
            <div class="col-xs-12">
              <label class="control-label">Address</label>
              <input type="text" class="form-control" ref="Address" :value="Address">
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>
  
<script>
  import Spinner from 'Components/Spinner.vue';
  export default {
    data: function() {
      return { ShowDialog: false, ShowSpinner: true, Image: false, ImageData: null, UserName: '', Name: '', Address: '', Company: '', EMail: '', TelegramID: '' };
    },
    created: function() {
      this.ClearImage();
    },
    methods: {
      show: function() {
        this.ShowDialog = true;
        $.ajax({ url: "getUser", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) {
            this.UserName = localStorage.getItem('UserName');
            this.Name = result.Name;
            this.Address = result.Address;
            this.Company = result.Company;
            if(result.Photo == null) {
              this.ClearImage();
            } else {
              this.ImageData = result.Photo;
            }
            this.EMail = result.EMail;
            this.TelegramID = result.TelegramID;
            this.Country = result.Country_Code;
            this.ShowSpinner = false;
          }.bind(this)
        });
        this.$nextTick(function() {
          $(this.$el).modal('show');
        });
      },
      Back: function() {
        this.ShowDialog = false;
      },
      Save: function() {
        
      },
      ShowImageDialog: function() {
        this.$refs.ImageChooser.click();
      },
      LoadImage: function(files) {
        files = files.target.files;    
        var reader = new FileReader();
        reader.readAsDataURL(files[0]);
        reader.onload = function(event) {
          const img = new Image();
          img.src = event.target.result;
          img.onload = function() {
            const elem = document.createElement('canvas');
            elem.width = 128;
            elem.height = 128;
            const ctx = elem.getContext('2d');
            ctx.beginPath();
            ctx.moveTo(0 + 16, 0);
            ctx.lineTo(128 - 16, 0);
            ctx.quadraticCurveTo(128, 0, 128, 16);
            ctx.lineTo(128, 0 + 128 - 16);
            ctx.quadraticCurveTo(0 + 128, 0 + 128, 0 + 128 - 16, 0 + 128);
            ctx.lineTo(0 + 16, 0 + 128);
            ctx.quadraticCurveTo(0, 0 + 128, 0, 0 + 128 - 16);
            ctx.lineTo(0, 0 + 16);
            ctx.quadraticCurveTo(0, 0, 0 + 16, 0);
            ctx.closePath();
            ctx.clip();
            ctx.drawImage(img, 0, 0, 128, 128);
            this.ImageData = ctx.canvas.toDataURL();
            this.Image = true;
          }.bind(this);
          reader.onerror = function(error) {
            console.log(error);
          };
        }.bind(this);
      },
      ClearImage: function() {
        const elem = document.createElement('canvas');
        elem.width = 128;
        elem.height = 128;
        const ctx = elem.getContext('2d');
        this.ImageData = ctx.canvas.toDataURL();
        this.Image = false;
      }
    },
    mounted: function() {
      this.$root.$el.append(this.$el);
    },
    destroyed: function() {
      this.$root.$el.removeChild(this.$el);
    },
    components: {Spinner}
  }
</script>

<style>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
  .popover-title {
    padding: 6px;
  }
</style>