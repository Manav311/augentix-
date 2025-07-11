app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.IvaDropDown = setIvaDropdown(Iva_Index.aroi);
    //$scope.IvaMenu = setIvaPage(Iva_Index.aroi);
    $scope.CurPage = msg.Automatic_ROI;
    $scope.msg_t = {
      augentix: ['Augentix', '多方科技', '多方科技'],
      noTxt: ['', '', ''],
      Auto: ['Auto', '自動', '自动'],
      Read_setting: ['Read out from device', '讀取設定', '读取设定'],
      Apply: ['Apply', '應用', '应用'],
      Loading: ['Loading...', '讀取中...', '读取中...'],
      Reboot: ['Reboot', '重新開機', '重新开机'],
      Switch2SysupdOS: ['System Update', '系統更新', '系统更新'],
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
      Edge_AI_Framework: [
        'Edge AI Framework',
        '邊緣人工智慧應用',
        '边缘人工智慧应用',
      ],
      Object_Detection: ['Object Detection', '物體檢測', '物体检测'],
      Pedestrian_Detection: ['Pedestrian Detection', '行人檢測', '行人检测'],
      Regional_Motion_Sensor: [
        'Regional Motion Sensor',
        '區域運動傳感器',
        '区域运动传感器',
      ],
      Electric_Fence: ['Electric Fence', '電子圍離', '电子围篱'],
      Enable_aroi_detection: [
        'Enable Automatic ROI',
        '開啓自動目標檢測',
        '开启自动目标检测',
      ],
      Enable_fixed_aspect_ratio: [
        'Enable Fixed Aspect Ratio',
        '保持相對比例',
        '保持相对比例',
      ],
      Minimum_ROI: ['Minimum ROI', '最小範圍', '最小范围'],
      Maximum_ROI: ['Maximum_ROI', '最大範圍', '最大范围'],
      Track_speed: ['Track Speed', '追蹤速度', '追踪速度'],
      Return_speed: ['Return Speed', '回歸速度', '回归速度'],
      Enable_subwindow: [
        'Enable AROI on Sub-Stream',
        '開啟次碼流目標檢測',
        '开启次码流目标检测',
      ],
      Channel_index: ['Channel Index', '通道編號', '信道编号'],
      Window_index: ['Window Index', '視窗編號', '视窗编号'],
      Enable_skip_shake: [
        'Skip shaking object',
        '無視抖動中物體',
        '无视抖动中物体',
      ],
      NG_By_PTZ_Enabled: [
        'To disable AROI, please disable PTZ first!!!',
        '關閉AROI前，請先關閉PTZ!!!',
        '关闭AROI前，请先关闭PTZ!!!',
      ],
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

    $scope.aroiJs = {
      ar_enabled: 0,
    };
    $scope.isValidAr = function () {
      return !!(
        $scope.aroiPref.aspect_ratio_width &&
        $scope.aroiPref.aspect_ratio_height
      );
    };
    $scope.onchange = function () {
      if ($scope.aroiJs.ar_enabled && !$scope.isValidAr()) {
        $scope.aroiPref.aspect_ratio_width = 1;
        $scope.aroiPref.aspect_ratio_height = 1;
        return;
      }
      if (!$scope.aroiJs.ar_enabled) {
        $scope.aroiPref.aspect_ratio_width = 0;
        $scope.aroiPref.aspect_ratio_height = 0;
        return;
      }
    };
    $scope.check_size = function () {
      if ($scope.aroiPref.min_roi_width > $scope.aroiPref.max_roi_width) {
        $scope.aroiPref.max_roi_width = $scope.aroiPref.min_roi_width;
      }
      if ($scope.aroiPref.min_roi_height > $scope.aroiPref.max_roi_height) {
        $scope.aroiPref.max_roi_height = $scope.aroiPref.min_roi_height;
      }
    };
    $scope.Result = 'result';
    $scope.sendData = function () {
      $scope.aroiPref.cmd_type = 'set';
      if ($scope.aroiPref.enabled) $scope.aroiPref.enabled = 1;
      else $scope.aroiPref.enabled = 0;
      if ($scope.aroiPref.en_skip_shake) $scope.aroiPref.en_skip_shake = 1;
      else $scope.aroiPref.en_skip_shake = 0;
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.aroiPref.enabled = $scope.aroiPref.enabled == 1;
      $scope.aroiPref.en_skip_shake = $scope.aroiPref.en_skip_shake == 1;
      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.aroiPref, function (key, value) {
          if (key === '$$hashKey') {
            return undefined;
          }
          return value;
        }),
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Result = resp;
          if (resp.data.rval == 0) {
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
          //   $scope.Result = "Post Error";
        }
      );
    };

    $scope.getPtzStatus = function () {
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
          $scope.ptzPref.enabled = $scope.ptzPref.enabled == 1;
          $scope.Result = resp;
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_VIDEO_PTZ_CONF');
          console.log(resp);
        }
      );
    }

    $scope.getData = function (hide_msg) {
      $scope.canApply = false;
      $scope.cmdStatus = $scope.msg_t.Loading[$scope.lang];
      $scope.getPtzStatus();
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
          "cmd_id": Cmd.AGTX_CMD_AROI_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_AROI_CONF');
          $scope.aroiPref = resp.data;
          $scope.canApply = true;
          $scope.aroiPref.enabled = $scope.aroiPref.enabled == 1;
          $scope.aroiPref.en_skip_shake = $scope.aroiPref.en_skip_shake == 1;
          $scope.Result = resp;
          $scope.updatePage();
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_AROI_CONF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.checkPtzStatus = function () {
      $scope.getPtzStatus();
      if ($scope.ptzPref.enabled == 1 && $scope.aroiPref.enabled == 0) {
        $scope.canApply = false;
        $scope.cmdStatus = $scope.msg_t.NG_By_PTZ_Enabled[$scope.lang];
        console.log('PTZ is enabled');
      } else {
        $scope.canApply = true;
        $scope.cmdStatus = $scope.msg_t.noTxt[$scope.lang];
        console.log('PTZ is not enabled');
      }
    }

    $scope.updatePage = function () {
      $scope.aroiJs.ar_enabled = $scope.isValidAr();
    };
    $scope.getData();
    $scope.updatePage();
  },
]);
