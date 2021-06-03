#define APP_CPU 1
#define PRO_CPU 0

#include "OV2640.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>

#include <esp_bt.h>
#include <esp_wifi.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <ArduinoJson.h>
#include <WebOTA.h>
#include <Preferences.h>

Preferences preferences;

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"

#include "home_wifi_multi.h"

OV2640 cam;

WebServer server(80);

void handleJPGSstream(void);
void streamCB(void * pvParameters);
void camCB(void* pvParameters);
char* allocateMemory(char* aPtr, size_t aSize);

void handleJPG(void);

void set_handler();
void get_handler();

void control3_handler();
void restart_handler();
void activatewebota_handler();
void reset_handler();
void handleNotFound();

unsigned int counter = 0;

//HTML-----------------------------------
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!doctype html>
<html>
		<head>
				<meta charset="utf-8">
				<meta name="viewport" content="width=device-width,initial-scale=1">
				<title>ESP32-CAM Stream and Save to SD</title>
				<style>
body{font-family:Arial,Helvetica,sans-serif;background:#181818;color:#EFEFEF;font-size:16px}h2{font-size:18px}section.main{display:flex}#menu,section.main{flex-direction:column}#menu{display:none;flex-wrap:nowrap;min-width:340px;background:#363636;padding:8px;border-radius:4px;margin-top:-10px;margin-right:10px}#content{display:flex;flex-wrap:wrap;align-items:stretch}figure{padding:0;margin:0;-webkit-margin-before:0;margin-block-start:0;-webkit-margin-after:0;margin-block-end:0;-webkit-margin-start:0;margin-inline-start:0;-webkit-margin-end:0;margin-inline-end:0}figure img{display:block;width:100%;height:auto;border-radius:4px;margin-top:8px}@media (min-width: 800px) and (orientation:landscape){#content{display:flex;flex-wrap:nowrap;align-items:stretch}figure img{display:block;max-width:100%;max-height:calc(100vh - 40px);width:auto;height:auto}figure{padding:0;margin:0;-webkit-margin-before:0;margin-block-start:0;-webkit-margin-after:0;margin-block-end:0;-webkit-margin-start:0;margin-inline-start:0;-webkit-margin-end:0;margin-inline-end:0}}section#buttons{display:flex;flex-wrap:nowrap;justify-content:space-between}#nav-toggle{cursor:pointer;display:block}#nav-toggle-cb{outline:0;opacity:0;width:0;height:0}#nav-toggle-cb:checked+#menu{display:flex}.input-group{display:flex;flex-wrap:nowrap;line-height:22px;margin:5px 0}.input-group>label{display:inline-block;padding-right:10px;min-width:47%}.input-group input,.input-group select{flex-grow:1}.range-max,.range-min{display:inline-block;padding:0 5px}button{display:block;margin:5px;padding:0 12px;border:0;line-height:28px;cursor:pointer;color:#fff;background:#ff3034;border-radius:5px;font-size:16px;outline:0}button:hover{background:#ff494d}button:active{background:#f21c21}button.disabled{cursor:default;background:#a0a0a0}input[type=range]{-webkit-appearance:none;width:100%;height:22px;background:#363636;cursor:pointer;margin:0}input[type=range]:focus{outline:0}input[type=range]::-webkit-slider-runnable-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}input[type=range]::-webkit-slider-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;-webkit-appearance:none;margin-top:-11.5px}input[type=range]:focus::-webkit-slider-runnable-track{background:#EFEFEF}input[type=range]::-moz-range-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}input[type=range]::-moz-range-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer}input[type=range]::-ms-track{width:100%;height:2px;cursor:pointer;background:0 0;border-color:transparent;color:transparent}input[type=range]::-ms-fill-lower{background:#EFEFEF;border:0 solid #EFEFEF;border-radius:0}input[type=range]::-ms-fill-upper{background:#EFEFEF;border:0 solid #EFEFEF;border-radius:0}input[type=range]::-ms-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;height:2px}input[type=range]:focus::-ms-fill-lower{background:#EFEFEF}input[type=range]:focus::-ms-fill-upper{background:#363636}.switch{display:block;position:relative;line-height:22px;font-size:16px;height:22px}.switch input{outline:0;opacity:0;width:0;height:0}.slider{width:50px;height:22px;border-radius:22px;cursor:pointer;background-color:grey}.slider,.slider:before{display:inline-block;transition:.4s}.slider:before{position:relative;content:"";border-radius:50%;height:16px;width:16px;left:4px;top:3px;background-color:#fff}input:checked+.slider{background-color:#ff3034}input:checked+.slider:before{-webkit-transform:translateX(26px);transform:translateX(26px)}select{border:1px solid #363636;font-size:14px;height:22px;outline:0;border-radius:5px}.image-container{position:relative;min-width:160px}.close{position:absolute;right:5px;top:5px;background:#ff3034;width:16px;height:16px;border-radius:100px;color:#fff;text-align:center;line-height:18px;cursor:pointer}.hidden{display:none}
				</style>
		</head>
		<body> 
				<section class="main">
	<div id="logo">
								<label for="nav-toggle-cb" id="nav-toggle">&#9776;&nbsp;&nbsp;Toggle settings</label>
						</div>
						<div id="content">
								<div id="sidebar">
										<input type="checkbox" id="nav-toggle-cb" checked="checked">
										<nav id="menu">
												<div class="input-group" id="framesize-group">
														<label for="framesize">Resolution</label>
														<select id="framesize" class="default-action">
																<option value="13">1600x1200</option>
																<option value="12">1280x1024</option>
																<option value="11">1280x768</option>
																<option value="10">1024x768</option>
																<option value="9">800x600</option>
																<option value="8">640x480</option>

														</select>
												</div>
												<div class="input-group" id="quality-group">
														<label for="quality">Quality</label>
														<div class="range-min">10</div>
														<input type="range" id="quality" min="10" max="63" value="10" class="default-action">
														<div class="range-max">63</div>
												</div>
												<div class="input-group" id="brightness-group">
														<label for="brightness">Brightness</label>
														<div class="range-min">-2</div>
														<input type="range" id="brightness" min="-2" max="2" value="0" class="default-action">
														<div class="range-max">2</div>
												</div>
												<div class="input-group" id="contrast-group">
														<label for="contrast">Contrast</label>
														<div class="range-min">-2</div>
														<input type="range" id="contrast" min="-2" max="2" value="0" class="default-action">
														<div class="range-max">2</div>
												</div>
												<div class="input-group" id="saturation-group">
														<label for="saturation">Saturation</label>
														<div class="range-min">-2</div>
														<input type="range" id="saturation" min="-2" max="2" value="0" class="default-action">
														<div class="range-max">2</div>
												</div>
												<div class="input-group" id="special_effect-group">
														<label for="special_effect">Special Effect</label>
														<select id="special_effect" class="default-action">
																<option value="0" selected="selected">No Effect</option>
																<option value="1">Negative</option>
																<option value="2">Grayscale</option>
																<option value="3">Red Tint</option>
																<option value="4">Green Tint</option>
																<option value="5">Blue Tint</option>
																<option value="6">Sepia</option>
														</select>
												</div>
												<div class="input-group" id="awb-group">
														<label for="awb">AWB</label>
														<div class="switch">
																<input id="awb" type="checkbox" class="default-action" checked="checked">
																<label class="slider" for="awb"></label>
														</div>
												</div>
												<div class="input-group" id="awb_gain-group">
														<label for="awb_gain">AWB Gain</label>
														<div class="switch">
																<input id="awb_gain" type="checkbox" class="default-action" checked="checked">
																<label class="slider" for="awb_gain"></label>
														</div>
												</div>
												<div class="input-group" id="wb_mode-group">
														<label for="wb_mode">WB Mode</label>
														<select id="wb_mode" class="default-action">
																<option value="0" selected="selected">Auto</option>
																<option value="1">Sunny</option>
																<option value="2">Cloudy</option>
																<option value="3">Office</option>
																<option value="4">Home</option>
														</select>
												</div>
												<div class="input-group" id="aec-group">
														<label for="aec">AEC SENSOR</label>
														<div class="switch">
																<input id="aec" type="checkbox" class="default-action" checked="checked">
																<label class="slider" for="aec"></label>
														</div>
												</div>
												<div class="input-group" id="aec2-group">
														<label for="aec2">AEC DSP</label>
														<div class="switch">
																<input id="aec2" type="checkbox" class="default-action" checked="checked">
																<label class="slider" for="aec2"></label>
														</div>
												</div>
												<div class="input-group" id="ae_level-group">
														<label for="ae_level">AE Level</label>
														<div class="range-min">-2</div>
														<input type="range" id="ae_level" min="-2" max="2" value="0" class="default-action">
														<div class="range-max">2</div>
												</div>
												<div class="input-group" id="aec_value-group">
														<label for="aec_value">Exposure</label>
														<div class="range-min">0</div>
														<input type="range" id="aec_value" min="0" max="1200" value="204" class="default-action">
														<div class="range-max">1200</div>
												</div>
												<div class="input-group" id="agc-group">
														<label for="agc">AGC</label>
														<div class="switch">
																<input id="agc" type="checkbox" class="default-action" checked="checked">
																<label class="slider" for="agc"></label>
														</div>
												</div>
												<div class="input-group hidden" id="agc_gain-group">
														<label for="agc_gain">Gain</label>
														<div class="range-min">1x</div>
														<input type="range" id="agc_gain" min="0" max="30" value="5" class="default-action">
														<div class="range-max">31x</div>
												</div>
												<div class="input-group" id="gainceiling-group">
														<label for="gainceiling">Gain Ceiling</label>
														<div class="range-min">2x</div>
														<input type="range" id="gainceiling" min="0" max="6" value="0" class="default-action">
														<div class="range-max">128x</div>
												</div>
												<div class="input-group" id="bpc-group">
														<label for="bpc">BPC</label>
														<div class="switch">
																<input id="bpc" type="checkbox" class="default-action">
																<label class="slider" for="bpc"></label>
														</div>
												</div>
												<div class="input-group" id="wpc-group">
														<label for="wpc">WPC</label>
														<div class="switch">
																<input id="wpc" type="checkbox" class="default-action" checked="checked">
																<label class="slider" for="wpc"></label>
														</div>
												</div>
												<div class="input-group" id="raw_gma-group">
														<label for="raw_gma">Raw GMA</label>
														<div class="switch">
																<input id="raw_gma" type="checkbox" class="default-action" checked="checked">
																<label class="slider" for="raw_gma"></label>
														</div>
												</div>
												<div class="input-group" id="lenc-group">
														<label for="lenc">Lens Correction</label>
														<div class="switch">
																<input id="lenc" type="checkbox" class="default-action" checked="checked">
																<label class="slider" for="lenc"></label>
														</div>
												</div>
												<div class="input-group" id="hmirror-group">
														<label for="hmirror">H-Mirror</label>
														<div class="switch">
																<input id="hmirror" type="checkbox" class="default-action" checked="checked">
																<label class="slider" for="hmirror"></label>
														</div>
												</div>
												<div class="input-group" id="vflip-group">
														<label for="vflip">V-Flip</label>
														<div class="switch">
																<input id="vflip" type="checkbox" class="default-action" checked="checked">
																<label class="slider" for="vflip"></label>
														</div>
												</div>
												<div class="input-group" id="dcw-group">
														<label for="dcw">DCW (Downsize EN)</label>
														<div class="switch">
																<input id="dcw" type="checkbox" class="default-action" checked="checked">
																<label class="slider" for="dcw"></label>
														</div>
												</div>
												<div class="input-group" id="colorbar-group">
														<label for="colorbar">Color Bar</label>
														<div class="switch">
																<input id="colorbar" type="checkbox" class="default-action">
																<label class="slider" for="colorbar"></label>
														</div>
												</div>											 
												<section id="buttons">
														<button id="get-still">Get Still</button>
														<button id="toggle-stream">Start Stream</button>													
												</section>
										</nav>
								</div>
								<figure>
										<div id="stream-container" class="image-container hidden">
												<div class="close" id="close-stream">×</div>
												<img id="stream" src="">
										</div>
								</figure>
						</div>
				</section>				
				<script>
			document.addEventListener('DOMContentLoaded', function () {
					function b(B) {
							let C;
							switch (B.type) {
							case 'checkbox':
									C = B.checked ? 1 : 0;
									break;
							case 'range':
							case 'select-one':
									C = B.value;
									break;
							case 'button':
							case 'submit':
									C = '1';
									break;
							default:
									return;
							}
							const D = `${c}/set?var=${B.id}&val=${C}`;
							fetch(D).then(E => {
									console.log(`request to ${D} finished, status: ${E.status}`)
							})
					}
					var c = document.location.origin;
					const e = B => {
									B.classList.add('hidden')
							},
							f = B => {
									B.classList.remove('hidden')
							},
							g = B => {
									B.classList.add('disabled'), B.disabled = !0
							},
							h = B => {
									B.classList.remove('disabled'), B.disabled = !1
							},
							i = (B, C, D) => {
									D = !(null != D) || D;
									let E;
									'checkbox' === B.type ? (E = B.checked, C = !!C, B.checked = C) : (E = B.value, B.value = C), D && E !== C ? b(B) : !D && ('aec' === B.id ? C ? e(v) : f(v) : 'agc' === B.id ? C ? (f(t), e(s)) : (e(t), f(s)) : 'awb_gain' === B.id ? C ? f(x) : e(x) : 'face_recognize' === B.id && (C ? h(n) : g(n)))
							};
					document.querySelectorAll('.close').forEach(B => {
							B.onclick = () => {
									e(B.parentNode)
							}
					}), fetch('http://192.168.31.61/get?cachebuster=' + Date.now()).then(function (B) {
							return B.json()
					}).then(function (B) {
							document.querySelectorAll('.default-action').forEach(C => {
									i(C, B[C.id], !1)
							});
							setTimeout(function(){q();},500);
					});
					const j = document.getElementById('stream'),
							k = document.getElementById('stream-container'),
							l = document.getElementById('get-still'),
							m = document.getElementById('toggle-stream'),
							o = document.getElementById('close-stream'),
							p = () => {
									window.stop(), m.innerHTML = 'Start Stream'
							},
							q = () => {
									j.src = `${c}/mjpeg/1`, f(k), m.innerHTML = 'Stop Stream'
							};
					l.onclick = () => {
							p(), j.src = `${c}/jpg?_cb=${Date.now()}`, f(k)
					}, o.onclick = () => {
							p(), e(k)
					}, m.onclick = () => {
							const B = 'Stop Stream' === m.innerHTML;
							B ? p() : q()
					}, document.querySelectorAll('.default-action').forEach(B => {
							B.onchange = () => b(B)
					});
					const r = document.getElementById('agc'),
							s = document.getElementById('agc_gain-group'),
							t = document.getElementById('gainceiling-group');
					r.onchange = () => {
							b(r), r.checked ? (f(t), e(s)) : (e(t), f(s))
					};
					const u = document.getElementById('aec'),
							v = document.getElementById('aec_value-group');
					u.onchange = () => {
							b(u), u.checked ? e(v) : f(v)
					};
					const w = document.getElementById('awb_gain'),
							x = document.getElementById('wb_mode-group');
					w.onchange = () => {
							b(w), w.checked ? f(x) : e(x)
					};
					const A = document.getElementById('framesize');
					A.onchange = () => {
							b(A), 5 < A.value && (i(y, !1), i(z, !1))
					}
			});
		</script>
		</body>
</html>
)rawliteral";
//HTML-----------------------------------


