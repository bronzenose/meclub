var connection;
var rRoot2 = Math.sqrt(2);

function initWebSocket(){
	connection = new WebSocket('ws://' + location.hostname + ':81/', ['RCTruck']);

	connection.onopen = function () {
		connection.send('Connect ' + new Date());
	};

	connection.onerror = function (error) {
		console.log('WebSocket Error ', error);
	};

	connection.onmessage = function (e) {
		console.log('Server: ', e.data);
		if('busy' == e.data) {
			enableUI(false);
		} else if('ready' == e.data) {
			enableUI(true);
		}
	};

	connection.onclose = function () {
		console.log('WebSocket connection closed');
	};
}

function vJoystickMoved(e) {
	if(0 != e.buttons) {
		//console.log(e);
		var cpxXMax = e.target.clientWidth; // alternative: offsetWidth https://developer.mozilla.org/en-US/docs/Web/API/CSS_Object_Model/Determining_the_dimensions_of_elements
		var cpxYMax = e.target.clientHeight;
		var rX = (e.offsetX/cpxXMax)-0.5;
		var rY = (e.offsetY/cpxYMax)-0.5;
		//console.log(rX, rY);
		vJsWheels(rX, rY);
	}
}

function vSendStop() {
	console.log('sendStop');
	if(connection) {
		connection.send('!');
	}
}

function vSendDrive(rX, rY) {
	// map X and Y to R and L.  X and Y are in range -1 to +1
	var rR = rX + rY; // I want the limit for rL and rR to be -1 to +1. but I want to clip the excess.
	var rL = rY - rX; // divide by rRoot2 for 45 degree rotation without scale
	if(rR > 1) rR = 1;
	else if(rR < -1) rR = -1;
	if(rL > 1) rL = 1;
	else if(rL < -1) rL = -1;
	var strCmd = 'J' + rL.toFixed(3) + ' ' + rR.toFixed(3);
	console.log(strCmd);
	if(connection){
		connection.send(strCmd);
	}
}

function vRcTruckInit() {
	initJoystick();
	initWebSocket();
}

function initJoystick() {
			console.log("touchscreen is", VirtualJoystick.touchScreenAvailable() ? "available" : "not available");
	
	var eContainer = document.getElementById('container');
				var cpxX = eContainer.clientWidth;
				var cpxY = eContainer.clientHeight;
				var cpxBaseXY = 0.5*Math.min(cpxX, cpxY);
			var joystick	= new VirtualJoystick({
				container	: eContainer,
				mouseSupport	: true,
				stationaryBase  : true,
				baseX           : cpxBaseXY,
				baseY           : cpxBaseXY
			});
			joystick.addEventListener('up', function(){
				//console.log('stop')
				vSendStop();
			});

			setInterval(function(){
				if(joystick.isPressed()){
					vSendDrive(-joystick.deltaX()/cpxBaseXY, -joystick.deltaY()/cpxBaseXY);
				}
			}, 1000/30);
}
