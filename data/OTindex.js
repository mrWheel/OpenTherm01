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
