<template>
  <div>
    <div class="col-lg-12 text-center">
      <h2>Hey... are you lost?</h2>
    </div>
    <div class="row q-pa-sm q-col-gutter-sm">
      <div class="col-md-4 col-xs-12">
        <q-btn color="primary" class="full-width full-height" unelevated  @click="open('http://www.openzmeter.com/')">
          <div>
            <h3>Project website</h3>
            <span>Find out about the latest project news, features and much more!<br><em>openZmeter</em> rocks!</span>
          </div>
        </q-btn>
      </div>
      <div class="col-md-4 col-xs-12">
        <q-btn color="primary" class="full-width full-height" unelevated @click="open('https://gitlab.com/zredalmeria/openZmeter/wikis/home')">
          <div>
            <h3>Wiki</h3>
            <span>Are you a beginner? or do you want to learn all about the underneath electrical theory?<br>Checkout the official knowledge base.</span>
          </div>
        </q-btn>
      </div>
      <div class="col-md-4 col-xs-12">
        <q-btn color="primary" class="full-width full-height" unelevated @click="open('http://forum.openzmeter.com')">
          <div>
            <h3>Forum</h3>
            <span>Meet other power quality enthusiasts and solve all your questions.<br>We are proud of our community!</span>
          </div>
        </q-btn>
      </div>
    </div>
  </div>
</template>

<script>
  import { openURL } from 'quasar';
  export default {
    data: function() {
      return { };
    },
    methods: {
      open: function(link) {
        openURL(link);
      }
    }
  }
</script>