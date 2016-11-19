from __future__ import unicode_literals

from django.db import models
from django.utils import timezone

class Data(models.Model):
	#measure time
	measureTime = DateTimeField(default=timezone.now)

	#temperature(celcious)
	temperature = FloatField()

	def __str__(self):
		return self.measureTime
