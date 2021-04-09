<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="!ShowDialog">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">User</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" title="Save changes" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
          <button type="button" class="btn btn-default btn-xs" title="Discard" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
      </template>
      <div class="row">
        <div class="col-xs-12 col-lg-4 col-sm-6">
          <div class="row">
            <div class="col-xs-12">
              <div class="rounded-panel">
                <div class="popover-title"><strong>Profile image</strong>
                  <div class="pull-right btn-group">
                    <button type="button" class="btn btn-default btn-xs" title="Browse image" @click="LoadImage" v-if="User.Photo==''"><img :src="require('Imgs/folder_explorer.png')"></button>
                    <button type="button" class="btn btn-default btn-xs" title="Remove image" @click="ClearImage" v-if="User.Photo!=''"><img :src="require('Imgs/cross.png')"></button>
                  </div>
                </div>
                <div class="popover-content" style="text-align: center">
                  <avatar ref="avatar" v-model="User.Photo"></avatar>
                </div>
              </div>
            </div>
          </div>
        </div>
        <div class="col-xs-12 col-lg-8 col-sm-6">
          <div class="row">
            <div class="col-xs-12" v-if="user!=null">
              <label class="control-label">Username</label>
              <div class="input-group">
                <input type="text" class="form-control" v-model="User.User" :disabled="user!=null">
                <span class="input-group-btn">
                  <button class="btn btn-default" title="Change password" type="button" @click="ShowDialog=true"><img :src="require('Imgs/key.png')"></button>
                </span>
              </div>
            </div>
            <div class="col-xs-12 col-sm-6" v-if="user==null">
              <label class="control-label">Full name</label>
              <input type="text" class="form-control" v-model="User.User">
            </div>
            <div class="col-xs-12 col-sm-6" v-if="user==null">
              <label class="control-label">Password</label>
              <password v-model="User.Password"></password>
            </div>
            <div class="col-xs-12">
              <label class="control-label">Full name</label>
              <input type="text" class="form-control" v-model="User.Name">
            </div>
            <div class="col-xs-12">
              <label class="control-label">Address</label>
              <input type="text" class="form-control" v-model="User.Address">
            </div>
          </div>
        </div>
      </div>
      <div class="row">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Telegram ID</label>
          <input-spin :big="true" :decimals="0" :min="-100000000000000" :max="100000000000000" v-model="User.Telegram"></input-spin>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">E-Mail</label>
          <input type="text" class="form-control" v-model="User.Email">
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Time zone</label>
          <option-picker :options="TimeZones" v-model="User.TimeZone" :big="true"></option-picker>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Company</label>
          <input type="text" class="form-control" v-model="User.Company">
        </div>
      </div>
    </dlg>
    <pass-dlg :user.sync="User" v-if="ShowDialog" @close="ShowDialog=false"></pass-dlg>
  </div>
</template>

<script>
  import moment from 'moment';
  import PassDlg from './Password.vue';
  export default {
    props: { user: Object },
    data: function() {
      return { ShowDialog: false, User: null, TimeZones: moment.tz.names().map( (name) => { return { name: name, value: name }; } ), ShowSpinner: false };
    },
    created: function() {
      this.User = (this.user == null) ? { User: 'login_name', Name: 'Full name', Email: 'example@example.com', Telegram: 0, Address: 'user address', Photo: '', Company: 'Company name', TimeZone: 'UTC', Password: ''} : this.user;
    },
    methods: {
      Save: function() {
        this.ShowSpinner = true;
        this.Post((this.user == null) ? 'addUser' : 'setUser', this.User).then(
          (result) => {
            this.ShowSpinner = false;
            this.$emit('close');
            this.$toast.open({type:'success', message: "User '" + this.User.User + "' saved", duration: 3000});
          },
          (fail) => {
            this.ShowSpinner = false;
            if(fail != null) this.$toast.open({type:'error', message: "Failed to save user<br>" + fail.message, duration: 3000});
          }
        );
      },
      LoadImage: function() {
        this.$refs.avatar.LoadImage();
      },
      ClearImage: function() {
        this.$refs.avatar.ClearImage();
      }
    },
    components: { PassDlg }
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