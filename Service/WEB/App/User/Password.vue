<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">User password</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" title="Save changes" @click="Save" :disabled="Error!=null"><img :src="require('Imgs/diskette.png')"></button>
        <button type="button" class="btn btn-default btn-xs" title="Discard" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
    </template>
    <form autocomplete="off">
      <div class="row">
        <div class="col-xs-12" v-if="ShowPrevious">
          <label class="control-label">Previous password</label>
          <password v-model="User.Previous" name="pass"></password>
        </div>
        <div class="col-xs-12">
          <label class="control-label">New password</label>
          <password v-model="User.Password1" name="new_pass"></password>
        </div>
        <div class="col-xs-12">
          <label class="control-label">Confirm password</label>
          <password v-model="User.Password2" name="new_pass2"></password>
        </div>
      </div>
    </form>
  </dlg>
</template>

<script>
  export default {
    props: { user: Object },
    data: function() {
      return { ShowPrevious: true, User: null, ShowSpinner: false, Error: false };
    },
    watch: {
      User: {
        handler: function() {
          this.Error = (this.User.Password1 != this.User.Password2) ? "Passwords do not match" : null;
        },
        deep: true
      }
    },
    created: function() {
      this.User = Object.assign({Previous: '', Password1: '', Password2: ''}, this.user);
      if((localStorage.getItem('UserName') == 'admin') && (this.user.User != 'admin')) this.ShowPrevious = false;
    },
    methods: {
      Save: function() {
        this.ShowSpinner = true;
        this.Post('setUserPassword', { User: this.User.User, Password: this.User.Password1, PrevPassword: this.User.Previous }).then(
          (result) => {
            this.ShowSpinner = false;
            this.$emit('close');
          },
          (fail) => {
            this.ShowSpinner = false;
            if(fail != null) this.$toast.open({type:'error', message: "Failed to save user password<br>" + fail.message, duration: 3000});
          }
        );
      },
    }
  }
</script>