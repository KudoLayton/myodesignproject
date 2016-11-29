from django.conf.urls import url
from . import views

urlpatterns = [
	url(r'^$', views.monitor, name='monitor'),
	url(r'^/input/$', views.input, name='input'),
]