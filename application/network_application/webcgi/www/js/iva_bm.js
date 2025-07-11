app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.IvaDropDown = setIvaDropdown(Iva_Index.bm);
    //$scope.IvaMenu = setIvaPage(Iva_Index.bm);
    $scope.CurPage = msg.Baby_Monitor;
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
      Pet_Feeding_Monitor: [
        'Pet Feeding Monitor',
        '竉物飲食監控',
        '宠物饮食监控',
      ],
      Baby_Monitor: ['Baby Monitor', '嬰兒安全監控', '婴儿安全监控'],
      Enable_baby_monitor: [
        'Enable Baby Monitor',
        '開啓嬰兒安全監控',
        '开启婴儿安全监控',
      ],
      Reset_command: ['Reset command:', '重置指令:', '重置指令:'],
      Ctrl_command: ['Ctrl command:', '控制指令:', '控制指令:'],
      None: ['None', '無', '無'],
      Reset: ['Reset', '重置場景', '重置场景'],
      Monitor_region: ['Monitor Region', '監控區域', '监控区域'],
      Boundary_thickness: [
        'Border thickness:',
        '區域邊界厚度:',
        '区域边界厚度:',
      ],
      Quality: ['Quality:', '偵測品質:', '侦测品质:'],
      Suppression: ['Suppression (s):', '抑制時間 (秒):', '抑制时间 (秒):'],
      Time_buffer: ['Time buffer (s):', '時間緩冲 (秒):', '时间缓冲 (秒):'],
      Save: ['Save', '儲存', '储存'],
      Load: ['Load', '讀取', '读取'],
    };

    $scope.range = function (min, max) {
      var input = [];
      for (var i = min; i < max; i++) {
        input.push(i);
      }
      return input;
    };

    $scope.bmPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_BM_CONF,
      cmd_type: 'set',
      enabled: 1,
      reset: 0,
      sensitivity: 50,
      quality: 100,
      video_chn_idx: 0,
      roi: {
        start_x: 0,
        start_y: 0,
        end_x: 100,
        end_y: 100,
      },
      boundary_thickness: 15,
      suppression: 3,
      time_buffer: 600,
      obj_life_th: 32,
      data_ctrl: 'NONE',
    };

    $scope.Result = 'result';

    $scope.check_coordinate = function () {
      if (
        $scope.bmPref.roi.start_x < 0 ||
        typeof $scope.bmPref.roi.start_x != 'number'
      )
        $scope.bmPref.roi.start_x = 0;
      if ($scope.bmPref.roi.start_x > 99) $scope.bmPref.roi.start_x = 99;
      if (
        $scope.bmPref.roi.start_y < 0 ||
        typeof $scope.bmPref.roi.start_y != 'number'
      )
        $scope.bmPref.roi.start_y = 0;
      if ($scope.bmPref.roi.start_y > 99) $scope.bmPref.roi.start_y = 99;
      if (
        $scope.bmPref.roi.end_x < 1 ||
        typeof $scope.bmPref.roi.end_x != 'number'
      )
        $scope.bmPref.roi.end_x = 1;
      if ($scope.bmPref.roi.end_x > 100) $scope.bmPref.roi.end_x = 100;
      if (
        $scope.bmPref.roi.end_y < 1 ||
        typeof $scope.bmPref.roi.end_y != 'number'
      )
        $scope.bmPref.roi.end_y = 1;
      if ($scope.bmPref.roi.end_y > 100) $scope.bmPref.roi.end_y = 100;
      if ($scope.bmPref.roi.start_x >= $scope.bmPref.roi.end_x) {
        $scope.bmPref.roi.start_x = $scope.bmPref.roi.end_x - 1;
      }
      if ($scope.bmPref.roi.start_y >= $scope.bmPref.roi.end_y) {
        $scope.bmPref.roi.start_y = $scope.bmPref.roi.end_y - 1;
      }
    };

    $scope.sendData = function () {
      $scope.bmPref.cmd_type = 'set';
      if ($scope.bmPref.enabled) $scope.bmPref.enabled = 1;
      else $scope.bmPref.enabled = 0;
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.bmPref.enabled = $scope.bmPref.enabled == 1;

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.bmPref),
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
          "cmd_id": Cmd.AGTX_CMD_BM_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_BM_CONF');
          $scope.bmPref = resp.data;
          $scope.canApply = true;
          $scope.Result = resp;
          $scope.bmPref.enabled = $scope.bmPref.enabled == 1;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_BM_CONF');
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
