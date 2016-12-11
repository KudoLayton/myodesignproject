function initDraw(barsrc){
	var curVelCanvas = document.getElementById('curVel');
	var cvctx = curVelCanvas.getContext('2d');
	var avrVelCanvas = document.getElementById('avrVel');
	var avctx = avrVelCanvas.getContext('2d');
	var bar = new Image();
	var percent = 0;
	bar.onload = function(){
		cvctx.strokeRect(0, canvas.height / 2 - canvas.height/2, canvas.width, canvas.height);
		avctx.strokeRect(0, canvas.height / 2 - canvas.height/2, canvas.width, canvas.height);
		cvctx.drawImage (bar, 0, 0, 0, bar.height, 0, canvas.height / 2 - canvas.height/2, 0, canvas.height);
		avctx.drawImage (bar, 0, 0, 0, bar.height, 0, canvas.height / 2 - canvas.height/2, 0, canvas.height);
	};
	bar.src = barsrc;

	function getData(){
		var xhr = new XMLHttpRequest();
		xhr.open('post', 'getdata/');
		var percent;
		xhr.onreadystatechange = function(){
			if(xhr.readyState === 4){
				if(xhr.status === 200){
					data = JSON.parse(xhr.responseText);
					document.getElementById("temperature").innerHTML = data[0].fields.temperature+"°C";
					document.getElementById("pressure").innerHTML = data[0].fields.pressure+"hPa";
					percent = data[0].fields.curVel / 3.5;
					cvctx.clearRect(0, 0, canvas.height, canvas.width);
					cvctx.strokeRect(0, canvas.height / 2 - canvas.height/2, canvas.width, canvas.height);
					cvctx.drawImage (bar, 0, 0, bar.width * percent, bar.height, 0, canvas.height / 2 - canvas.height/2, canvas.width * percent, canvas.height);
					percent = data[0].fields.aveVel / 3.5;
					avctx.clearRect(0, 0, canvas.height, canvas.width);
					avctx.strokeRect(0, canvas.height / 2 - canvas.height/2, canvas.width, canvas.height);
					avctx.drawImage (bar, 0, 0, bar.width * percent, bar.height, 0, canvas.height / 2 - canvas.height/2, canvas.width * percent, canvas.height);
				}
			}
		}
		xhr.send(null);
	}
}


