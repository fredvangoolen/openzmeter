<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="form-control">{{ Selected }}</span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown"><span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li v-for="option in options" v-bind:class="{active: (Selected == option)}"><a @click="OptionClick(option, $event)">{{ option }}</a></li>
        </ul>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    props: ['options', 'selected'],
    data() {
      return { Selected: '#FFFFFF' };
    },
    methods: {
      OptionClick(option, evt) {
        event.preventDefault();
        this.Selected = option;
        this.$emit('OptionChanged', option);
      }
    }, 
    created: function() {
      if(this.selected !== undefined) this.Selected = this.selected;
    }
  }
</script>

<style scoped>
  .btn-group {
    width: 100%;
    text-align: left;
  }
</style>
