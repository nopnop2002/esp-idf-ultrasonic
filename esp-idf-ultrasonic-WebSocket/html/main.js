//document.getElementById("datetime").innerHTML = "WebSocket is not connected";

var websocket = new WebSocket('ws://'+location.hostname+'/');

function sendText(name) {
	console.log('sendText');
	var data = {};
	data["id"] = name;
	console.log('data=', data);
	json_data = JSON.stringify(data);
	console.log('json_data=' + json_data);
	websocket.send(json_data);
}

websocket.onopen = function(evt) {
	console.log('WebSocket connection opened');
	var data = {};
	data["id"] = "init";
	console.log('data=', data);
	json_data = JSON.stringify(data);
	console.log('json_data=' + json_data);
	websocket.send(json_data);
	//document.getElementById("datetime").innerHTML = "WebSocket is connected!";
}

websocket.onmessage = function(evt) {
	var msg = evt.data;
	console.log("msg=" + msg);
	var values = msg.split('\4'); // \4 is EOT
	//console.log("values=" + values);
	switch(values[0]) {
		case 'HEAD':
			console.log("HEAD values[1]=" + values[1]);
			var h1 = document.getElementById( 'header' );
			h1.textContent = values[1];
			break;

		case 'METER':
			console.log("METER values[1]=" + values[1]);
			if (values[1] == "RADIAL") {
				document.getElementById("radial-canvas").style.display = "block";
			}
			if (values[1] == "LINEAR") {
				document.getElementById("linear-canvas").style.display = "block";
			}
			break;

		case 'DATA':
			//console.log("DATA values[1]=" + values[1]);
			var voltage = parseInt(values[1], 10);
			//console.log("DATA voltage=" + voltage);
			gauge1.value = voltage;
			//gauge1.update();
			gauge1.update({ valueText: values[1] });
			gauge2.value = voltage;
			//gauge1.update();
			gauge2.update({ valueText: values[1] });
			break;

		default:
			break;
	}
}

websocket.onclose = function(evt) {
	console.log('Websocket connection closed');
	//document.getElementById("datetime").innerHTML = "WebSocket closed";
}

websocket.onerror = function(evt) {
	console.log('Websocket error: ' + evt);
	//document.getElementById("datetime").innerHTML = "WebSocket error????!!!1!!";
}
