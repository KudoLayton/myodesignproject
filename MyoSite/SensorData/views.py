from django.shortcuts import render
from mjpegtools import MjpegParser

def monitor(request):
	image = MjpegParser(url='http://path-to-your-camera-mjpeg').serve()
	return HttpResponse(image)
	#return render(request, 'SensorData/monitor.html', {})