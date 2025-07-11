app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.msg_t = {
      augentix: ['Augentix', '多方科技', '多方科技'],
      noTxt: ['', '', ''],
      TimeStart: ['Day Mode Start', '日模式開始', '日模式开始'],
      TimeEnd: ['Day Mode End', '日模式結束', '日模式结束'],
      Auto: ['Auto', '自動', '自动'],
      Auto_Time_Switch: ['Auto Time Switch', '定時切換', '定时切换'],
      Read_setting: ['Read out from device', '讀取設定', '读取设定'],
      Apply: ['Apply', '應用', '应用'],
      Reboot: ['Reboot', '重新開機', '重新开机'],
      Loading: ['Loading...', '讀取中...', '读取中...'],
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      NoRespond: ['Device no respond!!', '設備無反應!!', '设备无反应!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
      Image_Preference: ['Image Preference', '圖像設置', '图像设置'],
      AWB_Preference: ['AWB Preference', '白平衡設置', '白平衡设置'],
      Stitching: ['Stitching', '畫面縫合', '画面缝合'],
      Advanced_Setting: ['Advanced Setting', '高級設置', '高级设置'],
      Image_Mode: ['Image Mode', '圖像模式', '图像模式'],
      Color: ['Color', '彩色', '彩色'],
      Gray_Scale: ['Gray Scale', '黑白', '黑白'],
      Night_Mode: ['Night Mode', '夜晚模式', '夜晚模式'],
      Day_Mode: ['Day Mode', '白天模式', '白天模式'],
      External_Mode: ['External Mode', '外部模式', '外部模式'],
      //'Auto_Mode': ['Auto Mode', '自动模式', '自動模式'],
      IR_Cut_filter_mode: [
        'IR Cut filter mode',
        '紅外截止濾鏡',
        '红外截止滤镜',
      ],
      IR_Led_mode: ['IR Led mode', '紅外燈模式', '紅外燈模式'],
      Backlight_Compensation: [
        'Backlight Compensation',
        '背光補償',
        '背光补偿',
      ],
      Sync_Night_Mode: [
        'Sync with night mode',
        '與當前夜間模式同步',
        '与当前夜间模式同步',
      ],
      WDR_Enable: ['WDR Enable', '開啓寬動態', '开启宽动态'],
      WDR_Strength: ['WDR Strength', '寬動態強度', '宽动态强度'],
    };

    var vlc;
    var itemId;

    $scope.timeSwitch = {
      TimeSwitch_enabled: false,
      time_start: '06:30',
      time_end: '18:30',
    };

    $scope.timeNow = {
      time_now_hr: 0,
      time_now_min: 0,
    };

    $scope.cameraAdv = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_ADV_IMG_PREF,
      cmd_type: 'set',
      backlight_compensation: 0,
      night_mode: 2, //"AUTO"
      icr_mode: 2, //"AUTO"
      ir_led_mode: 2, //"AUTO"
      image_mode: 2, //"AUTO"
      wdr_en: 0,
      wdr_strength: 0,
    };
    $scope.Yxxx = 333;
    $scope.cmd = 'cmd';
    $scope.Result = 'result';
    $scope.WDRoff = function () {
      return $scope.cameraAdv.wdr_en == 0;
    };
    $scope.Option = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_SYS_FEATURE_OPTION,
      cmd_type: 'reply',
      rval: 0,
      stitch_support: 0,
    };
    //$scope.Option.rval = 1
    $scope.Disrval = function () {
      if ($scope.Option.stitch_support == 1) return false;
      else return true;
    };

    $scope.dayMode = function () {
      if ($scope.cameraAdv.night_mode == 'OFF') {
        document.getElementById('daymode').checked = true;
        document.getElementById('imcolor').checked = true;
        document.getElementById('imgray').checked = false;
        document.getElementById('imauto').checked = false;
        document.getElementById('lmoff').checked = true;
        document.getElementById('lmon').checked = false;
        document.getElementById('lmauto').checked = false;
        $scope.cameraAdv.icr_mode = 'ON';
        $scope.cameraAdv.image_mode = 'COLOR';
        $scope.cameraAdv.ir_led_mode = 'OFF';
      }
    };
    $scope.nightMode = function () {
      if ($scope.cameraAdv.night_mode == 'ON') {
        document.getElementById('nightmode').checked = true;
        document.getElementById('imcolor').checked = false;
        document.getElementById('imgray').checked = true;
        document.getElementById('imauto').checked = false;
        document.getElementById('lmoff').checked = false;
        document.getElementById('lmon').checked = true;
        document.getElementById('lmauto').checked = false;
        $scope.cameraAdv.icr_mode = 'OFF';
        $scope.cameraAdv.image_mode = 'GRAYSCALE';
        $scope.cameraAdv.ir_led_mode = 'ON';
      }
    };

    $scope.autoMode = function () {
      if ($scope.cameraAdv.night_mode == 'AUTO') {
        document.getElementById('automode').checked = true;
        document.getElementById('imcolor').checked = false;
        document.getElementById('imgray').checked = false;
        document.getElementById('imauto').checked = true;
        document.getElementById('lmoff').checked = false;
        document.getElementById('lmon').checked = false;
        document.getElementById('lmauto').checked = true;
        $scope.cameraAdv.icr_mode = 'AUTO';
        $scope.cameraAdv.image_mode = 'AUTO';
        $scope.cameraAdv.ir_led_mode = 'AUTO';
      }
    };

    $scope.timeMode = function () {
      if ($scope.cameraAdv.night_mode == 'AUTOSWITCH') {
        document.getElementById('timemode').checked = true;
        document.getElementById('imcolor').checked = false;
        document.getElementById('imgray').checked = true;
        document.getElementById('imauto').checked = false;
        document.getElementById('lmoff').checked = false;
        document.getElementById('lmon').checked = true;
        document.getElementById('lmauto').checked = false;
        $scope.cameraAdv.icr_mode = 'OFF';
        $scope.cameraAdv.image_mode = 'OFF';
        $scope.cameraAdv.ir_led_mode = 'ON';
      }
    };
    
    $scope.sendData = function () {
      $scope.cameraAdv.cmd_type = 'set';
      $scope.cameraAdv.backlight_compensation = Number(
        $scope.cameraAdv.backlight_compensation
      );
      $scope.cameraAdv.wdr_en = Number($scope.cameraAdv.wdr_en);
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.cameraAdv.backlight_compensation = $scope.cameraAdv.backlight_compensation.toString();
      $scope.cameraAdv.wdr_en = $scope.cameraAdv.wdr_en.toString();

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.cameraAdv),
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Result = resp;
          if (resp.data.rval == 0) {
            if ($scope.timeSwitch.time_start == $scope.timeSwitch.time_end) {
              $scope.cmdStatus =
                'File: Day Mode Start should not equal to Day Mode End!!';
              return;
            }
            $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          } else {
            $scope.cmdStatus =
              $scope.msg_t.WriteFail[$scope.lang] + ' rval=' + resp.data.rval;
          }

          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          //	$scope.Result = "Post Error";
        }
      );

      //Set Time Switch Config
      if ($scope.cameraAdv.night_mode == 'AUTOSWITCH') {
        $scope.cmd = '/TimeSwitchSet.cgi';
        console.log('Assign Time Switch config');
        $scope.timeSwitch.time_start = document.getElementById(
          'timeStart'
        ).value;
        $scope.timeSwitch.time_end = document.getElementById('timeEnd').value;
        if ($scope.timeSwitch.time_start == $scope.timeSwitch.time_end) {
          $scope.cmdStatus =
            'File: Day Mode Start should not equal to Day Mode End!!';
          return;
        }
        var Now = new Date();
        $scope.timeNow.time_now_hr = Now.getHours();
        $scope.timeNow.time_now_min = Now.getMinutes();
        if ($scope.cameraAdv.night_mode == 'AUTOSWITCH') {
          $scope.timeSwitch.TimeSwitch_enabled = true;
        } else {
          $scope.timeSwitch.TimeSwitch_enabled = false;
        }
        console.log(JSON.stringify($scope.timeSwitch));
        console.log(JSON.stringify($scope.timeNow));

        $http({
          method: 'post',
          url: $scope.cmd,
          headers: {
            'Content-Type': 'application/json',
          },
          data: JSON.stringify($.extend({}, $scope.timeSwitch, $scope.timeNow)),
        }).then(
          function (resp) {
            console.log('post success');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          },
          function (resp) {
            console.log('post error');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
            //$scope.Result = "Post Error";
          }
        );
      }
    };

    $scope.getData = function (hide_msg) {
      $scope.canApply = false;
      $scope.cmdStatus = $scope.msg_t.Loading[$scope.lang];
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('get data');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify({
          "master_id": 1,
          "cmd_id": Cmd.AGTX_CMD_ADV_IMG_PREF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_ADV_IMG_PREF');
          $scope.cameraAdv = resp.data;
          $scope.cameraAdv.backlight_compensation = $scope.cameraAdv.backlight_compensation.toString();
          $scope.cameraAdv.wdr_en = $scope.cameraAdv.wdr_en.toString();
          $scope.Result = resp;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          $scope.nightMode();
          $scope.dayMode();
          $scope.autoMode();
          $scope.timeMode();
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_ADV_IMG_PREF');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );

      //Get Time Switch Config
      $scope.cmd = '/getTimeSwitch.cgi';
      console.log('getTimeSwitch.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.timeSwitch = resp.data;
          $scope.canApply = true;
          if ($scope.timeSwitch.TimeSwitch_enabled == 1) {
            $scope.timeSwitch.TimeSwitch_enabled = true;
          }
          if (
            $scope.timeSwitch.TimeSwitch_enabled.length == 0 &&
            $scope.timeSwitch.time_start.length == 0 &&
            $scope.timeSwitch.time_end.length == 0
          ) {
            $scope.cmdStatus = 'File: TimeSwitch.conf empty!!';
            return;
          }
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
          document.getElementById('timeStart').value =
            $scope.timeSwitch.time_start;
          document.getElementById('timeEnd').value = $scope.timeSwitch.time_end;
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getOption = function () {
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('Get option name');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify({
          "master_id": 1,
          "cmd_id": Cmd.AGTX_CMD_SYS_FEATURE_OPTION,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post success: ' + 'AGTX_CMD_SYS_FEATURE_OPTION');
          $scope.Option = resp.data;
          $scope.Option.stitch_support = Number(resp.data.stitch_support);
          $scope.Disrval();
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_SYS_FEATURE_OPTION');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getOption();
    $scope.getData();
    window.langId = $scope.lang;
  },
]);
