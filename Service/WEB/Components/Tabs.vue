<template>
  <div class="tabs" :class="{ light: light }"></div>
</template>

<script>
//  import scrollingTabs from 'jquery-bootstrap-scrolling-tabs';
//  import 'jquery-bootstrap-scrolling-tabs/dist/jquery.scrolling-tabs.css';
  export default {
    props: { value: String, tabs: Array, storage: String, light: Boolean },
    data() {
      return { Tabs: [] };
    },
    watch: {
      value: function(val) {
        if(this.storage != null) localStorage.setItem(this.storage, val);
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
        var Find = this.tabs.indexOf(localStorage.getItem(this.storage));
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
  .tabs >>> .nav-tabs>li>a {
    border: 1px solid #CCC;
    border-bottom: 1px solid transparent;
    color: #474544;
    background-color: #FFF;
    padding: 2px 6px;
  }
  .tabs >>> .nav-tabs>li>a:hover {
    border: 1px solid #CCC;
    border-bottom: 1px solid transparent;
    background-color: #CCC;
  }
  .tabs >>> .nav-tabs>li.active>a, .nav-tabs>li.active>a:focus, .nav-tabs>li.active>a:hover {
    color: #FFF;
    background-color: #474544;
  }
  .tabs >>> .nav {
    border-bottom: none;
  }
  .tabs >>> .glyphicon-chevron-left {
    background-image: url(../Imgs/resultset_previous.png);
    width: 16px !important;
    height: 16px !important;
    display: inline-block;
  }
  .tabs >>> .glyphicon-chevron-right {
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
  .tabs.light >>> .nav-tabs>li>a {
    border: 1px solid #ccc;
    color: #333;
    border-bottom: 1px solid #eee;
    background-color: #eee;
  }
  .tabs.light >>> .nav-tabs>li>a:hover {
    border-bottom: 1px solid #fff;
    background-color: #fff;
  }
  .tabs.light >>> .nav-tabs>li.active>a, .nav-tabs>li.active>a:focus, .nav-tabs>li.active>a:hover {
    color: #000;
    border-bottom: 1px solid #ccc;
    background-color: #ccc;
  }


</style>