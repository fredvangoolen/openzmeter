<template>
  <dlg ref="Dialog">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">User</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Save changes" v-on:click="Save"><img :src="require('Imgs/diskette.png')"></button>
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Discard" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: View=='Main' }" data-toggle="tooltip" title="Main configuration" v-on:click="View='Main'">Main</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: View=='Preferences' }" data-toggle="tooltip" title="Preferences" v-on:click="View='Preferences'">Preferences</button>
      </div>
    </template>
    <div v-bind:style="{ display: ((View=='Main') ? 'inherit' : 'none') }">
      <div class="row">
        <div class="col-xs-12 col-lg-4 col-sm-6">
          <div class="row">
            <div class="col-xs-12">
              <div class="rounded-panel">
                <div class="popover-title"><strong>Profile image</strong>
                  <div class="pull-right btn-group">
                    <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Browse image" v-on:click="LoadImage" v-if="User.Photo==''"><img :src="require('Imgs/folder_explorer.png')"></button>
                    <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Remove image" v-on:click="ClearImage" v-if="User.Photo!=''"><img :src="require('Imgs/cross.png')"></button>
                  </div>
                </div>
                <div class="popover-content" style="text-align: center">
                  <avatar ref="avatar" :value="User.Photo" v-on:change="User.Photo=$event"></avatar>
                </div>
              </div>
            </div>
          </div>
        </div>
        <div class="col-xs-12 col-lg-8 col-sm-6">
          <div class="row">
            <div class="col-xs-12">
              <label class="control-label">Username</label>
              <div class="input-group">
                <input type="text" class="form-control" :value="User.UserName" disabled>
                <span class="input-group-btn">
                  <button class="btn btn-default" data-toggle="tooltip" title="Change password" type="button"><img :src="require('Imgs/key.png')"></button>
                </span>
              </div>
            </div>
            <div class="col-xs-12">
              <label class="control-label">Full name</label>
              <input type="text" class="form-control" :value="User.Name">
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
          <input type="text" class="form-control" v-model="User.TelegramID">
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">E-Mail</label>
          <input type="text" class="form-control" v-model="User.EMail">
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Time zone</label>
          <option-picker :options="TimeZones" :selected="User.TimeZone" v-on:change="User.TimeZone=$event" :big="true"></option-picker>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Company</label>
          <input type="text" class="form-control" v-model="User.Company">
        </div>
      </div>
    </div>
    <div v-bind:style="{ display: ((View=='Preferences') ? 'inherit' : 'none') }">
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label" data-toggle="tooltip" title="Notify alarms with selected priority or higher">Alarms priority</label>
          <option-picker :options="['LOW', 'NORMAL', 'HIGH']" :selected="User.TimeZone" v-on:change="User.TimeZone=$event" :big="true"></option-picker>
        </div>
      </div>
      Alarms notifications priority

      phase names<br>
      colors -> one color per param, decrement brightness for each phase
      colors -> one color per phase, thiker main variable and thin line for secondari


    </div>
  </dlg>
</template>

<script>
  import Dlg from 'Components/Dialog.vue';
  import OptionPicker from 'Components/OptionPicker.vue';
  import moment from 'moment';
  export default {
    data: function() {
      return { View: 'Main', TimeZones: moment.tz.names(), User: {} };
    },
    methods: {
      LoadUser: function(user) {
        this.$set(this, 'User', user);
        this.$refs.Dialog.Show();
      },
      Save: function() {
        this.$refs.Dialog.ShowSpinner = true;

      },
      LoadImage: function() {
        this.$refs.avatar.LoadImage();
      },
      ClearImage: function() {
        this.$refs.avatar.ClearImage();
      }
    },
    components: { OptionPicker, Dlg }
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