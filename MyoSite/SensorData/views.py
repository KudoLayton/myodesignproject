from django.shortcuts import render

def monitor(request):
	return render(request, 'SensorData/monitor.html', {})