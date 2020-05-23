static const char indexPage[] =
R"(
<!DOCTYPE html>
<head>
<meta charset=utf-8>
<meta http-equiv=X-UA-Compatible content="IE=edge">
<meta name=viewport content="width=device-width,initial-scale=1">
<link rel=icon href=/favicon.ico> <title>OpenTherm GW</title>
<link href="https://fonts.googleapis.com/css?family=Dosis:400,700" rel="stylesheet">
<style>
* {
	box-sizing: border-box
}
  
html {
	-webkit-font-smoothing: antialiased;
	-moz-osx-font-smoothing: grayscale;
	font-family: 'Dosis', sans-serif;
	line-height: 1.6;
	color: #666;
	background: #F6F6F6;
}
 
.outer {
	display: flex;
	flex-wrap: wrap;
}

@media screen and (min-width: 600px) {
	.card {
	  flex: 1 1 calc(50% - 2rem);
	}
}
  
@media screen and (min-width: 900px) {
	.card {
	  flex: 1 1 calc(33% - 2rem);
	}
}

.card {
	margin: 1rem;
	background: white;
	box-shadow: 2px 4px 25px rgba(0, 0, 0, .1);
	border-radius: 12px;
	overflow: hidden;
	transition: all .2s linear;
	max-width: 200px;  
	min-width: 180px;
}

.card:hover {
	box-shadow: 2px 8px 45px rgba(0, 0, 0, .15);
	transform: translate3D(0, -2px, 0);
}
  
h1 {
	text-align: center;
	padding: 0.5rem 1.5rem;
	background: #314b77;
	margin: 0 0 0rem 0;
	font-size: 1.5rem;
	color: white;
}

h2 {
	text-align: center;
	font-size: 3rem;
	padding: 0.2rem 0.2rem;	
	margin: 0;
}

h3 {
	text-align: center;
	font-size: 1.3rem;
	font-family: 'Heiti TC';
	padding: 0.2rem 0.2rem;	
	margin: 0;
}
</style>
<script>
const APIGW='http://'+window.location.host+'/api/';

function update()
{
    console.log("About to call API")
    fetch(APIGW+"v0/status")
    .then(response => response.json())
    .then(json => {
        console.log("Received: ", json["otinfo"])

        
        for( var key in json["otinfo"] ){
            console.log("Key now is "+key);
            console.log("Value is "+json["otinfo"][key]);
            
            el = document.getElementById(key)
            if (el)
                el.innerHTML = json["otinfo"][key];
            
        }

        
        let stat = parseInt(json["otinfo"]["status"]);

        if ( stat & 2 )
            document.getElementById("CH_h").style.background="red"
        else
            document.getElementById("CH_h").style.background="#314b77"

        if ( stat & 4 )
            document.getElementById("DHW_h").style.background="red"
        else
            document.getElementById("DHW_h").style.background="#314b77"

        if ( stat & 8 )
            document.getElementById("boiler_h").style.background="red"
        else
            document.getElementById("boiler_h").style.background="#314b77"
    
    })
    .catch(function(error) {
        console.log("Error catched")
        var p = document.createElement('p');
        p.appendChild(
        document.createTextNode('Error: ' + error.message)
        );
    });
    
}
</script>
</head>

<body><noscript><strong>Please enable JavaScript to continue.</strong></noscript>
<div class=outer>
<div class=card id=dhw>
<h1 id=DHW_h>DHW</h1>
<h2 id=Tdhw>0.0</h2>
<h2 id=TdhwSet>0.0</h2>
</div>
<div class=card id=boiler>
<h1 id=boiler_h>Boiler</h1>

<h2 id=Tboiler>0.0</h2>
<h2 id=Tret>0.0</h2>

<h2 id=modulation_level>0.0</h2>
<h2 id=status>status</h2>

</div>
<div class=card id=ch>
<h1 id=CH_h>CH</h1>
<h2 id=Tset>0.0</h2>

<h2 id=ch_pres>0.0</h2>
<h2 id=TrSet>0.0</h2>
<h2 id=Tr>0.0</h2>

</div>
</div>
<script>
update() // fill fist-time data
var timer = setInterval(update, 15 * 1000) // update every 15 seconds
</script>
</body>
</html>
)";