// ===== rtos task handles =========================
// Streaming is implemented with 3 tasks:
TaskHandle_t tMjpeg;	 // handles client connections to the webserver
TaskHandle_t tCam;		 // handles getting picture frames from the camera and storing them locally
TaskHandle_t tStream;	// actually streaming frames to all connected clients

// frameSync semaphore is used to prevent streaming buffer as it is replaced with the next frame
SemaphoreHandle_t frameSync = NULL;

// Queue stores currently connected clients to whom we are streaming
QueueHandle_t streamingClients;

// We will try to achieve 25 FPS frame rate
const int FPS = 14;

// We will handle web client requests every 50 ms (20 Hz)
const int WSINTERVAL = 100;

// ======== Server Connection Handler Task ==========================
void mjpegCB(void* pvParameters) {
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(WSINTERVAL);

	// Creating frame synchronization semaphore and initializing it
	frameSync = xSemaphoreCreateBinary();
	xSemaphoreGive( frameSync );

	// Creating a queue to track all connected clients
	streamingClients = xQueueCreate( 10, sizeof(WiFiClient*) );

	//=== setup section	==================

	//	Creating RTOS task for grabbing frames from the camera
	xTaskCreatePinnedToCore(
		camCB,				// callback
		"cam",				// name
		4096,				// stacj size
		NULL,				// parameters
		2,					// priority
		&tCam,				// RTOS task handle
		APP_CPU);			// core

	//	Creating task to push the stream to all connected clients
	xTaskCreatePinnedToCore(
		streamCB,
		"strmCB",
		4 * 1024,
		NULL, //(void*) handler,
		2,
		&tStream,
		APP_CPU);

	//	Registering webserver handling routines
	server.on("/mjpeg/1", HTTP_GET, handleJPGSstream);
	server.on("/jpg", HTTP_GET, handleJPG);
	server.on("/get", HTTP_GET, get_handler);
	server.on("/set", HTTP_GET, set_handler);

	server.on("/control", HTTP_GET, control3_handler);
	server.on("/restart", HTTP_GET, restart_handler);
	server.on("/activatewebota", HTTP_GET, activatewebota_handler);
	server.on("/reset", HTTP_GET, reset_handler);
	server.onNotFound(handleNotFound);

	//	Starting webserver
	server.begin();

	//=== loop() section	===================
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		server.handleClient();

		//	After every server client handling request, we let other tasks run and then pause
		taskYIELD();
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}


