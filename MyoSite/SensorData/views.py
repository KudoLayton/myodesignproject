from django.http import HttpResponse, HttpRequest
from django.shortcuts import render
from django.utils import timezone
from mjpegtools import MjpegParser
from models import Data

def monitor(request):
	image = MjpegParser(url='http://192.168.0.6:8080/test.mjpg').serve()
	
	return HttpResponse(image.as_mjpeg())

def input(request):
	new_ip = request.META['REMOTE_ADDR']
	new_temp = 0
	if request.method == "POST":
		new_temp = float(request.POST.get('temp'))
	data = Data(ip=new_ip, measureTime=timezone.now(), temperature=new_temp)
	data.save()
