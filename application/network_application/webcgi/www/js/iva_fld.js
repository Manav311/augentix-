app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.IvaDropDown = setIvaDropdown(Iva_Index.fld);
    //$scope.IvaMenu = setIvaPage(Iva_Index.fld);
    $scope.CurPage = msg.Fall_Detection;
    $scope.msg_t = {
      augentix: ['Augentix', '多方科技', '多方科技'],
      noTxt: ['', '', ''],
      Auto: ['Auto', '自動', '自动'],
      Read_setting: ['Read out from device', '讀取設定', '读取设定'],
      Apply: ['Apply', '應用', '应用'],
      Loading: ['Loading...', '讀取中...', '读取中...'],
      Reboot: ['Reboot', '重新開機', '重新开机'],
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      NoRespond: ['Device no respond!!', '設備無反應!!', '设备无反应!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
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
      Pet_Feeding_Monitor: [
        'Pet Feeding Monitor',
        '竉物飲食監控',
        '宠物饮食监控',
      ],
      Baby_Monitor: ['Baby Monitor', '嬰兒安全監控', '婴儿安全监控'],
      Enable_fall_detection: [
        'Enable Fall Detection',
        '開啓跌倒檢測',
        '开启跌倒检测',
      ],
      Fall_Detection: ['Fall Detection', '跌倒檢測', '跌倒检测'],
      Obj_life_th: [
        'Object life threshold:',
        '最小偵測物體生命阈值:',
        '最小侦测物体生命阈值:',
      ],
      Obj_falling_mv_th: [
        'Object falling mv threshold ( 0.25 pixel / frame):',
        '最小偵測物體跌倒速度阈值 (0.25 像素/幀):',
        '最小侦测物体跌倒速度阈值 (0.25 像素/帧):',
      ],
      Obj_stop_mv_th: [
        'Object stop mv threshold (0.25 pixel / frame):',
        '最小偵測物體靜止速度阈值 (0.25 像素/幀):',
        '最小侦测物体静止速度阈值 (0.25 像素/帧):',
      ],
      Obj_high_ratio_th: [
        'Object high ratio threshold with height history of object:',
        '最小偵測物體倒和未跌前高度變化百分比:',
        '最小侦测物体倒和未跌前高度变化百分比:',
      ],
      Falling_period_th: [
        'Object period threshold for falling status (1/100 s):',
        '最小偵測物體跌倒時間 (1/100 秒):',
        '最小侦测物体跌倒时间阈值 (1/100 秒):',
      ],
      Down_period_th: [
        'Object period threshold for down status (1/100 s):',
        '最小偵測物體倒後可動多久阈值 (1/100 秒):',
        '最小侦测物体倒后可动多久阈值 (1/100 秒):',
      ],
      Fallen_period_th: [
        'Object period threshold for fallen status (1/100 s):',
        '最小偵測物體判定已跌倒時間阈值 (1/100 秒):',
        '最小侦测物体判定已跌倒时间阈值 (1/100 秒):',
      ],
      Demo_level: [
        'Demo level of detail:',
        '展示細節:',
        '展示细节:',
      ],
      Simple: ['Simple', '簡易 ', '简易 '],
      Detailed: ['Detailed', '詳細 ', '详细 '],   
    };

    $scope.range = function (min, max) {
      var input = [];
      for (var i = min; i < max; i++) {
        input.push(i);
      }
      return input;
    };

    $scope.fldConf = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_FLD_CONF,
      cmd_type: 'set',
      video_chn_idx: 0,
      enabled: 0,
      demo_level: 0,
      down_period_th: 200,
      fallen_period_th: 500,
      falling_period_th: 200,
      obj_falling_mv_th: 28,
      obj_high_ratio_th: 75,
      obj_life_th: 0,
      obj_stop_mv_th: 4,
    };

    $scope.Result = 'result';

    $scope.sendData = function () {
      $scope.fldConf.cmd_type = 'set';
      if ($scope.fldConf.enabled) $scope.fldConf.enabled = 1;
      else $scope.fldConf.enabled = 0;
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.fldConf.enabled = $scope.fldConf.enabled == 1;

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.fldConf),
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
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          //	$scope.Result = "Post Error";
        }
      );
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
          "cmd_id": Cmd.AGTX_CMD_FLD_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_FLD_CONF');
          $scope.fldConf = resp.data;
          $scope.canApply = true;
          $scope.Result = resp;
          $scope.fldConf.enabled = $scope.fldConf.enabled == 1;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_FLD_CONF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getData(1);
  },
]);
