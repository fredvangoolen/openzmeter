<template>
  <b-button-group>
    <b-button size="xs" class="btn btn-default disabled" data-toggle="tooltip" title="Show phase R"><img src="Imgs/phase_r.png"></b-button>
    <b-button class="btn btn-default btn-xs disabled" data-toggle="tooltip" title="Show phase S"><img src="Imgs/phase_s.png"></b-button>
    <b-button class="btn btn-default btn-xs disabled" data-toggle="tooltip" title="Show phase T"><img src="Imgs/phase_t.png"></b-button>
    <b-button class="btn btn-default btn-xs disabled" data-toggle="tooltip" title="Show phase N"><img src="Imgs/phase_n.png"></b-button>
  </b-button-group>
</template> 

<script>
export default {
    props: ['phases', 'multi'],
    data() {
      return {}
    }
}  
</script>
  
<style>
</style>