from django.http import HttpResponse, HttpRequest
from django.shortcuts import render
from django.utils import timezone
from mjpegtools import MjpegParser
from models import Data
from django.views.decorators.csrf import csrf_exempt
from django.core import serializers

def monitor(request):
	try:
		lastdata = Data.objects.latest('id')
		return render(request, 'SensorData/monitor.html', {'ip': lastdata.ip})
	except:
		return render(request, 'SensorData/monitor.html', {'ip': "No Data"})

@csrf_exempt
def getData(request):
	lastdata = Data.objects.latest('id')
	sendData = serializers.serialize("json", [lastdata,])
	return HttpResponse(sendData)


@csrf_exempt
def input(request):
	new_ip = request.META['REMOTE_ADDR']
	new_temp = 0
	new_press = 0
	new_curVel = 0
	new_avrVel = 0
	try:
		if request.method == "POST":
			new_temp = float(request.POST.get('temp'))
			new_press = float(request.POST.get('press'))
			new_curVel = float(request.POST.get('curvel'))
			new_avrVel = float(request.POST.get('avrvel'))
		data = Data(ip=new_ip, measureTime=timezone.now(), temperature=new_temp, curVel = new_curVel, aveVel = new_avrVel, pressure = new_press)
		data.save()
		return HttpResponse('Successfully Store Data!\n')
	except:
		ErrorStr = 'Fail: ' + request.body + '\n'
		return HttpResponse(ErrorStr)
