app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.VideoMenu = setPage(Video_Index.ptz, Video_Index, VideoMenu);
    $scope.msg_t = {
      augentix: ['Augentix', '多方科技', '多方科技'],
      noTxt: ['', '', ''],
      noTxtTab: [' ', ' ', ' '],
      Left: ['<', '<', '<'],
      Auto: ['Auto', '自動', '自动'],
      Read_setting: ['Read out from device', '讀取設定', '读取设定'],
      Apply: ['Apply', '應用', '应W用'],
      Reboot: ['Reboot', '重新開機', '重新开机'],
      Switch2SysupdOS: ['System Update', '系統更新', '系统更新'],
      Loading: ['Loading...', '讀取中...', '读取中...'],
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      NoRespond: ['Device no respond!!', '設備無反應!!', '设备无反应!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
      Sensitivity: ['Sensitivity:', '敏感度:', '敏感度:'],
      Start: ['Start', '起始', '起始'],
      End: ['End', '結束', '结束'],
      Width: ['Width', '寬度', '宽度'],
      Height: ['Height', '高度', '高度'],
      Coord: ['Coordinate', '座標', '座标'],
      Tamper_Detection: ['Tamper Detection', '入侵檢測', '入侵检测'],
      Motion_Detection: ['Motion Detection', '移動檢測', '移动检测'],
      Automatic_ROI: ['Automatic ROI', '自動目標檢測', '自动目标检测'],
      LightOnOff_Detection: [
        'LightOnOff Detection',
        '開關燈檢測',
        '开关灯检测',
      ],
      Object_Detection: ['Object Detection', '物體檢測', '物体检测'],
      Pedestrian_Detection: ['Pedestrian Detection', '行人檢測', '行人检测'],
      Regional_Motion_Sensor: [
        'Regional Motion Sensor',
        '區域運動傳感器',
        '区域运动传感器',
      ],
      Electric_Fence: ['Electric Fence', '電子圍離', '电子围篱'],
      Enable_Pan_Tilt_Zoom: [
        'Enable Pan-Tilt-Zoom',
        '開啓數碼平移/倾斜/變焦',
        '开启数码平移/倾斜/变焦',
      ],
      Manual: ['MANUAL', '手動', '手动'],
      Scan: ['SCAN', '巡航', '巡航'],
      PTZ_Mode: ['PTZ mode', 'PTZ 模式', 'PTZ 模式'],
      PTZ_Roi_Ratio: ['PTZ Roi Ratio', 'PTZ 目標比例', 'PTZ 目標比例'],
      Ctrl_Speed: ['Ctrl Speed', '目標移動速度', '目标移动速度'],
      Simple: ['Simple', '簡單', '简单'],
      Full: ['Full', '全手動', '全手动'],
      Control_mode: ['Control Mode', '控制模式', '控制模式'],
      Control_Panel: ['PTZ Control Panel', 'PTZ 控制平台', 'PTZ 控制平台'],
      PTZ_ctrl_win: [
        'PTZ SubChannel Window',
        'PTZ 副通道窗口',
        'PTZ 副通道窗口',
      ],
      NG_By_AROI_NOT_Enabled: [
        'To enable PTZ, please enable AROI first!!!',
        '開啟PTZ前，請先開啓AROI!!!',
        '开启PTZ前，请先开啓AROI!!!',
      ],
    };

    $scope.ptzCtx = {
      mode: '0',
      ctrl_speed: 4,
      ctrl_zoom_in: 0,
      ctrl_zoom_out: 0,
      ctrl_mv_top: 0,
      ctrl_mv_buttom: 0,
      ctrl_mv_left: 0,
      ctrl_mv_right: 0,
    };

    $scope.ptzPref = {
      master_id: 11,
      cmd_id: Cmd.AGTX_CMD_VIDEO_PTZ_CONF,
      cmd_type: 'set',
      enabled: 0,
      mode: 'AUTO',
      roi_height: 384,
      roi_width: 341,
      subwindow_disp: {
        win: [{ chn_idx: '1', win_idx: '0' }],
        win_num: 1,
      },
      win_speed_x: 32767,
      win_speed_y: 32767,
      zoom_speed_width: 0,
      zoom_speed_height: 0,
      speed_x: 4,
      speed_y: 4,
      mv_pos_x: 32767,
      mv_pos_y: 32767,
      zoom_level: 1024,
      zoom_change: 1024,
    };

    $scope.aroiPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_AROI_CONF,
      cmd_type: 'set',
      enabled: 0,
      en_skip_shake: 1,
      aspect_ratio_width: 0,
      aspect_ratio_height: 0,
      min_roi_width: 50,
      min_roi_height: 100,
      max_roi_width: 100,
      max_roi_height: 100,
      track_speed: 32,
      return_speed: 50,
    };

    $scope.ptzCtxModeChange = function () {
      if ($scope.ptzCtx.mode == '0') {
        $('#full_control').hide();
        $('#easy_control').show();
      } else {
        $('#easy_control').hide();
        $('#full_control').show();
      }
    };

    $scope.subWindowToInt = function () {
      $scope.ptzPref.subwindow_disp.win[0].chn_idx = parseInt(
        $scope.ptzPref.subwindow_disp.win[0].chn_idx
      );
      $scope.ptzPref.subwindow_disp.win[0].win_idx = parseInt(
        $scope.ptzPref.subwindow_disp.win[0].win_idx
      );
    };
    $scope.subWindowToStr = function () {
      $scope.ptzPref.subwindow_disp.win[0].chn_idx = $scope.ptzPref.subwindow_disp.win[0].chn_idx.toString();
      $scope.ptzPref.subwindow_disp.win[0].win_idx = $scope.ptzPref.subwindow_disp.win[0].win_idx.toString();
    };

    $scope.updateSpeed = function () {
      $scope.ptzPref.speed_x = $scope.ptzCtx.ctrl_speed;
      $scope.ptzPref.speed_y = $scope.ptzCtx.ctrl_speed;
    };

    $scope.updateCtrlSpeed = function () {
      $scope.ptzCtx.ctrl_speed = $scope.ptzPref.speed_x;
    };

    $scope.calcData = function () {
      $scope.ptzPref.speed_x = $scope.ptzCtx.ctrl_speed;
      $scope.ptzPref.speed_y = $scope.ptzCtx.ctrl_speed;
      $scope.ptzPref.mv_pos_x = 32767;
      $scope.ptzPref.mv_pos_y = 32767;
      $scope.ptzPref.zoom_level = 1024;
      $scope.ptzPref.zoom_change = 1024;
      $scope.ptzPref.win_speed_x = 32767;
      $scope.ptzPref.win_speed_y = 32767;
      $scope.ptzPref.zoom_speed_width = 0;
      $scope.ptzPref.zoom_speed_height = 0;

      if ($scope.ptzCtx.ctrl_zoom_in) {
        $scope.ptzPref.zoom_speed_width = -512;
        $scope.ptzPref.zoom_speed_height = -512;
      } else if ($scope.ptzCtx.ctrl_zoom_out) {
        $scope.ptzPref.zoom_speed_width = 512;
        $scope.ptzPref.zoom_speed_height = 512;
      } else if ($scope.ptzCtx.ctrl_mv_top) {
        $scope.ptzPref.win_speed_x = 0;
        $scope.ptzPref.win_speed_y = -1024;
      } else if ($scope.ptzCtx.ctrl_mv_buttom) {
        $scope.ptzPref.win_speed_x = 0;
        $scope.ptzPref.win_speed_y = 1024;
      } else if ($scope.ptzCtx.ctrl_mv_left) {
        $scope.ptzPref.win_speed_x = -1024;
        $scope.ptzPref.win_speed_y = 0;
      } else if ($scope.ptzCtx.ctrl_mv_right) {
        $scope.ptzPref.win_speed_x = 1024;
        $scope.ptzPref.win_speed_y = 0;
      }
    };
    $scope.sendData = function () {
      $scope.ptzPref.cmd_type = 'set';
      if ($scope.ptzPref.enabled) $scope.ptzPref.enabled = 1;
      else $scope.ptzPref.enabled = 0;
      $scope.subWindowToInt();
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.subWindowToStr();
      $scope.ptzPref.enabled = $scope.ptzPref.enabled == 1;
      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.ptzPref, function (key, value) {
          if (key === '$$hashKey') {
            return undefined;
          }
          return value;
        }),
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Result = resp;
          if (resp.data.rval >= 0) {
            $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          } else {
            $scope.cmdStatus =
              $scope.msg_t.WriteFail[$scope.lang] + ',rval=' + resp.data.rval;
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //   $scope.Result = "Post Error";
        }
      );
    };

    $scope.getAroiStatus = function () {
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('get aroi data');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify({
          "master_id": 1,
          "cmd_id": Cmd.AGTX_CMD_AROI_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_AROI_CONF');
          $scope.aroiPref = resp.data;
          $scope.aroiPref.enabled = $scope.aroiPref.enabled == 1;
          $scope.Result = resp;
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_AROI_CONF');
          console.log(resp);
        }
      );
    }

    $scope.getData = function (hide_msg = 0) {
      $scope.canApply = false;
      $scope.cmdStatus = $scope.msg_t.Loading[$scope.lang];
      $scope.getAroiStatus();
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
          "cmd_id": Cmd.AGTX_CMD_VIDEO_PTZ_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_VIDEO_PTZ_CONF');
          $scope.ptzPref = resp.data;
          $scope.canApply = true;
          $scope.ptzPref.enabled = $scope.ptzPref.enabled == 1;
          $scope.Result = resp;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          $scope.subWindowToStr();
          $scope.updateCtrlSpeed();
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_VIDEO_PTZ_CONF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.checkAroiStatus = function () {
      $scope.getAroiStatus();
      if ($scope.aroiPref.enabled == 0 && $scope.ptzPref.enabled == 1) {
        $scope.canApply = false;
        $scope.cmdStatus = $scope.msg_t.NG_By_AROI_NOT_Enabled[$scope.lang];
        console.log('AROI is enabled');
      } else {
        $scope.canApply = true;
        $scope.cmdStatus = $scope.msg_t.noTxt[$scope.lang];
        console.log('AROI is not enabled');
      }
    }

    $scope.getData();

    var key_dir = -1;
    var key_code = -999;
    var KEY = {
      "ZOOM_IN" : ["+", "="],
      "ZOOM_OUT" : ["-"],
      "ARROW_TOP" : "ArrowUp",
      "ARROW_LEFT": "ArrowLeft",
      "ARROW_RIGHT": "ArrowRight",
      "ARROW_BOTTOM": "ArrowDown"
    };

    $scope.keyPress = function (dir, e) {
      if (key_dir == dir && e.keyCode == key_code)
        return;
      //console.log(e.type, e.keyCode, e.key)
      key_dir = dir;
      key_code = e.keyCode;
      if ($scope.ptzPref.enabled) {
        if (dir == 1) {
          if (e.key == KEY.ARROW_BOTTOM) {
            $("#btn_mv_bottom").addClass("focus");
            $scope.mouse_update(3);
          } else if (e.key == KEY.ARROW_TOP) {
            $("#btn_mv_top").addClass("focus");
            $scope.mouse_update(2);
          } else if (e.key == KEY.ARROW_LEFT) {
            $("#btn_mv_left").addClass("focus");
            $scope.mouse_update(4);
          } else if (e.key == KEY.ARROW_RIGHT) {
            $("#btn_mv_right").addClass("focus");
            $scope.mouse_update(5);
          } else if (KEY.ZOOM_IN.includes(e.key)) {
            $("#btn_zoom_in").addClass("focus");
            $scope.mouse_update(0);
          } else if (KEY.ZOOM_OUT.includes(e.key)) {
            $("#btn_zoom_out").addClass("focus");
            $scope.mouse_update(1);
          }
        } else {
          if (KEY.ZOOM_IN.includes(e.key)) $("#btn_zoom_in").removeClass("focus");
          else if (KEY.ZOOM_OUT.includes(e.key)) $("#btn_zoom_out").removeClass("focus");
          else if (e.key == KEY.ARROW_LEFT) $("#btn_mv_left").removeClass("focus");
          else if (e.key == KEY.ARROW_RIGHT) $("#btn_mv_right").removeClass("focus");
          else if (e.key == KEY.ARROW_TOP) $("#btn_mv_top").removeClass("focus");
          else if (e.key == KEY.ARROW_BOTTOM) $("#btn_mv_bottom").removeClass("focus");

          $scope.mouse_update(-1);
        }
      }
      // 37 left 39 right 38 up 40 down 108 -
    };

    $scope.mouse_update = function (dir) {
      $scope.ptzCtx.ctrl_zoom_in = 0;
      $scope.ptzCtx.ctrl_zoom_out = 0;
      $scope.ptzCtx.ctrl_mv_top = 0;
      $scope.ptzCtx.ctrl_mv_buttom = 0;
      $scope.ptzCtx.ctrl_mv_left = 0;
      $scope.ptzCtx.ctrl_mv_right = 0;
      switch (dir) {
        case 0:
          $scope.ptzCtx.ctrl_zoom_in = 1;
          break;
        case 1:
          $scope.ptzCtx.ctrl_zoom_out = 1;
          break;
        case 2:
          $scope.ptzCtx.ctrl_mv_top = 1;
          break;
        case 3:
          $scope.ptzCtx.ctrl_mv_buttom = 1;
          break;
        case 4:
          $scope.ptzCtx.ctrl_mv_left = 1;
          break;
        case 5:
          $scope.ptzCtx.ctrl_mv_right = 1;
          break;
        default:
          break;
      }
      $scope.calcData();
      $scope.sendData(1);
    };
  },
]);
