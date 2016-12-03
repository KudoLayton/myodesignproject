#include<stdio.h>
#include<curl/curl.h>

int main() {
	CURL *curl;
	CURLcode res;
	
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.0.83:8000/input/");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "temp=28.2");
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) 
			fprintf(stderr, "curl_Easy_perform() failed: %s\n", curl_easy_strerror(res));
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
	return 0;
}
