<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="!ShowDialog">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">User</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Save changes" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Discard" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
        <div class="pull-right btn-group" v-if="false">
          <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Main' }" data-toggle="tooltip" title="Main configuration" v-on:click="View='Main'">Main</button>
          <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Preferences' }" data-toggle="tooltip" title="Preferences" v-on:click="View='Preferences'">Preferences</button>
        </div>
      </template>
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null">{{ Error }}</div>
      <div v-if="View=='Main'">
        <div class="row">
          <div class="col-xs-12 col-lg-4 col-sm-6">
            <div class="row">
              <div class="col-xs-12">
                <div class="rounded-panel">
                  <div class="popover-title"><strong>Profile image</strong>
                    <div class="pull-right btn-group">
                      <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Browse image" @click="LoadImage" v-if="User.Photo==''"><img :src="require('Imgs/folder_explorer.png')"></button>
                      <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Remove image" @click="ClearImage" v-if="User.Photo!=''"><img :src="require('Imgs/cross.png')"></button>
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
                    <button class="btn btn-default" data-toggle="tooltip" title="Change password" type="button" @click="ShowDialog=true"><img :src="require('Imgs/key.png')"></button>
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
            <input type="text" class="form-control" v-model="User.Telegram">
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
      </div>
      <div v-if="View=='Preferences'">
        <div class="row">
          <div class="col-xs-12">
            <label class="control-label" data-toggle="tooltip" title="Notify alarms with selected priority or higher">Alarms priority</label>
            <option-picker :options="['LOW', 'NORMAL', 'HIGH']" v-model="User.TimeZone" :big="true"></option-picker>
          </div>
        </div>
        Alarms notifications priority

        phase names<br>
        colors -> one color per param, decrement brightness for each phase
        colors -> one color per phase, thiker main variable and thin line for secondari


      </div>
    </dlg>
    <password :user.sync="User" v-if="ShowDialog" @close="ShowDialog=false"></password>
  </div>
</template>

<script>
  import Password from './Password.vue';
  import moment from 'moment';
  export default {
    props: { user: Object },
    data: function() {
      return { ShowDialog: false, View: 'Main', User: null, TimeZones: moment.tz.names().map( (name) => { return { name: name, value: name }; } ), ShowSpinner: false, Error: null };
    },
    created: function() {
      this.User = (this.user == null) ? { User: 'login_name', Name: 'Full name', Email: 'example@example.com', Telegram: '0', Address: 'user address', Photo: '', Company: 'Company name', TimeZone: 'UTC'} : this.user;
    },
    methods: {
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post((this.user == null) ? 'addUser' : 'setUser', this.User).then(
          (result) => {
            this.ShowSpinner = false;
            this.$emit('close');
          },
          (fail) => {
            this.ShowSpinner = false;
            this.Error = 'Failed to save user!';
            setTimeout(() => { this.Error = null; }, 1000);
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
    components: { Password }
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
  @media only screen and (max-width: 768px) {
  .modal-dialog {
    margin-top:120px !important;
  }
}
</style>