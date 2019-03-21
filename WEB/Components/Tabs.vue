<template>
  <ul class="nav nav-tabs">
    <li v-for="(tab, index) in tabs" v-bind:class="{ active: (Index == index) }"><a data-toggle="tab">{{ tab }}</a></li>
  </ul>
</template>

<script>
  import scrollingTabs from 'jquery-bootstrap-scrolling-tabs';
  import 'jquery-bootstrap-scrolling-tabs/dist/jquery.scrolling-tabs.css';
  export default {
    props: ['tabs', 'variable'],
    data() {
      return { Index: 0 };
    },
    created: function() {
      this.Index = parseInt(localStorage.getItem(this.variable));
      if((this.Index < 0) || (this.Index >= this.tabs.length) || (isNaN(this.Index) === true)) this.Index = 0;
      localStorage.setItem(this.variable, this.Index);
    },
    mounted: function() {
      $(this.$el).scrollingTabs({ tabClickHandler: this.TabChange });
      this.$emit('TabChange', this.Index, this.tabs[this.Index]);
    },
    methods: {
      TabChange: function(sender) {
        this.Index = $(sender['target']).parent().index();
        localStorage.setItem(this.variable, this.Index);
        this.$emit('TabChange', this.Index, $(sender['target']).text());
      }
    }
  }
</script>

<style></style>
