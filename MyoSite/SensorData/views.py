from django.http import HttpResponse
from django.shortcuts import render
from mjpegtools import MjpegParser

def monitor(request):
	return render(request, 'SensorData/monitor.html', {})