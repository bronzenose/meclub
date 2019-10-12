var connection = new WebSocket('ws://' + location.hostname + ':81/', ['RCTruck']);

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

function sliderChange(nSlider) {
	// nothing to do for now.
}

function startWheels() {
	// if we don't force these to be numbers, they are strings until we do math on them
	// BUT the + operator works with strings or numbers so +n1 APPENDS the string n1
	// instead of adding the NUMBER n1.  Bastards.
	var nD = Number(document.getElementById('duration').value);
	var nL = Number(document.getElementById('wheelL').value);
	var nR = Number(document.getElementById('wheelR').value);

	var nDLR = (nD*10000)+(nL*100) + nR;
	var strCmd = 'G' + nDLR.toString();
	console.log(strCmd);
	connection.send(strCmd);
}

function stopWheels() {
	var strCmd = '!';
	console.log(strCmd);
	connection.send(strCmd);
}

function enableID(id, bEnable){
	e = document.getElementById(id);
	e.className=bEnable ? 'enabled' : 'disabled';
	e.disabled=!bEnable;
}

function enableUI(bEnable) {
	// we never disable the 'stop' control.
	['wheelL', 'wheelR', 'duration', 'go'].forEach(function(strid, i, rg) {enableID(strid, bEnable);});
}