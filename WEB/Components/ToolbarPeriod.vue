<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="input-group-addon"><img src="Imgs/outlook_calendar_work_week.png"></span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ Selected }}<span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li><a href="#">Day</a></li>
          <li><a href="#">Week</a></li>
          <li><a href="#">Month</a></li>
          <li><a href="#">Year</a></li>
        </ul>
      </div>
    </div>
  </div>
</template> 

<script>
  export default {
    data() {
      return { Selected: 'Custom' };
    }
  }
</script>