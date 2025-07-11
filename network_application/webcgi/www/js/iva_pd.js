app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.IvaDropDown = setIvaDropdown(Iva_Index.pd);
    /* $scope.IvaMenu = setIvaPage(Iva_Index.pd); */
    $scope.CurPage = msg.Pedestrian_Detection;
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
      Enable_pedestrian_detection: [
        'Enable pedestrain detection',
        '開啓行人檢測',
        '开启行人检测',
      ],
      Min_aspect_ratio: [
        'Min aspect ratio:',
        '最小偵測寬高比:',
        '最小侦测宽高比:',
      ],
      Max_aspect_ratio: [
        'Max aspect ratio:',
        '最大偵測寬高比:',
        '最大侦测宽高比:',
      ],
      Min_size: ['Min size:', '最小偵測物體:', '最小侦测物体:'],
      Max_size: ['Max size:', '最大偵測物體:', '最大侦测物体:'],
      NG_By_EAIF_Enabled: [
        'Can not enable Pedestrain Detecction since Edge AI Framework is already enabled!!',
        '無法開啟行人檢測,因為物體檢測已經開啟了!!',
        '无法开启行人检测,因为物体检测已经开启了!!',
      ],
    };

    $scope.pdPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_PD_CONF,
      cmd_type: 'set',
      enabled: 0,
      /* 連結到 enabled */
      min_aspect_ratio_w: 1,
      /* 連結到 minimum aspect ratio */
      min_aspect_ratio_h: 5,
      /* 連結到 minimum aspect ratio */
      max_aspect_ratio_w: 1,
      /* 連結到 maximum aspect ratio */
      max_aspect_ratio_h: 1,
      /* 連結到 maximum aspect ratio */
      min_size: 0,
      /* 連結到 minimal size threshold */
      max_size: 100,
      /* 連結到 maximum size threshold */
      video_chn_idx: 0,
      /* 定值 */
    };

    $scope.eaifPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_EAIF_CONF,
      cmd_type: 'set',
      enabled: 0,
      obj_life_th: 0,
      target_idx: 0,
      api: 'CLASSIFY',
      data_fmt: 'JPEG',
      url: 'http://192.168.10.87:80',
      pos_stop_count_th: 3,
      pos_classify_period: 100,
      neg_classify_period: 25,
      video_chn_idx: 0,
      face_reco_model:"facenet",
      detect_model:"yolov5",
      classify_model:"shuffleNetV2",
      classify_cv_model:"C4",
      human_classify_model:"shuffleNetV2"
    };
    $scope.canApply = true;
    $scope.Result = 'result';
    $scope.check_size = function () {
      if ($scope.pdPref.min_size > $scope.pdPref.max_size) {
        $scope.pdPref.max_size = $scope.pdPref.min_size;
      }
    };
    $scope.getEaifStatus = function () {
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
          "cmd_id": Cmd.AGTX_CMD_EAIF_CONF,
          "cmd_type": "get",
        }),
      }).then(function (resp) {
        console.log('post getdata success: ' + 'AGTX_CMD_EAIF_CONF');
        console.log(resp);
        $scope.eaifPref = resp.data;
      })
    }

    $scope.checkEaifStatus = function () {
      $scope.getEaifStatus();
      if ($scope.eaifPref.enabled == 1) {
        $scope.canApply = false;
        $scope.cmdStatus = $scope.msg_t.NG_By_EAIF_Enabled[$scope.lang];
        console.log('EAIF is enabled');
      } else {
        $scope.canApply = true;
        $scope.cmdStatus = $scope.msg_t.noTxt[$scope.lang];
        console.log('EAIF is not enabled');
      }
    }

    $scope.sendData = function () {
      $scope.pdPref.cmd_type = 'set';
      if ($scope.pdPref.enabled) $scope.pdPref.enabled = 1;
      else $scope.pdPref.enabled = 0;
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.pdPref.enabled = $scope.pdPref.enabled == 1;

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.pdPref, function (key, value) {
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
      console.log('get data');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify({
          "master_id": 1,
          "cmd_id": Cmd.AGTX_CMD_PD_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success:' + 'AGTX_CMD_PD_CONF');
          $scope.pdPref = resp.data;
          $scope.canApply = true;
          $scope.pdPref.enabled = $scope.pdPref.enabled == 1;
          $scope.Result = resp;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error:' + 'AGTX_CMD_PD_CONF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getEaifStatus();
    $scope.getData(1);
  },
]);
