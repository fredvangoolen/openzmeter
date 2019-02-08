<template>
  <div class="btn-group">
    <button type="button" class="btn btn-default btn-xs disabled" data-toggle="tooltip" title="Show phase R"><img src="Imgs/phase_r.png"></button>
    <button type="button" class="btn btn-default btn-xs disabled" data-toggle="tooltip" title="Show phase S"><img src="Imgs/phase_s.png"></button>
    <button type="button" class="btn btn-default btn-xs disabled" data-toggle="tooltip" title="Show phase T"><img src="Imgs/phase_t.png"></button>
    <button type="button" class="btn btn-default btn-xs disabled" data-toggle="tooltip" title="Show phase N"><img src="Imgs/phase_n.png"></button>
  </div>
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