// Commonly used variables:
volatile size_t camSize;		// size of the current frame, byte
volatile char* camBuf;			// pointer to the current frame


// ==== RTOS task to grab frames from the camera =========================
void camCB(void* pvParameters) {

	TickType_t xLastWakeTime;

	//	A running interval associated with currently desired frame rate
	const TickType_t xFrequency = pdMS_TO_TICKS(1000 / FPS);

	// Mutex for the critical section of swithing the active frames around
	portMUX_TYPE xSemaphore = portMUX_INITIALIZER_UNLOCKED;

	//	Pointers to the 2 frames, their respective sizes and index of the current frame
	char* fbs[2] = { NULL, NULL };
	size_t fSize[2] = { 0, 0 };
	int ifb = 0;

	//=== loop() section	===================
	xLastWakeTime = xTaskGetTickCount();

	for (;;) {

		//	Grab a frame from the camera and query its size
		cam.run();
		size_t s = cam.getSize();

		//	If frame size is more that we have previously allocated - request	125% of the current frame space
		if (s > fSize[ifb]) {
			fSize[ifb] = s * 4 / 3;
			fbs[ifb] = allocateMemory(fbs[ifb], fSize[ifb]);
		}

		//	Copy current frame into local buffer
		char* b = (char*) cam.getfb();
		memcpy(fbs[ifb], b, s);

		//	Let other tasks run and wait until the end of the current frame rate interval (if any time left)
		taskYIELD();
		vTaskDelayUntil(&xLastWakeTime, xFrequency);

		//	Only switch frames around if no frame is currently being streamed to a client
		//	Wait on a semaphore until client operation completes
		xSemaphoreTake( frameSync, portMAX_DELAY );

		//	Do not allow interrupts while switching the current frame
		portENTER_CRITICAL(&xSemaphore);
		camBuf = fbs[ifb];
		camSize = s;
		ifb++;
		ifb &= 1;	// this should produce 1, 0, 1, 0, 1 ... sequence
		portEXIT_CRITICAL(&xSemaphore);

		//	Let anyone waiting for a frame know that the frame is ready
		xSemaphoreGive( frameSync );

		//	Technically only needed once: let the streaming task know that we have at least one frame
		//	and it could start sending frames to the clients, if any
		xTaskNotifyGive( tStream );

		//	Immediately let other (streaming) tasks run
		taskYIELD();

		//	If streaming task has suspended itself (no active clients to stream to)
		//	there is no need to grab frames from the camera. We can save some juice
		//	by suspedning the tasks
		if ( eTaskGetState( tStream ) == eSuspended ) {
			vTaskSuspend(NULL);	// passing NULL means "suspend yourself"
		}
	}
}


