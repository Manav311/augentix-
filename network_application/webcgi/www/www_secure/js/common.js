var app = angular.module('HC_App', []);

app.config([
  '$httpProvider',
  function ($httpProvider) {
    //initialize get if not there
    if (!$httpProvider.defaults.headers.get) {
      $httpProvider.defaults.headers.get = {};
    }
    // Answer edited to include suggestions from comments
    // because previous version of code introduced browser-related errors

    //disable IE ajax request caching
    $httpProvider.defaults.headers.get['If-Modified-Since'] =
      'Mon, 26 Jul 1997 05:00:00 GMT';
    // extra
    $httpProvider.defaults.headers.get['Cache-Control'] = 'no-cache';
    $httpProvider.defaults.headers.get['Pragma'] = 'no-cache';
  },
]);
app.service('fileUpload', [
  '$http',
  function ($http) {
    this.uploadFileToUrl = function (file, uploadUrl) {
      var fd = new FormData();
      fd.append('data', file);
      console.log(fd);
      $http
        .post(uploadUrl, fd, {
          transformRequest: angular.identity,
          headers: {
            'Content-Type': undefined,
          },
        })
        .then(
          function (resp) {},
          function (resp) {}
        );
    };
  },
]);
app.config([
  '$locationProvider',
  function ($locationProvider) {
    // $locationProvider.html5Mode(true);
    $locationProvider.html5Mode({
      enabled: true,
      requireBase: false,
    });
  },
]);

// FLV stream sources
const { protocol, hostname } = window.location;
const flvPort = (protocol === 'https:') ? 443 : 80;
window.flvOrigin = `${protocol}//${hostname}:${flvPort}`;
window.flvLive0 = `${window.flvOrigin}/live/0`;
window.flvLive1 = `${window.flvOrigin}/live/1`;
window.flvLive2 = `${window.flvOrigin}/live/2`;
window.flvLiveAudio0 = `${window.flvOrigin}/liveaudio/0`;
window.flvLiveAudio1 = `${window.flvOrigin}/liveaudio/1`;
window.flvLiveAudio2 = `${window.flvOrigin}/liveaudio/2`;
// Stream source from relay server
window.flvRelayLiveAudio0 = 'http://localhost:8000/live/0.flv';
window.flvRelayLiveAudio1 = 'http://localhost:8000/live/1.flv';

// Relay server API
window.relayAPIOrigin = 'http://localhost:5236';

// WebSocket
if (protocol === 'https:') {
  const wsProtocol = 'wss:';
  const wsPort = 7681;
  window.websocketURL = `${wsProtocol}//${hostname}:${wsPort}`;
}

var Iva_Index = {
  td: 0,
  number: 1,
};

var Cmd = {
  AGTX_CMD_SYS_INFO: 1048579,
  AGTX_CMD_SYS_FEATURE_OPTION: 1048580,
  AGTX_CMD_PRODUCT_OPTION_LIST: 1048581,
  AGTX_CMD_SYS_DB_INFO: 1048583,
  AGTX_CMD_VIDEO_DEV_CONF: 3145730,
  AGTX_CMD_VIDEO_STRM_CONF: 3145731,
  AGTX_CMD_STITCH_CONF: 3145732,
  AGTX_CMD_AWB_PREF: 3145733,
  AGTX_CMD_IMG_PREF: 3145734,
  AGTX_CMD_ADV_IMG_PREF: 3145735,
  AGTX_CMD_DIP_CAL: 3145736,
  AGTX_CMD_DIP_DBC: 3145737,
  AGTX_CMD_DIP_DCC: 3145738,
  AGTX_CMD_DIP_LSC: 3145739,
  AGTX_CMD_DIP_CTRL: 3145740,
  AGTX_CMD_DIP_AE: 3145741,
  AGTX_CMD_DIP_AWB: 3145742,
  AGTX_CMD_DIP_PTA: 3145743,
  AGTX_CMD_DIP_CSM: 3145744,
  AGTX_CMD_DIP_SHP: 3145745,
  AGTX_CMD_DIP_NR: 3145746,
  AGTX_CMD_DIP_ROI: 3145747,
  AGTX_CMD_DIP_TE: 3145748,
  AGTX_CMD_DIP_GAMMA: 3145749,
  AGTX_CMD_DIP_ISO: 3145750,
  AGTX_CMD_COLOR_CONF: 3145751,
  AGTX_CMD_PRODUCT_OPTION: 3145752,
  AGTX_CMD_RES_OPTION: 3145753,
  AGTX_CMD_VENC_OPTION: 3145754,
  AGTX_CMD_LDC_CONF: 3145755,
  AGTX_CMD_VIDEO_LAYOUT_CONF: 3145756,
  AGTX_CMD_PANORAMA_CONF: 3145757,
  AGTX_CMD_PANNING_CONF: 3145758,
  AGTX_CMD_SURROUND_CONF: 3145759,
  AGTX_CMD_ANTI_FLICKER_CONF: 3145760,
  AGTX_CMD_DIP_SHP_WIN: 3145761,
  AGTX_CMD_DIP_NR_WIN: 3145762,
  AGTX_CMD_PRIVATE_MODE_CONF: 3145763,
  AGTX_CMD_AUDIO_CONF: 4194305,
  AGTX_CMD_VOICE_CONF: 4194306,
  AGTX_CMD_SIREN_CONF: 4194307,
  AGTX_CMD_EVT_CONF: 5242881,
  AGTX_CMD_GPIO_CONF: 5242882,
  AGTX_CMD_EVT_PARAM: 5242883,
  AGTX_CMD_LOCAL_RECORD_CONF: 5242884,
  AGTX_CMD_PWM_CONF: 5242885,
  AGTX_CMD_PIR_CONF: 5242886,
  AGTX_CMD_FLOODLIGHT_CONF: 5242887,
  AGTX_CMD_LIGHT_SENSOR_CONF: 5242888,
  AGTX_CMD_OSD_CONF: 6291457,
  AGTX_CMD_OSD_PM_CONF: 6291458,
  AGTX_CMD_TD_CONF: 7340033,
  AGTX_CMD_MD_CONF: 7340034,
  AGTX_CMD_AROI_CONF: 7340035,
  AGTX_CMD_PD_CONF: 7340036,
  AGTX_CMD_OD_CONF: 7340037,
  AGTX_CMD_RMS_CONF: 7340038,
  AGTX_CMD_LD_CONF: 7340039,
  AGTX_CMD_EF_CONF: 7340040,
  AGTX_CMD_VDBG_CONF: 7340041,
  AGTX_CMD_VIDEO_PTZ_CONF: 7340042,
  AGTX_CMD_SHD_CONF: 7340043,
  AGTX_CMD_EAIF_CONF: 7340044,
  AGTX_CMD_PFM_CONF: 7340045,
  AGTX_CMD_BM_CONF: 7340046,
  AGTX_CMD_DK_CONF: 7340047,
  AGTX_CMD_FLD_CONF: 7340048,
  AGTX_CMD_LSD_CONF: 8388609,
};

