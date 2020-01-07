<template>
  <ul class="nav nav-tabs">
    <li v-for="tab in tabs" :class="{ active: (tab == value) }" :key="tab"><a data-toggle="tab">{{ tab }}</a></li>
  </ul>
</template>

<script>
  import scrollingTabs from 'jquery-bootstrap-scrolling-tabs';
  import 'jquery-bootstrap-scrolling-tabs/dist/jquery.scrolling-tabs.css';
  export default {
    props: ['value', 'tabs', 'storage'],
    data() {
      return { };
    },
    watch: {
      value: function(val) {
        if(this.storage != undefined) localStorage.setItem(this.storage, val);
      }
    },
    mounted: function() {
      $(this.$el).scrollingTabs({ enableSwiping: true, tabClickHandler: this.TabChange });
      if(this.storage != undefined) {
        var Find = this.tabs.indexOf(localStorage.getItem(this.storage));
        this.$emit('input', (Find == -1) ? this.tabs[0] : this.tabs[Find]);
      } else {
        var Find = this.tabs.indexOf(this.value);
        this.$emit('input', (Find == -1) ? this.tabs[0] : this.tabs[Find]);
      }
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
</style>