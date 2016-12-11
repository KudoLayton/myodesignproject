/*!
 * jQuery Mjpeg Streaming Player Plugin 
 * This project is modified from zfaas Pty's 'jQuery Clipchamp MJPEG Player Plugin'
 * https://github.com/clipchamp/jquery-clipchamp-mjpeg-player-plugin
 * 
 * Released under the MIT license
 */
 (function($) {
	var DEFAULT_FPS = 24,
		DEFAULT_AUTOLOOP = true,
		JPEG_MAGIG_NUMBER = [0xff, 0xd8, 0xd9, 0xda];

	var requestAnimationFrame = window.requestAnimationFrame || window.webkitRequestAnimationFrame || window.mozRequestAnimationFrame || window.msRequestAnimationFrame || window.setTimeout;

	function playMJPEGInternal(wrapperElement, mjpegUrl, fps, autoloop) {
		fps = (typeof fps === 'number') ? fps : DEFAULT_FPS;
		autoloop = (typeof autoloop === 'boolean') ? autoloop : DEFAULT_AUTOLOOP;

		var playbackFinished = false,
			imageElement = document.createElement('img'),
			jpegUrl;

		imageElement.setAttribute('style', 'width:100%;');
		wrapperElement.appendChild(imageElement);

		var jpegs = [];

		var xhr = new XMLHttpRequest();

		xhr.open('GET', mjpegUrl, true);		
		xhr.overrideMimeType('text/plain; charset=x-user-defined');
		var lastSize = 0;
		xhr.onreadystatechange = function(){
			var newTextReceived;
			if(xhr.readyState > 2){
				newTextReceived = xhr.responseText.substring(lastSize);
				var buf = new ArrayBuffer(newTextReceived.length * 2);
				var bufView = new Uint16Array(buf);
				var arraybuf = new ArrayBuffer(newTextReceived.length);
				var arrayView = new Uint8Array(arraybuf);

				for (var i=0, strLen=newTextReceived.length; i < strLen; i++){
					bufView[i] = newTextReceived.charCodeAt(i);
				}

				var startIndex;
				for (var i=0, ii=bufView.length; i<ii; ++i) {
					arrayView[i] = bufView[i] / 0x100 === 0xf7 ? bufView[i] % 0x100 : bufView[i];

					if (bufView[i] === 0xf7ff && bufView[i+1] === 0xf7d8){
						startIndex = i;
					}
					if (bufView[i] === 0xf7ff && bufView[i+1] === 0xf7d9){
						if(typeof startIndex === 'number'){
							jpegs.push(new Blob([arrayView.subarray(startIndex, i)], {type: 'image/jpeg'}));
							lastSize += i;
						}
					}

				}
			}
		};

		xhr.send();
			function showNextFrame() {
				if (jpegs.length > 0){
					if(jpegs.length > 2){
						jpegs.shift();
					}
					if (imageElement) {
						if (jpegUrl) {
							URL.revokeObjectURL(jpegUrl);
						}
						jpegUrl = URL.createObjectURL(jpegs[0]);
						imageElement.onload = function() {
							setTimeout(function() {
								requestAnimationFrame(showNextFrame);
							}, 1000/fps);
						};
						imageElement.setAttribute('src', jpegUrl);
						
					}else{
						setTimeout(function() {
							requestAnimationFrame(showNextFrame);
						}, 1000/fps);
					}
				}
				else{
					setTimeout(function() {
						requestAnimationFrame(showNextFrame);
					}, 1000);
				}
			};

			setTimeout(function() {
				requestAnimationFrame(showNextFrame);
			}, 1000/fps);
		return {
			finish: function() {
				if (imageElement) {
					imageElement.src = '';
					wrapperElement.removeChild(imageElement);
					imageElement = undefined;
				}
			}
		};	
	};

	// optionally make available as jQuery plugin
	if (typeof $ === 'function') {
		$.fn.clipchamp_mjpeg_player = function(mjpegUrl, fps, autoloop, callback) {
			if (typeof mjpegUrl === 'string') {
				if (typeof callback === 'function') {
					return this.each(function() {
						callback($(this)[0], playMJPEGInternal($(this)[0], mjpegUrl, fps, autoloop));
					});
				} else {
					throw new Error('Callback must be given and must be a function');
				}
			} else {
				throw new Error('MJPEG URL must be a string');
			}

		}
	}

	// optionally provide AMD module definition
	if (typeof define === 'function') {
		define('jquery.clipchamp.mjpeg.player', [], function() {
			return {
				playMJPEG: function(wrapperElement, mjpegUrl, fps, autoloop) {
					if (wrapperElement instanceof Element) {
						if (typeof mjpegUrl === 'string') {
							return playMJPEGInternal(wrapperElement, mjpegUrl, fps, autoloop);
						} else {
							throw new Error('MJPEG URL must be a string');
						}
					} else {
						throw new Error('No parent element was given');
					}
				}
			};
		});
	}
} )


(typeof jQuery === 'function' ? jQuery : undefined);