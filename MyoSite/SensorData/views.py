from django.http import HttpResponse, HttpRequest
from django.shortcuts import render
from django.utils import timezone
from mjpegtools import MjpegParser
from models import Data
from django.views.decorators.csrf import csrf_exempt
from django.core import serializers

def monitor(request):	
	return render(request, 'SensorData/monitor.html', {})

@csrf_exempt
def getData(request):
	lastdata = Data.objects.latest('id')
	sendData = serializers.serialize("json", [lastdata,])
	return HttpResponse(sendData)


@csrf_exempt
def input(request):
	new_ip = request.META['REMOTE_ADDR']
	new_temp = 0
	if request.method == "POST":
		new_temp = float(request.POST.get('temp'))
	data = Data(ip=new_ip, measureTime=timezone.now(), temperature=new_temp)
	data.save()
	return HttpResponse('Successfully Store Data!\n')
