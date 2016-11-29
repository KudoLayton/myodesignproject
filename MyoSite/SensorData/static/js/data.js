function getData(){
	var xhr = new XMLHttpRequest();
	xhr.open('post', 'getdata/');
	xhr.onreadystatechange = function(){
		if(xhr.readyState === 4){
			if(xhr.status === 200){
				data = JSON.parse(xhr.responseText);
				document.getElementById("temperature").innerHTML = data.temperature;
			}
		}
	}
	xhr.send(null);
}