// ==== Memory allocator that takes advantage of PSRAM if present =======================
char* allocateMemory(char* aPtr, size_t aSize) {

	//	Since current buffer is too smal, free it
	if (aPtr != NULL) free(aPtr);


	size_t freeHeap = ESP.getFreeHeap();
	char* ptr = NULL;

	// If memory requested is more than 2/3 of the currently free heap, try PSRAM immediately
	if ( aSize > freeHeap * 2 / 3 ) {
		if ( psramFound() && ESP.getFreePsram() > aSize ) {
			ptr = (char*) ps_malloc(aSize);
		}
	}
	else {
		//	Enough free heap - let's try allocating fast RAM as a buffer
		ptr = (char*) malloc(aSize);

		//	If allocation on the heap failed, let's give PSRAM one more chance:
		if ( ptr == NULL && psramFound() && ESP.getFreePsram() > aSize) {
			ptr = (char*) ps_malloc(aSize);
		}
	}

	// Finally, if the memory pointer is NULL, we were not able to allocate any memory, and that is a terminal condition.
	if (ptr == NULL) {
		ESP.restart();
	}
	return ptr;
}


// ==== STREAMING ======================================================
const char HEADER[] = "HTTP/1.1 200 OK\r\n" \
					  "Access-Control-Allow-Origin: *\r\n" \
					  "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";
