<template>
  <div class="text-center">
    <h1>Hey... are you lost?</h1>
    <div class="container">
     	<div class="row">
			    <div class="col-lg-4 col-md-4 col-sm-4 col-xs-12">
               
					<div class="box-part text-center">
                        
            <i class="fa fa-behance fa-3x" aria-hidden="true"></i>
                        
						<div class="title">
							<h3>Project website</h3>
						</div>
                        
						<div class="text">
							<span> Find out about the latest project news, features and much more!<br><em>openZmeter</em> rocks!</span>
						</div>
                        
						<a href="http://www.openzmeter.com/" target='_blank'>Learn More</a>
                        
					 </div>
				</div>	 
				
				 <div class="col-lg-4 col-md-4 col-sm-4 col-xs-12">
               
					<div class="box-part text-center">
					    
					    <i class="fa fa-twitter fa-3x" aria-hidden="true"></i>
                    
						<div class="title">
							<h3>Wiki</h3>
						</div>
                        
						<div class="text">
							<span>Are you a beginner? or do you want to learn all about the underneath electrical theory?<br>Checkout the official knowledge base.</span>
						</div>
                        
						<a href="https://gitlab.com/zredalmeria/openZmeter/wikis/home" target='_blank'>Learn More</a>
                        
					 </div>
				</div>	 
				
				 <div class="col-lg-4 col-md-4 col-sm-4 col-xs-12">
               
					<div class="box-part text-center">
                        
                        <i class="fa fa-facebook fa-3x" aria-hidden="true"></i>
                        
						<div class="title">
							<h3>Forum</h3>
						</div>
                        
						<div class="text">
							<span>Meet other power quality enthusiasts and solve all your questions.<br>We are proud of our community!</span>
						</div>
                        
						<a href="http://forum.openzmeter.com/" target='_blank'>Learn More</a>
                        
					 </div>
				</div>		
		</div>		
</div>
  </div>
</template>

<script>
  export default {
    data () {
      return { };
    }
  }
</script>

<style>
  h3 {
    font-family: 'Josefin Sans', sans-serif;
  }
  .box-part{
    background:#FFF;
    border-radius:10px;
    padding:60px 10px;
    margin:30px 0px;
  }
  .box-part:hover{
    background: #474544;
  }
  .box-part:hover .fa , 
  .box-part:hover .title , 
  .box-part:hover .text ,
  .box-part:hover a{
    color:#FFF;
    -webkit-transition: all 0.2s ease-out;
    -moz-transition: all 0.2s ease-out;
    -o-transition: all 0.2s ease-out;
    transition: all 0.2s ease-out;
  }
  .text{
    margin:20px 0px;
  }
  .fa{
    color:#4183D7;
  }
</style>