function setIvaPage(idx) {
  /* IvaMenu in message.js */
  var i;
  for (i = 0; i < Iva_Index.number; i++) {
    if (i == idx) {
      IvaMenu[i].page_item = 'page-item active';
    }
  }
  return IvaMenu;
}

function setVideoPage(idx) {
  /*  in message.js */
  var i;
  for (i = 0; i < Iva_Index.number; i++) {
    if (i == idx) {
      VideoMenu[i].page_item = 'page-item active';
    }
  }
  return VideoMenu;
}

function setIvaDropdown(idx) {
  /* IvaMenu in message.js */
  var i;
  for (i = 0; i < Iva_Index.number; i++) {
    if (i == idx) {
      IvaDropDown[i].dropdown = 'dropdown-item active';
    }
  }
  return IvaDropDown;
}

function setAudioPage(idx) {
  /* AudioMenu in message.js */
  var i;
  for (i = 0; i < Audio_Index.number; i++) {
    if (i == idx) {
      AudioMenu[i].page_item = 'page-item active';
    }
  }
  return AudioMenu;
}

function setAudioDropdown(idx) {
  /* AudioMenu in message.js */
  var i;
  for (i = 0; i < Audio_Index.number; i++) {
    if (i == idx) {
      AudioDropDown[i].dropdown = 'dropdown-item active';
    }
  }
  return AudioDropDown;
}

var Video_Index = {
  video: 0,
  //'video_layout':1,
  ptz: 1,
  eis: 2,
  number: 3,
};

var Audio_Index = {
  lsd: 0,
  number: 1,
};

function setPage(idx, menu_idx, menu) {
  /* IvaMenu in message.js */
  var i;
  for (i = 0; i < menu_idx.number; i++) {
    if (i == idx) {
      menu[i].page_item = 'page-item active';
    }
  }
  return menu;
}

function getLangFromUrlPath(location) {
  var urlpath = location.path();
  var lang = 0;
  if (urlpath.substr(1, 7) == 'html_en') {
    lang = 0;
  }
  if (urlpath.substr(1, 7) == 'html_tw') {
    lang = 1;
  }
  if (urlpath.substr(1, 7) == 'html_cn') {
    lang = 2;
  }
  return lang;
}

function MAX(a, b) {
  if (a > b) {
    return a;
  }
  return b;
}

function MIN(a, b) {
  if (a < b) {
    return a;
  }
  return b;
}

/* unused 
function sendIvaData(ivaPref, http) {
	var Result = "result";
	var cmdStatus = "";
	ivaPref.cmd_type = "set";
	if (ivaPref.enabled)
		ivaPref.enabled = 1;
	else
		ivaPref.enabled = 0;
	var cmd = "/cgi-bin/msg.cgi";
	ivaPref.enabled = (ivaPref.enabled == 1);
	console.log('enter submitForm');
	http({
		method: 'post',
		url: cmd,
		headers: {
			'Content-Type': 'application/json',
		},
		data: JSON.stringify(ivaPref, function(key, value) {	if (key === "$$hashKey") {return undefined;} return value;}),
	}).then(function(resp) {
		console.log('post success');
		Result = resp;
		if (resp.data.rval == 0) {
			cmdStatus = "Setting Done!!";
		} else {
			cmdStatus = "Setting Error,rval=" + resp.data.rval;
		}
		console.log(resp);
	}, function(resp) {
		console.log('post error');
		console.log(resp);
		//	$scope.Result = "Post Error";
	});

	
	return [cmdStatus, Result];
};

function getIvaData(ivaPref, http) {
	var cmd = "/cgi-bin/msg.cgi";
	var cmdStatus = "";
	var Result = "";
	console.log('get data');
	http({
		method: 'post',
		url: cmd,
		headers: {
			'Content-Type': 'application/json',
		},
		data: JSON.stringify({
			"master_id": 1,
			"cmd_id": ivaPref.cmd_id,
			"cmd_type": "get",
		}),
	}).then(function(resp) {
			console.log('post getdata success');
			ivaPref = resp.data;
			ivaPref.enabled = (ivaPref.enabled == 1);
			Result = resp;
			cmdStatus = "";
			console.log(resp);
		},
		function(resp) {
			console.log('post error');
			console.log(resp);
			cmdStatus = "Get setting error!";
			//$scope.Result = "Post Error";
		});
	return [cmdStatus, Result];
};

*/
