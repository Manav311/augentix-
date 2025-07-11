$(document).ready(function () {
  $('#shd_div').hide();
  $('#shd_div_toggle').removeClass('btn-info');
  $('#shd_div_toggle').addClass('btn-secondary');
});

app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.IvaDropDown = setIvaDropdown(Iva_Index.od);
    //$scope.IvaMenu = setIvaPage(Iva_Index.od);
    $scope.CurPage = msg.Object_Detection;
    $scope.msg_t = {
      augentix: ['Augentix', '多方科技', '多方科技'],
      noTxt: ['', '', ''],
      Auto: ['Auto', '自動', '自动'],
      Read_setting: ['Read out from device', '讀取設定', '读取设定'],
      Apply: ['Apply', '應用', '应用'],
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
      Enable_object_detection: [
        'Enable object detection',
        '開啓物體檢測',
        '开启物体检测',
      ],
      Quality: ['Quality:', '偵測品質:', '侦测品质:'],
      Obj_size_th: ['Object size threshold:', '最小偵測物體:', '最小侦测物体:'],
      Obj_life_th: [
        'Object life threshold:',
        '最小偵測物體生命阈值:',
        '最小侦测物体生命阈值:',
      ],
      Enable_shake_det: [
        'Detect shaking object',
        '偵測抖動中物體',
        '侦测抖动中物体',
      ],
      Longterm_life_th: [
        'Longterm item life threshold:',
        '長久物體激活閾值:',
        '长久物体激活阈值:',
      ],
      Instance_duration: [
        'Instance object duration(0.1s):',
        '閃爍物體時間(0.1秒):',
        '闪烁物体时间(0.1秒):',
      ],
      Shaking_update_duration: [
        'Shaking update duration (1s):',
        '抖動物體更新時間(秒):',
        '抖动物体更新时间(秒):',
      ],
      Longterm_dec_period: [
        'Longterm item life decrement period (1s):',
        '長久物體生命下降時間(秒):',
        '长久物体生命下降时间(秒):',
      ],
      Longterm_num: ['Longterm item number:', '長久物體數量:', '长久物体数量:'],
      Longterm_item: ['Longterm item', '長久物體', '长久物体'],
      Enable_GMV_motor: [
        'Enable GMV motor (For QA test)',
        '開啟GMV馬達 (QA測試用)',
        '开启GMV马达 (QA测试用)',
	  ],
      Enable_ODv5: [
        'Enable ODv5 (callback-based OD)',
        '開啟 ODv5 (callback-based OD)',
        '开启 ODv5 (callback-based OD)',
    ],
    OD_4: ['ODv4', 'ODv4', 'ODv4'],
    OD_5: ['ODv5', 'ODv5', 'ODv5'],
    };

    $scope.odPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_OD_CONF,
      cmd_type: 'set',
      enabled: 0,
      /* 連結到 enabled */
      en_shake_det: 1,
      od_qual: 70,
      /* 連結到 quality */
      od_track_refine: 86,
      /* 連結到 track_refine */
      od_size_th: 6,
      /* 連結到 size threshold */
      od_sen: 99,
      /* 連結到 sensitivity */
      video_chn_idx: 0,
      /* 定值 */
      en_motor:0,
      version: 'OD_V4',
      en_crop_outside_obj:1,
      en_stop_det:1,
      en_gmv_det:0
    };

    $scope.shdPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_SHD_CONF,
      cmd_type: 'set',
      enabled: true,
      obj_life_th: 32,
      sensitivity: 75,
      quality: 75,
      longterm_life_th: 10,
      instance_duration: 10,
      shaking_update_duration: 30,
      longterm_dec_period: 1800,
      longterm_num: 0,
      longterm_list: [
        {
          start_x: 0,
          start_y: 0,
          end_x: 0,
          end_y: 0,
        },
      ],
    };

    $scope.check_coordinate = function () {
      for (i = 0; i < $scope.shdPref.longterm_list.length; i++) {
        if (
          $scope.shdPref.longterm_list[i].start_x < 0 ||
          typeof $scope.shdPref.longterm_list[i].start_x != 'number'
        )
          $scope.shdPref.longterm_list[i].start_x = 0;
        if ($scope.shdPref.longterm_list[i].start_x > 99)
          $scope.shdPref.longterm_list[i].start_x = 99;
        if (
          $scope.shdPref.longterm_list[i].start_y < 0 ||
          typeof $scope.shdPref.longterm_list[i].start_y != 'number'
        )
          $scope.shdPref.longterm_list[i].start_y = 0;
        if ($scope.shdPref.longterm_list[i].start_y > 99)
          $scope.shdPref.longterm_list[i].start_y = 99;
        if (
          $scope.shdPref.longterm_list[i].end_x < 1 ||
          typeof $scope.shdPref.longterm_list[i].end_x != 'number'
        )
          $scope.shdPref.longterm_list[i].end_x = 1;
        if ($scope.shdPref.longterm_list[i].end_x > 100)
          $scope.shdPref.longterm_list[i].end_x = 100;
        if (
          $scope.shdPref.longterm_list[i].end_y < 1 ||
          typeof $scope.shdPref.longterm_list[i].end_y != 'number'
        )
          $scope.shdPref.longterm_list[i].end_y = 1;
        if ($scope.shdPref.longterm_list[i].end_y > 100)
          $scope.shdPref.longterm_list[i].end_y = 100;
        if (
          $scope.shdPref.longterm_list[i].start_x >=
          $scope.shdPref.longterm_list[i].end_x
        ) {
          $scope.shdPref.longterm_list[i].start_x =
            $scope.shdPref.longterm_list[i].end_x - 1;
        }
        if (
          $scope.shdPref.longterm_list[i].start_y >=
          $scope.shdPref.longterm_list[i].end_y
        ) {
          $scope.shdPref.longterm_list[i].start_y =
            $scope.shdPref.longterm_list[i].end_y - 1;
        }
      }
    };

    $scope.range = function (min, max) {
      var input = [];
      for (var i = min; i < max; i++) {
        input.push(i);
      }
      return input;
    };

    $scope.shdOnchange = function () {
      $('#shd_div').toggle();
      $('#shd_div_toggle').toggleClass('btn-info');
      $('#shd_div_toggle').toggleClass('btn-secondary');
    };

    $scope.Result = 'result';
    $scope.sendData = function () {
      $scope.odPref.cmd_type = 'set';
      if ($scope.odPref.enabled) $scope.odPref.enabled = 1;
      else $scope.odPref.enabled = 0;
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.odPref.enabled = $scope.odPref.enabled == 1;
      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.odPref, function (key, value) {
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
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          //	$scope.Result = "Post Error";
        }
      );
    };
    $scope.getData = function (hide_msg) {
      $scope.canApply = false;
      $scope.cmdStatus = $scope.msg_t.Loading[$scope.lang];
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('get data: ' + 'AGTX_CMD_OD_CONF');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify({
          "master_id": 1,
          "cmd_id": Cmd.AGTX_CMD_OD_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_OD_CONF');
          $scope.odPref = resp.data;
          $scope.canApply = true;
          $scope.odPref.enabled = $scope.odPref.enabled == 1;
          $scope.Result = resp;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_OD_CONF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.sendShdData = function () {
      $scope.shdPref.cmd_type = 'set';
      if ($scope.shdPref.enabled) $scope.shdPref.enabled = 1;
      else $scope.shdPref.enabled = 0;
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.shdPref.enabled = $scope.shdPref.enabled == 1;

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.shdPref, function (key, value) {
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
            $scope.cmdShdStatus = $scope.msg_t.WriteOK[$scope.lang];
          } else {
            $scope.cmdShdStatus =
              $scope.msg_t.WriteFail[$scope.lang] + ' rval=' + resp.data.rval;
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdShdStatus = $scope.msg_t.WriteFail[$scope.lang];
          //	$scope.Result = "Post Error";
        }
      );
    };
    $scope.getShdData = function (hide_msg) {
      $scope.cmdShdStatus = $scope.msg_t.Loading[$scope.lang];
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
          "cmd_id": Cmd.AGTX_CMD_SHD_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_SHD_CONF');
          $scope.shdPref = resp.data;
          $scope.shdPref.enabled = $scope.shdPref.enabled == 1;
          $scope.Result = resp;
          if (hide_msg != 1) {
            $scope.cmdShdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdShdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_SHD_CONF');
          console.log(resp);
          $scope.cmdShdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getData(0);
    $scope.getShdData(0);
  },
]);
