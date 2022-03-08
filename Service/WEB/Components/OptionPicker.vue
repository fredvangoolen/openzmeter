<template>
  <z-button-group>
    <div class="input-group input-group-xs">
      <span class="input-group-addon" v-if="prefix!=null">{{ prefix }}</span>
      <span class="form-control">{{ Selected.name }}</span>
      <span class="input-group-addon postfix" v-if="postfix!=null">{{ postfix }}</span>
      <div class="input-group-btn" ref="button">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" ><span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li v-for="option in options" :key="option.value" :class="{ active: (value == option.value) }"><a @click="$emit('input', option.value)">{{ option.name }}</a></li>
        </ul>
      </div>
    </div>
  </z-button-group>
</template>

<script>
  export default {
    props: { options: Array, value: [Boolean, String, Number], prefix: String, postfix: String },
    data() {
      return { Selected: {} };
    },
    mounted: function() {
      $(this.$refs.button).on("shown.bs.dropdown", function() {
        var Item = $(this).find(".dropdown-menu li.active");
        if(Item.length == 1) {
          var top = $(this).children('ul').scrollTop() + Item.offset().top - $(this).children('ul').offset().top;
          $(this).children('ul').scrollTop(top);
        }
      });
      for(var idx in this.options) {
        if(this.options[idx].value !== this.value) continue;
        this.Selected = this.options[idx];
        return;
      }
    },
    watch: {
      value: function(val) {
        for(var idx in this.options) {
          if(this.options[idx].value !== val) continue;
          this.Selected = this.options[idx];
          return;
        }
      }
    }
  }
</script>

<style scoped>
  .btn-group {
    width: 100%;
    text-align: left;
  }
  .btn-group ul {
    max-height: 142px;
    overflow-y: auto;
  }
  .postfix {
    border-left: none;
  }
</style>