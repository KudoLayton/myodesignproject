from __future__ import unicode_literals

from django.db import models
from django.utils import timezone

class Data(models.Model):
	#ip
	ip = models.CharField(max_length=100)

	#measure time
	measureTime = models.DateTimeField(default=timezone.now)

	#temperature(celcious)
	temperature = models.FloatField()

	#current velocity(m/s)
	curVel = models.FloatField()

	#average velocity(m/s)
	aveVel = models.FloatField()

	#pressure velocity(hPa)
	pressure = models.FloatField()

	def __str__(self):
		return self.measureTime + ": " + temperature
