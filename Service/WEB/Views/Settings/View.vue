<template>
  <q-page class="q-gutter-sm" v-show="Loaded==null">
    <devices @done="Done" @queries="AddQueries" />
    <tariffs @done="Done" @queries="AddQueries" />
    <connectivity @done="Done" @queries="AddQueries" />
    <users @done="Done" @queries="AddQueries" />
  </q-page>
</template>

<script>
  import Users from './Users.vue';
  import Devices from './Devices.vue';
  import Tariffs from './Tariffs.vue';
  import Connectivity from './Connectivity.vue';
  import { mdiCheckAll } from '@quasar/extras/mdi-v6';
  export default {
    data: function() {
      return { Queries: 0, Pending: 0, Loaded: null };
    },
    created: function() {
      this.mdiCheckAll = mdiCheckAll;
      this.Loaded = this.$q.notify({ group: false, timeout: 0, position: 'bottom-right', spinner: true, message: 'Loading modules...', caption: '0%' });
    },
    methods: {
      UpdateNotification: function() {
        if(this.Loaded != null) this.Loaded({ caption: Math.trunc(((this.Queries-this.Pending)/this.Queries)*100, 0) + '%' });
      },
      AddQueries: function(val) {
        this.Queries += val;
        this.Pending += val;
        this.UpdateNotification();
      },
      Done: function() {
        if(this.Pending > 0) this.Pending--;
        this.UpdateNotification();
        if((this.Pending == 0) && (this.Loaded != null)){
          this.Loaded({ type: 'positive', icon: mdiCheckAll, spinner: false, caption: 'Done!', timeout: 1000 });
          this.Loaded = null;
        }
      }
    },
    components: { Users, Devices, Tariffs, Connectivity }
  }
</script>