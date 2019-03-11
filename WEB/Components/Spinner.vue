<template>
  <div class="spinner">
    <div>
      <div class="rect1"></div>
      <div class="rect2"></div>
      <div class="rect3"></div>
      <div class="rect4"></div>
      <div class="rect5"></div>
    </div>
  </div>
</template>
<script>
  export default {
    data() {
      return  { };
    }
  }
</script>
<style>
  .spinner {
    margin: auto auto;
    text-align: center;
    font-size: 10px;
    position: absolute;
    top: 0px;
    left: 0px;
    right: 0px;
    bottom: 0px;
    background-color: rgba(255, 255, 255, 0.8);
    z-index: 1000;
  }
  .spinner > div {
    transform: translate(0%, -50%);
    position: relative;
    top: 50%;
    height: 50px;
  }
  .spinner > div > div {
    background-color: #333;
    height: 100%;
    width: 6px;
    display: inline-block;
    -webkit-animation: sk-stretchdelay 1.2s infinite ease-in-out;
    animation: sk-stretchdelay 1.2s infinite ease-in-out;
  }
  .spinner > div > .rect2 {
    -webkit-animation-delay: -1.1s;
    animation-delay: -1.1s;
    background-color: #474544;
  }
  .spinner > div > .rect3 {
    -webkit-animation-delay: -1.0s;
    animation-delay: -1.0s;
    background-color: #e8701c;
  }
  .spinner > div > .rect4 {
    -webkit-animation-delay: -0.9s;
    animation-delay: -0.9s;
    background-color: #474544;
  }
  .spinner > div > .rect5 {
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