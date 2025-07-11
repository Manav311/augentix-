app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.IvaDropDown = setIvaDropdown(Iva_Index.dk);
    //$scope.IvaMenu = setIvaPage(Iva_Index.dk);
    $scope.CurPage = msg.Door_Keeper;
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
      Enable_door_keepr: [
        'Enable Door Keeper',
        '開啓',
        '开启',
      ],
      Fall_Detection: ['Fall Detection', '跌倒檢測', '跌倒检测'],
      Door_keeper: ['Door keeper', '門鈴監控', '门铃监控'],
      Obj_life_th: [
        'Object life threshold:',
        '最小偵測物體生命阈值:',
        '最小侦测物体生命阈值:',
      ],
      Loiter_period_th: [
        'Object Period threshold for loitering status (1/100 s):',
        '最小偵測物體徘徊狀態阈值 (1/100 秒):',
        '最小侦测物体徘徊状态阈值 (1/100 秒):',
      ],
      Overlap_ratio_th: [
        'Overlap threshold with roi (percentage):',
        '最小偵測物體與重點區域的重疊率 (百分比):',
        '最小侦测物体与重点区域的重叠率 (百分比):',
      ],
      Region: ['Region', '區域', '区域'],
      Start: ['Start', '起始', '起始'],
      End: ['End', '結束', '结束'],
    };

    $scope.range = function (min, max) {
      var input = [];
      for (var i = min; i < max; i++) {
        input.push(i);
      }
      return input;
    };

    $scope.dkConf = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_DK_CONF,
      cmd_type: 'set',
      video_chn_idx: 0,
      enabled: 0,
      obj_life_th: 30,
      loiter_period_th: 300,
      overlap_ratio_th: 25,
      roi: {
        start_x: 35,
        start_y: 0,
        end_x: 65,
        end_y: 100
      }
    };

    $scope.Result = 'result';

    $scope.sendData = function () {
      $scope.dkConf.cmd_type = 'set';
      if ($scope.dkConf.enabled) $scope.dkConf.enabled = 1;
      else $scope.dkConf.enabled = 0;
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.dkConf.enabled = $scope.dkConf.enabled == 1;

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.dkConf),
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
          "cmd_id": Cmd.AGTX_CMD_DK_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_DK_CONF');
          $scope.dkConf = resp.data;
          $scope.canApply = true;
          $scope.Result = resp;
          $scope.dkConf.enabled = $scope.dkConf.enabled == 1;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_DK_CONF');
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
