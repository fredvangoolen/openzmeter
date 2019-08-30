<template>
  <div>
    <input type="file" ref="ImageChooser" accept="image/*" style="display:none" v-on:change="FileChanged">
    <img id="dynamic" width="128" height="128" :src="ImageData">
  </div>
</template>

<script>
  export default {
    props: ['value'],
    data() {
      return { ImageData: null };
    },
    mounted: function() {
      this.SetImageFile(require('Imgs/avatar.png'));
    },
    methods: {
      LoadImage: function() {
        this.$refs.ImageChooser.click();
      },
      FileChanged: function(files) {
        files = files.target.files;    
        var reader = new FileReader();
        reader.readAsDataURL(files[0]);
        reader.onload = (event) => {
          this.SetImageFile(event.target.result);
          this.$emit('change', this.ImageData);
        };
        reader.onerror = (error) => {
          console.log(error);
        };
      },
      ClearImage: function() {
        this.SetImageFile(require('Imgs/avatar.png'));
        this.$emit('change', '');
      },
      SetImageFile: function(imgFile) {
        const img = new Image();
        img.src = imgFile;
        img.onload = () => {
          const elem = document.createElement('canvas');
          elem.width = 128;
          elem.height = 128;
          const ctx = elem.getContext('2d');
          ctx.beginPath();
          ctx.moveTo(0 + 16, 0);
          ctx.lineTo(128 - 16, 0);
          ctx.quadraticCurveTo(128, 0, 128, 16);
          ctx.lineTo(128, 0 + 128 - 16);
          ctx.quadraticCurveTo(0 + 128, 0 + 128, 0 + 128 - 16, 0 + 128);
          ctx.lineTo(0 + 16, 0 + 128);
          ctx.quadraticCurveTo(0, 0 + 128, 0, 0 + 128 - 16);
          ctx.lineTo(0, 0 + 16);
          ctx.quadraticCurveTo(0, 0, 0 + 16, 0);
          ctx.closePath();
          ctx.clip();
          ctx.drawImage(img, 0, 0, 128, 128);
          this.ImageData = ctx.canvas.toDataURL();
        };
      }
    }
  }
</script>

<style scoped>
  
  
</style>
