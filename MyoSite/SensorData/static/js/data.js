function getData(show){
	var xhr = new XMLHttpRequest();
	xhr.open('post', 'getdata/');
	xhr.onreadystatechange = show;
	xhr.send(null);
}

function showData(){
	if(xhr.readyState === 4){
		if(xhr.status === 200){
			data = JSON.parse(xhr.responseText);
			document.getElementById("temperature").innerHTML = data[0].fields.temperature; 
			setInterval(getData, 1000);
		}
	}
}