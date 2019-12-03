<template>
  <ul class="nav nav-tabs">
    <li v-for="tab in tabs" v-bind:class="{ active: (tab == Value) }" :index="tab"><a data-toggle="tab">{{ tab }}</a></li>
  </ul>
</template>

<script>
  import scrollingTabs from 'jquery-bootstrap-scrolling-tabs';
  import 'jquery-bootstrap-scrolling-tabs/dist/jquery.scrolling-tabs.css';
  export default {
    props: ['value', 'tabs', 'storage'],
    data() {
      return { Value: null };
    },
    created: function() {
      if(this.storage != undefined) {
        var Find = this.tabs.indexOf(localStorage.getItem(this.storage));
        this.Value = (Find == -1) ? this.tabs[0] : this.tabs[Find];
      } else {
        var Find = this.tabs.indexOf(this.value);
        this.Value = (Find == -1) ? this.tabs[0] : this.tabs[Find];
      }
      this.$emit('input', this.Value);        
    },
    mounted: function() {
      $(this.$el).scrollingTabs({ enableSwiping: true, tabClickHandler: this.TabChange });
    },
    beforeDestroy: function() {
      $(this.$el).scrollingTabs('destroy');
    },
    methods: {
      TabChange: function(sender) {
        if(this.storage != undefined) {
          localStorage.setItem(this.storage, $(sender['target']).text());
        }
        this.$emit('input', $(sender['target']).text());
      }
    }
  }
</script>

<style></style>
