function initDraw(barsrc){
	var curVelCanvas = document.getElementById('curVel');
	var cvctx = curVelCanvas.getContext('2d');
	var avrVelCanvas = document.getElementById('avrVel');
	var avctx = avrVelCanvas.getContext('2d');
	var bar = new Image();
	bar.onload = function(){
		cvctx.strokeRect(0, 0, curVelCanvas.width, curVelCanvas.height);
		avctx.strokeRect(0, 0, avrVelCanvas.width, avrVelCanvas.height);
		cvctx.drawImage (bar, 0, 0, 0, bar.height, 0, 0, 0, curVelCanvas.height);
		avctx.drawImage (bar, 0, 0, 0, bar.height, 0, 0, 0, avrVelCanvas.height);
	};
	bar.src = barsrc;
	return bar;
}

function getData(bar){
	var xhr = new XMLHttpRequest();
	xhr.open('post', 'getdata/');
	xhr.onreadystatechange = function(){
		if(xhr.readyState === 4){
			if(xhr.status === 200){
				data = JSON.parse(xhr.responseText);
				document.getElementById("temperature").innerHTML = data[0].fields.temperature+"°C";
				document.getElementById("pressure").innerHTML = data[0].fields.pressure+"hPa";
				var cvpercent = data[0].fields.curVel / 3.5;
				var avpercent = data[0].fields.aveVel / 3.5;
				var curVelCanvas = document.getElementById('curVel');
				var cvctx = curVelCanvas.getContext('2d');
				var avrVelCanvas = document.getElementById('avrVel');
				var avctx = avrVelCanvas.getContext('2d');
				cvctx.clearRect(0, 0, curVelCanvas.height, curVelCanvas.width);
				cvctx.strokeRect(0, 0, curVelCanvas.width, curVelCanvas.height);
				cvctx.drawImage (bar, 0, 0, bar.width * cvpercent, bar.height, 0, 0, curVelCanvas.width * cvpercent, curVelCanvas.height);
				avctx.clearRect(0, 0, avrVelCanvas.height, avrVelCanvas.width);
				avctx.strokeRect(0, 0, avrVelCanvas.width, avrVelCanvas.height);
				avctx.drawImage (bar, 0, 0, bar.width * avpercent, bar.height, 0, 0, avrVelCanvas.width * avpercent, avrVelCanvas.height);
			}
		}
	}
	xhr.send(null);
}
