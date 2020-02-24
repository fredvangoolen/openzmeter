<template>
  <div>
    <ul class="nav nav-tabs" ref="tabs">
      <li v-for="tab in tabs" :class="{ active: (tab==value) }" :key="tab"><a data-toggle="tab">{{ tab }}</a></li>
    </ul>
  </div>
</template>

<script>
  import scrollingTabs from 'jquery-bootstrap-scrolling-tabs';
  import 'jquery-bootstrap-scrolling-tabs/dist/jquery.scrolling-tabs.css';
  export default {
    props: {value: String, tabs: Array, storage: String },
    data() {
      return { };
    },
    watch: {
      value: function(val) {
        if(this.storage != null) localStorage.setItem(this.storage, val);
      }
    },
    created: function() {
      if(this.storage != null) {
        var Find = this.tabs.indexOf(localStorage.getItem(this.storage));
        this.$emit('input', (Find == -1) ? this.tabs[0] : this.tabs[Find]);
      } else {
        var Find = this.tabs.indexOf(this.value);
        this.$emit('input', (Find == -1) ? this.tabs[0] : this.tabs[Find]);
      }
      this.$nextTick(() => {$(this.$refs.tabs).scrollingTabs({ enableSwiping: true, tabClickHandler: this.TabChange }); });
    },
    beforeDestroy: function() {
      $(this.$refs.tabs).scrollingTabs('destroy');
    },
    methods: {
      TabChange: function(sender) {
        this.$emit('input', $(sender['target']).text());
      }
    }
  }
</script>

<style scoped>
  .nav {
    border-bottom: none;
  }
  .glyphicon-chevron-left {
    background-image: url(../Imgs/resultset_previous.png);
    width: 16px !important;
    height: 16px !important;
    display: inline-block;
  }
  .glyphicon-chevron-right {
    background-image: url(../Imgs/resultset_next.png);
    width: 16px !important;
    height: 16px !important;
    display: inline-block;
  }
</style>