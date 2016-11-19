from __future__ import unicode_literals

from django.db import models
from django.utils import timezone

class Data(models.Model):
	#measure time
	measureTime = models.DateTimeField(default=timezone.now)

	#temperature(celcious)
	temperature = models.FloatField()

	def __str__(self):
		return self.measureTime
