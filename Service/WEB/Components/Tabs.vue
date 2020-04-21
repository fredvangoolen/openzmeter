<template>
  <div class="tabs"></div>
</template>

<script>
  import scrollingTabs from 'jquery-bootstrap-scrolling-tabs';
  import 'jquery-bootstrap-scrolling-tabs/dist/jquery.scrolling-tabs.css';
  export default {
    props: { value: String, tabs: Array, storage: String },
    data() {
      return { Tabs: [] };
    },
    watch: {
      value: function(val) {
        if(this.storage != null) this.LocalStorage_Set(this.storage, val);
        var Find = this.tabs.indexOf(this.value);
        if(Find == -1) {
          this.$emit('input', this.tabs[0]);
        } else {
          for(var i in this.Tabs) this.Tabs[i].active = (this.Tabs[i].title == this.value);
          $(this.$el).scrollingTabs('refresh', { forceActiveTab: true });
        }
      },
      tabs: function(val, prev) {
        if(val.length == prev.length) {
          var Diff = false;
          for(var i in val) {
            if(val[i] != prev[i]) Diff = true;
          }
          if(Diff == false) return;
        }
        $(this.$el).scrollingTabs('destroy');
        this.Tabs = [];
        for(var i in this.tabs) this.Tabs.push({ paneId: i, title: this.tabs[i], active: (this.tabs[i] == this.value), disabled: false });
        $(this.$el).scrollingTabs({ enableSwiping: true, tabClickHandler: this.TabChange, tabs: this.Tabs, ignoreTabPanes: true });
        var Find = this.tabs.indexOf(this.value);
        if(Find == -1) this.$emit('input', (Find == -1) ? this.tabs[0] : this.tabs[Find]);
      },
    },
    mounted: function() {
      this.Tabs = [];
      for(var i in this.tabs) this.Tabs.push({ paneId: i, title: this.tabs[i], active: (this.tabs[i] == this.value), disabled: false });
      $(this.$el).scrollingTabs({ enableSwiping: true, tabClickHandler: this.TabChange, tabs: this.Tabs, ignoreTabPanes: true });
      if(this.storage != null) {
        var Find = this.tabs.indexOf(this.LocalStorage_Get(this.storage));
      } else {
        var Find = this.tabs.indexOf(this.value);
      }
      this.$emit('input', (Find == -1) ? this.tabs[0] : this.tabs[Find]);
    },
    beforeDestroy: function() {
      $(this.$el).scrollingTabs('destroy');
    },
    methods: {
      TabChange: function(sender) {
        this.$emit('input', $(sender['target']).text());
      }
    }
  }
</script>

<style scoped>
  .tabs >>> .nav {
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
  .tabs >>> div {
    height: 26px;
  }
  .tabs >>> .scrtabs-tab-scroll-arrow {
    height: 26px !important;
    padding-top: 3px !important;
    width: 18px;
  }
  .tabs >>> a {
    padding: 2px 6px;
  }
</style>