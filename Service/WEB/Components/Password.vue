<template>
  <div class="input-group">
    <input :type="Show ? 'text' : 'password'" class="form-control" :value="value" @input="$emit('input', $event.target.value)">
    <span class="input-group-btn">
      <button class="btn btn-default" data-toggle="tooltip" title="Show password" type="button" v-if="!Show" @click="Show=true"><img :src="require('Imgs/lock.png')"></button>
      <button class="btn btn-default" data-toggle="tooltip" title="Hide password" type="button" v-if="Show" @click="Show=false"><img :src="require('Imgs/lock_open.png')"></button>
    </span>
  </div>
</template>

<script>
  export default {
    props: { value: String },
    data: function() {
      return { Show: false };
    }
  }
</script>