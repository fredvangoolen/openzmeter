<template>
  <div class="spinner">
    <div v-if="progress==null" class="bars">
      <div class="rect1"></div>
      <div class="rect2"></div>
      <div class="rect3"></div>
      <div class="rect4"></div>
      <div class="rect5"></div>
    </div>
    <div v-else class="progress">
      <div class="bar" :style="{ width: progress + '%' }"></div>
      <div class="text">{{ Round(progress, 0) }}%</div>
    </div>
  </div>
</template>
<script>
  export default {
    props: { progress: Number },
    data: function() {
      return  { };
    }
  }
</script>
<style>
  .spinner {
    margin: auto auto;
    text-align: center;
    position: absolute;
    top: 0px;
    left: 0px;
    right: 0px;
    bottom: 0px;
    background-color: rgba(255, 255, 255, 0.8);
    z-index: 999;
  }
  .spinner > .progress {
    overflow: hidden;
    height: 12px;
    background-color: #f5f5f5;
    border-radius: 4px;
    margin: 3px 6px;
    border: 1px solid #ccc;
    transform: translate(-50%, -50%);
    top: 50%;
    left: 50%;
    position: relative;
    height: 20px;
    max-width: 100px;
    background: #474544;
  }
  .spinner > .progress > .text {
    color: #EEE;
    font-weight: bold;
    font-size: 12px;
    position: absolute;
    width: 100%;
    top: 0px;
  }
  .spinner > .progress > .bar {
    background-color: #e8701c;
    height: 100%;
    box-shadow: inset 0 -1px 0 rgba(0,0,0,.15);
    background-image: linear-gradient(-45deg,rgba(255,255,255,.15) 25%,transparent 25%,transparent 50%,rgba(255,255,255,.15) 50%,rgba(255,255,255,.15) 75%,transparent 75%,transparent);
    background-size: 40px 40px;
    animation: progress-bar 2s linear infinite;
  }
  @keyframes progress-bar {
    from  { background-position: -40px 0; }
    to    { background-position: 0 0; }
  }
  .spinner > .bars {
    transform: translate(0%, -50%);
    position: relative;
    top: 50%;
    height: 50px;
  }
  .spinner > .bars > div {
    background-color: #333;
    height: 100%;
    width: 6px;
    display: inline-block;
    -webkit-animation: sk-stretchdelay 1.2s infinite ease-in-out;
    animation: sk-stretchdelay 1.2s infinite ease-in-out;
  }
  .spinner > .bars > .rect2 {
    -webkit-animation-delay: -1.1s;
    animation-delay: -1.1s;
    background-color: #474544;
  }
  .spinner > .bars > .rect3 {
    -webkit-animation-delay: -1.0s;
    animation-delay: -1.0s;
    background-color: #e8701c;
  }
  .spinner > .bars > .rect4 {
    -webkit-animation-delay: -0.9s;
    animation-delay: -0.9s;
    background-color: #474544;
  }
  .spinner > .bars > .rect5 {
    -webkit-animation-delay: -0.8s;
    animation-delay: -0.8s;
  }
  @-webkit-keyframes sk-stretchdelay {
    0%, 40%, 100% { -webkit-transform: scaleY(0.4) }
    20% { -webkit-transform: scaleY(1.0) }
  }
  @keyframes sk-stretchdelay {
    0%, 40%, 100% {
      transform: scaleY(0.4);
      -webkit-transform: scaleY(0.4);
    }  20% {
      transform: scaleY(1.0);
      -webkit-transform: scaleY(1.0);
    }
  }
</style>