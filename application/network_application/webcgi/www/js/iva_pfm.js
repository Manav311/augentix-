app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.IvaDropDown = setIvaDropdown(Iva_Index.pfm);
    /* $scope.IvaMenu = setIvaPage(Iva_Index.pfm); */
    $scope.CurPage = msg.Pet_Feeding_Monitor;
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
      Enable_pet_feeding_monitor: [
        'Enable Pet Feeding Monitor',
        '開啓竉物飲食監控',
        '开启宠物饮食监控',
      ],
      Duration: ['Finish Time Duration (s):', '結束時間(秒):', '結束时间(秒):'],
      Register_command: ['Register Command:', '註冊指令: ', '注册指令: '],
      None: ['None', '無', '無'],
      Register: ['Register', '註冊場景', '注册场景'],
      Feeding: ['Feeding', '餵食', '喂食'],
      Region: ['Region of Feeding Plate', '餐盤區域', '餐盘区域'],
      Schedule: ['Schedule', '排程', '排程'],
      Time_num: ['Feeding Schedule', '餵食排程', '餵食排程'],
      Hour: ['Hour', '小時', '小时'],
      Minute: ['Minute', '分鐘', '分钟'],
      Second: ['Second', '秒', '秒'],
      Regis_to_feeding_interval: [
        'Registration-to-feeding Time Interval (s):',
        '註冊場景至竉物餵食相隔時間(秒):',
        '注册场景至宠物餵食相隔时间(秒):',
      ],
    };

    angular.element(document).ready(function () {
      var i;
      for (i = 0; i < 10; i++) {
        var time_schedule = { hr: 0, min: 0, sec: 0 };
        $scope.pfmJs.schedule.push(time_schedule);
      }
    });

    $scope.schedule_change = function (value) {
      if (value == 1) $scope.pfmPref.time_number++;
      if (value == -1) $scope.pfmPref.time_number--;
      $scope.pfmPref.time_number =
        $scope.pfmPref.time_number > 10 ? 10 : $scope.pfmPref.time_number;
      $scope.pfmPref.time_number =
        $scope.pfmPref.time_number < 0 ? 0 : $scope.pfmPref.time_number;
    };

    $scope.range = function (min, max) {
      var input = [];
      for (var i = min; i < max; i++) {
        input.push(i);
      }
      return input;
    };

    $scope.pfmPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_PFM_CONF,
      cmd_type: 'set',
      enabled: 1,
      endurance: 50,
      register_scene: 0,
      sensitivity: 50,
      video_chn_idx: 0,
      roi: {
        start_x: 35,
        start_y: 65,
        end_x: 65,
        end_y: 100,
      },
      regis_to_feeding_interval: 300,
      time_number: 0,
      schedule: [25200 /* 07:00 */, 68400 /* 19:00 */, 0, 0, 0, 0, 0, 0, 0, 0],
    };
    $scope.pfmJs = {
      schedule: [],
    };

    $scope.Result = 'result';

    $scope.update_time = function (i) {
      $scope.pfmPref.schedule[i] =
        $scope.pfmJs.schedule[i].hr * 60 * 60 +
        $scope.pfmJs.schedule[i].min * 60 +
        $scope.pfmJs.schedule[i].sec;
    };

    $scope.reload_time = function (i) {
      $scope.pfmJs.schedule[i].hr = Math.floor(
        $scope.pfmPref.schedule[i] / (60 * 60)
      );
      var mins = $scope.pfmPref.schedule[i] % (60 * 60);
      $scope.pfmJs.schedule[i].min = Math.floor(mins / 60);
      var secs = $scope.pfmPref.schedule[i] % 60;
      $scope.pfmJs.schedule[i].sec = Math.floor(secs);
    };

    $scope.check_coordinate = function () {
      if (
        $scope.pfmPref.roi.start_x < 0 ||
        typeof $scope.pfmPref.roi.start_x != 'number'
      )
        $scope.pfmPref.roi.start_x = 0;
      if ($scope.pfmPref.roi.start_x > 99) $scope.pfmPref.roi.start_x = 99;
      if (
        $scope.pfmPref.roi.start_y < 0 ||
        typeof $scope.pfmPref.roi.start_y != 'number'
      )
        $scope.pfmPref.roi.start_y = 0;
      if ($scope.pfmPref.roi.start_y > 99) $scope.pfmPref.roi.start_y = 99;
      if (
        $scope.pfmPref.roi.end_x < 1 ||
        typeof $scope.pfmPref.roi.end_x != 'number'
      )
        $scope.pfmPref.roi.end_x = 1;
      if ($scope.pfmPref.roi.end_x > 100) $scope.pfmPref.roi.end_x = 100;
      if (
        $scope.pfmPref.roi.end_y < 1 ||
        typeof $scope.pfmPref.roi.end_y != 'number'
      )
        $scope.pfmPref.roi.end_y = 1;
      if ($scope.pfmPref.roi.end_y > 100) $scope.pfmPref.roi.end_y = 100;
      if ($scope.pfmPref.roi.start_x >= $scope.pfmPref.roi.end_x) {
        $scope.pfmPref.roi.start_x = $scope.pfmPref.roi.end_x - 1;
      }
      if ($scope.pfmPref.roi.start_y >= $scope.pfmPref.roi.end_y) {
        $scope.pfmPref.roi.start_y = $scope.pfmPref.roi.end_y - 1;
      }
    };

    $scope.sendData = function () {
      $scope.pfmPref.cmd_type = 'set';
      if ($scope.pfmPref.enabled) $scope.pfmPref.enabled = 1;
      else $scope.pfmPref.enabled = 0;
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.pfmPref.enabled = $scope.pfmPref.enabled == 1;

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.pfmPref),
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
          "cmd_id": Cmd.AGTX_CMD_PFM_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_PFM_CONF');
          $scope.pfmPref = resp.data;
          $scope.canApply = true;
          $scope.Result = resp;
          $scope.pfmPref.enabled = $scope.pfmPref.enabled == 1;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          var k;
          for (k = 0; k < $scope.pfmPref.time_number; k++) {
            $scope.reload_time(k);
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_PFM_CONF');
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
