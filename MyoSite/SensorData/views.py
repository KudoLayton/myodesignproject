from django.http import HttpResponse
from django.shortcuts import render
from mjpegtools import MjpegParser

def monitor(request):
	image = MjpegParser(url='http://192.168.0.6:8080/test.mjpg').serve()
	print image
	return render(request, 'SensorData/monitor.html', {})