const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";
const int hdrLen = strlen(HEADER);
const int bdrLen = strlen(BOUNDARY);
const int cntLen = strlen(CTNTTYPE);


// ==== Handle connection request from clients ===============================
void handleJPGSstream(void)
{
	//	Can only acommodate 10 clients. The limit is a default for WiFi connections
	if ( !uxQueueSpacesAvailable(streamingClients) ) return;


	//	Create a new WiFi Client object to keep track of this one
	WiFiClient* client = new WiFiClient();
	*client = server.client();

	//	Immediately send this client a header
	client->write(HEADER, hdrLen);
	client->write(BOUNDARY, bdrLen);

	// Push the client to the streaming queue
	xQueueSend(streamingClients, (void *) &client, 0);

	// Wake up streaming tasks, if they were previously suspended:
	if ( eTaskGetState( tCam ) == eSuspended ) vTaskResume( tCam );
	if ( eTaskGetState( tStream ) == eSuspended ) vTaskResume( tStream );
}


// ==== Actually stream content to all connected clients ========================
void streamCB(void * pvParameters) {
	char buf[16];
	TickType_t xLastWakeTime;
	TickType_t xFrequency;

	//	Wait until the first frame is captured and there is something to send
	//	to clients
	ulTaskNotifyTake( pdTRUE,					/* Clear the notification value before exiting. */
										portMAX_DELAY ); /* Block indefinitely. */

	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		// Default assumption we are running according to the FPS
		xFrequency = pdMS_TO_TICKS(1000 / FPS);

		//	Only bother to send anything if there is someone watching
		UBaseType_t activeClients = uxQueueMessagesWaiting(streamingClients);
		if ( activeClients ) {
			// Adjust the period to the number of connected clients
			xFrequency /= activeClients;

			//	Since we are sending the same frame to everyone,
			//	pop a client from the the front of the queue
			WiFiClient *client;
			xQueueReceive (streamingClients, (void*) &client, 0);

			//	Check if this client is still connected.

			if (!client->connected()) {
				//	delete this client reference if s/he has disconnected
				//	and don't put it back on the queue anymore. Bye!
				delete client;
			}
			else {

				//	Ok. This is an actively connected client.
				//	Let's grab a semaphore to prevent frame changes while we
				//	are serving this frame
				xSemaphoreTake( frameSync, portMAX_DELAY );

				client->write(CTNTTYPE, cntLen);
				sprintf(buf, "%d\r\n\r\n", camSize);
				client->write(buf, strlen(buf));
				client->write((char*) camBuf, (size_t)camSize);
				client->write(BOUNDARY, bdrLen);

				// Since this client is still connected, push it to the end
				// of the queue for further processing
				xQueueSend(streamingClients, (void *) &client, 0);

				//	The frame has been served. Release the semaphore and let other tasks run.
				//	If there is a frame switch ready, it will happen now in between frames
				xSemaphoreGive( frameSync );
				taskYIELD();
			}
		}
		else {
			//	Since there are no connected clients, there is no reason to waste battery running
			vTaskSuspend(NULL);
		}
		//	Let other tasks run after serving every client
		taskYIELD();
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

const char JHEADER[] =  "HTTP/1.1 200 OK\r\n" \
						"Content-disposition: inline; filename=capture.jpg\r\n" \
						"Content-type: image/jpeg\r\n\r\n";
const int jhdLen = strlen(JHEADER);

// ==== Serve up one JPEG frame =============================================
void handleJPG(void)
{
	WiFiClient client = server.client();

	if (!client.connected()) return;
	cam.run();
	client.write(JHEADER, jhdLen);
	client.write((char*)cam.getfb(), cam.getSize());
}


// ==== Handle invalid URL requests ============================================
void handleNotFound(){
	String message = "Server is running!\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\nVersion: ";
	message += "2";
	message += "\n";
	message += "\nCounter: ";
	message += counter;
	message += "\n";
	server.send(200, "text / plain", message);
}



// ==== SETUP method ==================================================================
void setup(){
	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // prevent brownouts by silencing them
	
	// Setup Serial connection:
	Serial.begin(115200);
	delay(1000); // wait for a second to let Serial connect

	preferences.begin("CameraSettings", false);
	counter = preferences.getInt("counter", 0);
	counter++;
	preferences.putInt("counter", counter);
	
	
	// Configure the camera
	camera_config_t config;
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer = LEDC_TIMER_0;
	config.pin_d0 = Y2_GPIO_NUM;
	config.pin_d1 = Y3_GPIO_NUM;
	config.pin_d2 = Y4_GPIO_NUM;
	config.pin_d3 = Y5_GPIO_NUM;
	config.pin_d4 = Y6_GPIO_NUM;
	config.pin_d5 = Y7_GPIO_NUM;
	config.pin_d6 = Y8_GPIO_NUM;
	config.pin_d7 = Y9_GPIO_NUM;
	config.pin_xclk = XCLK_GPIO_NUM;
	config.pin_pclk = PCLK_GPIO_NUM;
	config.pin_vsync = VSYNC_GPIO_NUM;
	config.pin_href = HREF_GPIO_NUM;
	config.pin_sscb_sda = SIOD_GPIO_NUM;
	config.pin_sscb_scl = SIOC_GPIO_NUM;
	config.pin_pwdn = PWDN_GPIO_NUM;
	config.pin_reset = RESET_GPIO_NUM;
	config.xclk_freq_hz = 20000000;
	config.pixel_format = PIXFORMAT_JPEG;
	config.frame_size = FRAMESIZE_UXGA;
	config.jpeg_quality = 0;
	config.fb_count = 2;
	

	#if defined(CAMERA_MODEL_ESP_EYE)
		pinMode(13, INPUT_PULLUP);
		pinMode(14, INPUT_PULLUP);
	#endif

	if (cam.init(config) != ESP_OK) {
		Serial.println("Error initializing the camera");
		delay(10000);
		ESP.restart();
	}

	sensor_t * s = esp_camera_sensor_get();
	
	s->set_framesize(s, (framesize_t)preferences.getInt("framesize", s->status.framesize));
	s->set_quality(s, 63 - preferences.getInt("quality", s->status.quality));
	s->set_contrast(s, preferences.getInt("contrast", s->status.framesize));
	s->set_brightness(s, preferences.getInt("brightness", s->status.brightness));
	s->set_saturation(s, preferences.getInt("saturation", s->status.saturation));
	s->set_gainceiling(s, (gainceiling_t)preferences.getInt("gainceiling", s->status.gainceiling));
	s->set_colorbar(s, preferences.getInt("colorbar", s->status.colorbar));
	s->set_whitebal(s, preferences.getInt("awb", s->status.awb));
	s->set_gain_ctrl(s, preferences.getInt("agc", s->status.agc));
	s->set_exposure_ctrl(s, preferences.getInt("aec", s->status.aec));
	s->set_hmirror(s, preferences.getInt("hmirror", s->status.hmirror));
	s->set_vflip(s, preferences.getInt("vflip", s->status.vflip));
	s->set_awb_gain(s, preferences.getInt("awb_gain", s->status.awb_gain));
	s->set_agc_gain(s, preferences.getInt("agc_gain", s->status.agc_gain));
	s->set_aec_value(s, preferences.getInt("aec_value", s->status.aec_value));
	s->set_aec2(s, preferences.getInt("aec2", s->status.aec2));
	s->set_dcw(s, preferences.getInt("dcw", s->status.dcw));
	s->set_bpc(s, preferences.getInt("bpc", s->status.bpc));
	s->set_wpc(s, preferences.getInt("wpc", s->status.wpc));
	s->set_raw_gma(s, preferences.getInt("raw_gma", s->status.raw_gma));
	s->set_lenc(s, preferences.getInt("lenc", s->status.lenc));
	s->set_special_effect(s, preferences.getInt("special_effect", s->status.special_effect));
	s->set_wb_mode(s, preferences.getInt("wb_mode", s->status.wb_mode));
	s->set_ae_level(s, preferences.getInt("ae_level", s->status.ae_level));

	preferences.end();

	ledcSetup(7, 5000, 8);
	ledcAttachPin(4, 7);	//pin4 is LED
	
	//	Configure and connect to WiFi
	IPAddress ip;

	WiFi.mode(WIFI_STA);
	WiFi.begin(SSID1, PWD1);
	WiFi.setSleep(false);
	Serial.print("Connecting to WiFi");
	while (WiFi.status() != WL_CONNECTED){
		delay(500);
		Serial.print(F("."));
	}
	ip = WiFi.localIP();
	Serial.println(F("WiFi connected"));
	Serial.println("");
	Serial.print("Stream Link: http://");
	Serial.print(ip);
	Serial.println("/mjpeg/1");

	
	for (int i=0;i<5;i++) {
		ledcWrite(7,10);	// flash led
		delay(50);
		ledcWrite(7,0);
		delay(50);		
	} 
				

	// Start mainstreaming RTOS task
	xTaskCreatePinnedToCore(
		mjpegCB,
		"mjpeg",
		4 * 1024,
		NULL,
		2,
		&tMjpeg,
		APP_CPU);
}

bool flag = false;

void loop() {
	if(flag) {
		webota.delay(1000);
		webota.handle();
	} else
		vTaskDelay(1000);
}

void get_handler(){
	sensor_t * s = esp_camera_sensor_get();
	StaticJsonDocument<400> data;
	
	preferences.begin("CameraSettings", false);
	
	int framesize = preferences.getInt("framesize", 999);
	data["framesize"] = (framesize == 999) ? s->status.framesize : framesize;
	int quality = preferences.getInt("quality", 999);
	data["quality"] = (quality == 999) ? 63 - s->status.quality : quality;
	int contrast = preferences.getInt("contrast", 999);
	data["contrast"] = (contrast == 999) ? s->status.contrast : contrast;
	int brightness = preferences.getInt("brightness", 999);
	data["brightness"] = (brightness == 999) ? s->status.brightness : brightness;
	int saturation = preferences.getInt("saturation", 999);
	data["saturation"] = (saturation == 999) ? s->status.saturation : saturation;
	int gainceiling = preferences.getInt("gainceiling", 999);
	data["gainceiling"] = (gainceiling == 999) ? s->status.gainceiling : gainceiling;
	int colorbar = preferences.getInt("colorbar", 999);
	data["colorbar"] = (colorbar == 999 ) ? s->status.colorbar : colorbar;
	int awb = preferences.getInt("awb", 999);
	data["awb"] = (awb == 999) ? s->status.awb : awb;
	int agc = preferences.getInt("agc", 999);
	data["agc"] = (agc == 999) ? s->status.agc : agc;
	int aec = preferences.getInt("aec", 999);
	data["aec"] = (aec == 999) ? s->status.aec : aec;
	int hmirror = preferences.getInt("hmirror", 999);
	data["hmirror"] = (hmirror == 999) ? s->status.hmirror : hmirror;
	int vflip = preferences.getInt("vflip", 999);
	data["vflip"] = (vflip == 999) ? s->status.vflip : vflip;
	int awb_gain = preferences.getInt("awb_gain", 999);
	data["awb_gain"] = (awb_gain == 999) ? s->status.awb_gain : awb_gain;
	int agc_gain = preferences.getInt("agc_gain", 999);
	data["agc_gain"] = (agc_gain == 999) ? s->status.agc_gain : agc_gain;
	int aec_value = preferences.getInt("aec_value", 999);
	data["aec_value"] = (aec_value == 999) ? s->status.aec_value : aec_value;
	int aec2 = preferences.getInt("aec2", 999);
	data["aec2"] = (aec2 == 999) ? s->status.aec2 : aec2;
	int dcw = preferences.getInt("dcw", 999);
	data["dcw"] = (dcw == 999) ? s->status.dcw : dcw;
	int bpc = preferences.getInt("bpc", 999);
	data["bpc"] = (bpc == 999) ? s->status.bpc : bpc;
	int wpc = preferences.getInt("wpc", 999);
	data["wpc"] = (wpc == 999) ? s->status.wpc : wpc;
	int raw_gma = preferences.getInt("raw_gma", 999);
	data["raw_gma"] = (raw_gma == 999) ? s->status.raw_gma : raw_gma;
	int lenc = preferences.getInt("lenc", 999);
	data["lenc"] = (lenc == 999) ? s->status.lenc : lenc;
	int special_effect = preferences.getInt("special_effect", 999);
	data["special_effect"] = (special_effect == 999) ? s->status.special_effect : special_effect;
	int wb_mode = preferences.getInt("wb_mode", 999);
	data["wb_mode"] = (wb_mode == 999) ? s->status.wb_mode : wb_mode;
	int ae_level = preferences.getInt("ae_level", 999);
	data["ae_level"] = (ae_level == 999) ? s->status.ae_level : ae_level;
	
	preferences.end();
	
	String response;
	serializeJson(data, response);
	server.send(200, "application/json", response);
}

void set_handler(){
	String variable = server.arg("var");
	String value = server.arg("val");
 	
		int val = value.toInt();
		
		sensor_t * s = esp_camera_sensor_get();
		
		preferences.begin("CameraSettings", false);
		
		if(variable == "framesize") {
				if(s->pixformat == PIXFORMAT_JPEG) 
					s->set_framesize(s, (framesize_t)val);
					preferences.putInt("framesize", val);
		}
		else if(variable == "quality"){
			s->set_quality(s, 63 - val);
			preferences.putInt("quality", val);
		} else if(variable == "contrast"){
			s->set_contrast(s, val);
			preferences.putInt("contrast", val);
		} else if(variable == "brightness"){
			s->set_brightness(s, val);
			preferences.putInt("brightness", val);
		} else if(variable == "saturation"){
			s->set_saturation(s, val);
			preferences.putInt("saturation", val);
		} else if(variable == "gainceiling"){
			s->set_gainceiling(s, (gainceiling_t)val);
			preferences.putInt("gainceiling", val);
		} else if(variable == "colorbar"){
			s->set_colorbar(s, val);
			preferences.putInt("colorbar", val);
		} else if(variable == "awb"){
			s->set_whitebal(s, val);
			preferences.putInt("awb", val);
		} else if(variable == "agc"){
			s->set_gain_ctrl(s, val);
			preferences.putInt("agc", val);
		} else if(variable == "aec"){
			s->set_exposure_ctrl(s, val);
			preferences.putInt("aec", val);
		} else if(variable == "hmirror"){
			s->set_hmirror(s, val);
			preferences.putInt("hmirror", val);
		} else if(variable == "vflip"){
			s->set_vflip(s, val);
			preferences.putInt("vflip", val);
		} else if(variable == "awb_gain"){
			s->set_awb_gain(s, val);
			preferences.putInt("awb_gain", val);
		} else if(variable == "agc_gain"){
			s->set_agc_gain(s, val);
			preferences.putInt("agc_gain", val);
		} else if(variable == "aec_value"){
			s->set_aec_value(s, val);
			preferences.putInt("aec_value", val);
		} else if(variable == "aec2"){
			s->set_aec2(s, val);
			preferences.putInt("aec2", val);
		} else if(variable == "dcw"){
			s->set_dcw(s, val);
			preferences.putInt("dcw", val);
		} else if(variable == "bpc"){
			s->set_bpc(s, val);
			preferences.putInt("bpc", val);
		} else if(variable == "wpc"){
			s->set_wpc(s, val);
			preferences.putInt("wpc", val);
		} else if(variable == "raw_gma"){
			s->set_raw_gma(s, val);
			preferences.putInt("raw_gma", val);
		} else if(variable == "lenc"){
			s->set_lenc(s, val);
			preferences.putInt("lenc", val);
		} else if(variable == "special_effect"){
			s->set_special_effect(s, val);
			preferences.putInt("special_effect", val);
		} else if(variable == "wb_mode"){
			s->set_wb_mode(s, val);
			preferences.putInt("wb_mode", val);
		} else if(variable == "ae_level"){
			s->set_ae_level(s, val);
			preferences.putInt("ae_level", val);
		}
		
		preferences.end();
		
		server.send(200, "text/plain", ("OK"));
}

void control3_handler(){
	server.send(200, "text/html", (const char *)INDEX_HTML);
}

void restart_handler(){
	server.send(200, "text/plain", ("OK"));
	delay(500);
	ESP.restart();
}

void activatewebota_handler(){
	flag = true;
	server.send(200, "text/plain", ("OK"));
}

void reset_handler(){
	preferences.begin("CameraSettings", false);
	preferences.clear();
	preferences.end();
	server.send(200, "text/plain", ("OK"));
	server.send(200, "text/plain", ("OK"));
	delay(500);
	ESP.restart();
}