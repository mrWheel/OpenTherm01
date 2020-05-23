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

        if(json["otinfo"]["status"] != 0)
        {
            if ( json["otinfo"]["status"] & 2 )
                document.getElementById("CH").style.background="red"
            else
                document.getElementById("CH").style.background="#314b77"

            if ( json["otinfo"]["status"] & 4 )
                document.getElementById("DHW").style.background="red"
            else
                document.getElementById("DHW").style.background="#314b77"

            if ( json["otinfo"]["status"] & 8 )
                document.getElementById("Boiler").style.background="red"
            else
                document.getElementById("Boiler").style.background="#314b77"
        } 
    })
    .catch(function(error) {
        console.log("Error catched")
        var p = document.createElement('p');
        p.appendChild(
        document.createTextNode('Error: ' + error.message)
        );
    });
    
}
