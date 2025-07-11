app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.IvaDropDown = setIvaDropdown(Iva_Index.ef);
    /* $scope.IvaMenu = setIvaPage(Iva_Index.ef); */
    $scope.CurPage = msg.Electric_Fence;
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
      ObjOverLimit: [
        'Object minimal width/height is greater than maximal value!! ',
        '最小偵測物件寬度/長度大於最大值!!',
        '最小侦测对象宽度/长度大于最大值!!',
      ],
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
      Enable_electric_fence: [
        'Enable electric fence',
        '開啓電子圍籬檢測',
        '开启电子围篱检测',
      ],
      EF_active_lines: ['Active lines', '有效線數', '有效线数'],
      EF_Line: ['Line', '線', '线'],
      Obj_v_th: [
        'Object speed threshold',
        '最小偵測物體速度',
        '最小侦测物体速度',
      ],
      Obj_min_w: [
        'Object minimal width',
        '最小偵測物件寬度',
        '最小侦测对象宽度',
      ],
      Obj_min_h: [
        'Object minimal height',
        '最小偵測物件長度',
        '最小侦测对象长度',
      ],
      Obj_max_w: [
        'Object maximal width',
        '最大偵測物件寬度',
        '最大侦测对象宽度',
      ],
      Obj_max_h: [
        'Object maximal height',
        '最大偵測物件長度',
        '最大侦测对象长度',
      ],
      Obj_area: ['Object area', '最小偵測物件面積', '最小侦测对象面积'],
      EF_line_mode: [
        'Efence line mode',
        '電子圍離偵測模式',
        '电子围离侦测模式',
      ],
      EF_neg: ['Negative direction', '反向', '反向'],
      EF_pos: ['Positive direction', '正向', '正向'],
      EF_both: ['BOTH', '雙向', '双向'],
    };

    $scope.efPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_EF_CONF,
      cmd_type: 'set',
      line_cnt: 1,
      line_list: [
        /* 定值 */ {
          end_x: 50,
          /* 值域 0-100，表示畫面的百分比 */
          end_y: 100,
          /* 值域 0-100，表示畫面的百分比 */
          id: 0,
          /* 表示第幾個 line */
          start_x: 50,
          /* 值域 0-100，表示畫面的百分比 */
          start_y: 0,
          /* 值域 0-100，表示畫面的百分比 */
          mode: 'DIR_BOTH',
          /* Direction */
          obj_min_w: 0,
          obj_min_h: 0,
          obj_max_w: 50,
          obj_max_h: 50,
          obj_area: 0,
          /* Object size threshold */
          obj_v_th: 0,
          /* Object speed threshold */
        },
      ],
      enabled: 0,
      /* 連結到 enabled */
      video_chn_idx: 0,
      /* 定值 */
    };
    $scope.Result = 'result';
    $scope.check_coordinate = function () {
      for (i = 0; i < $scope.efPref.line_list.length; i++) {
        if (
          $scope.efPref.line_list[i].start_x < 0 ||
          typeof $scope.efPref.line_list[i].start_x != 'number'
        )
          $scope.efPref.line_list[i].start_x = 0;
        if ($scope.efPref.line_list[i].start_x > 100)
          $scope.efPref.line_list[i].start_x = 100;
        if (
          $scope.efPref.line_list[i].start_y < 0 ||
          typeof $scope.efPref.line_list[i].start_y != 'number'
        )
          $scope.efPref.line_list[i].start_y = 0;
        if ($scope.efPref.line_list[i].start_y > 100)
          $scope.efPref.line_list[i].start_y = 100;
        if (
          $scope.efPref.line_list[i].end_x < 0 ||
          typeof $scope.efPref.line_list[i].end_x != 'number'
        )
          $scope.efPref.line_list[i].end_x = 0;
        if ($scope.efPref.line_list[i].end_x > 100)
          $scope.efPref.line_list[i].end_x = 100;
        if (
          $scope.efPref.line_list[i].end_y < 0 ||
          typeof $scope.efPref.line_list[i].end_y != 'number'
        )
          $scope.efPref.line_list[i].end_y = 0;
        if ($scope.efPref.line_list[i].end_y > 100)
          $scope.efPref.line_list[i].end_y = 100;
      }
    };
    $scope.is_valid_obj_size = function () {
      //obj_min should not bigger than obj_max
      for (i = 0; i < $scope.efPref.line_list.length; i++) {
        if (
          $scope.efPref.line_list[i].obj_min_w >
          $scope.efPref.line_list[i].obj_max_w
        ) {
          $scope.cmdStatus = $scope.msg_t.ObjOverLimit[$scope.lang];
          return false;
        }
        if (
          $scope.efPref.line_list[i].obj_min_h >
          $scope.efPref.line_list[i].obj_max_h
        ) {
          $scope.cmdStatus = $scope.msg_t.ObjOverLimit[$scope.lang];
          return false;
        }
      }
      return true;
    };
    $scope.line_cnt_change = function () {
      if (
        $scope.efPref.line_cnt < 1 ||
        typeof $scope.efPref.line_cnt != 'number'
      )
        $scope.efPref.line_cnt = 1;
      if ($scope.efPref.line_cnt > 16) $scope.efPref.line_cnt = 16;

      if ($scope.efPref.line_cnt > $scope.efPref.line_list.length) {
        // Add array
        len = $scope.efPref.line_list.length;
        line_cnt = $scope.efPref.line_cnt - len;
        for (i = 0; i < line_cnt; i++) {
          $scope.efPref.line_list.push({
            id: len + i,
            start_x: 0,
            start_y: 0,
            end_x: 10,
            end_y: 10,
            obj_min_w: 0,
            obj_min_d: 0,
            obj_max_w: 50,
            obj_max_d: 50,
            obj_area: 0,
            obj_v_th: 0,
            mode: 'DIR_BOTH',
          });
        }
      }
      if ($scope.efPref.line_cnt < $scope.efPref.line_list.length) {
        // Remove array
        var len = $scope.efPref.line_list.length;
        var line_cnt = len - $scope.efPref.line_cnt;
        var i;
        for (i = 0; i < line_cnt; i++) {
          $scope.efPref.line_list.pop();
        }
      }
    };
    $scope.sendData = function () {
      $scope.check_coordinate();

      if (!$scope.is_valid_obj_size()) {
        console.log('Wrong object minimal value');
        return;
      }

      $scope.efPref.cmd_type = 'set';
      if ($scope.efPref.enabled) $scope.efPref.enabled = 1;
      else $scope.efPref.enabled = 0;
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.efPref.enabled = $scope.efPref.enabled == 1;

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.efPref, function (key, value) {
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
      $scope.cmd = 
      '/cgi-bin/msg.cgi';
      console.log('get data');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify({
          "master_id": 1,
          "cmd_id": Cmd.AGTX_CMD_EF_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_EF_CONF');
          $scope.efPref = resp.data;
          $scope.canApply = true;
          $scope.efPref.enabled = $scope.efPref.enabled == 1;
          $scope.line_cnt_change();
          $scope.Result = resp;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_EF_CONF');
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
