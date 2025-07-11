app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    var hidden_v = $location.search()['v']
    if (hidden_v) {
      $('.hide').removeClass("hide").addClass("show");
    }
    $scope.LMenu = LMenu;
    $scope.canShow = true;
    if (window.location.href.substr(window.location.href.lastIndexOf('/') + 1) == "iva_hd.html") {
      $scope.IvaDropDown = setIvaDropdown(Iva_Index.hd);
      $scope.CurPage = msg.Human_Detection;
      $scope.canShow = true;
    } else if (window.location.href.substr(window.location.href.lastIndexOf('/') + 1) == "iva_fd.html") {
      $scope.IvaDropDown = setIvaDropdown(Iva_Index.fd);
      $scope.CurPage = msg.Face_Detection;
      $scope.canShow = false;
    }
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
      Target_idx: ['Target video window', '目標影像', '目标影像'],
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
      Enable_edge_ai_framework: [
        'Enable Edge AI Framework',
        '開啓物體檢測',
        '开启物体检测',
      ],
      Obj_Life_Thr: [
        'Object life threshold:',
        '最小偵測物體生命阈值:',
        '最小侦测物体生命阈值:',
      ],
      Api: ['Appication:', '應用:', '应用:'],
      Face_Detect: ['Face Detection', '人臉檢測', '人臉检测'],
      Detect: ['Detect', '物體檢測', '物体检测'],
      Classify: ['Classify', '物體分類', '物体分类'],
      Classify_cv: ['Classify(CV)', '物體分類(CV)', '物体分类(CV)'],
      Face_reco: ['Face Recognition', '臉部辨識', '脸部辨识'],
      Enable_Human_Classify: ['Enable Human Detection', '開啓人形檢測', '开启人形检测'],
      Enable_Face_Detect: ['Enable Face Detection', '開啓人臉檢測', '开启人臉检测'],
      Data_format: ['Data Format', '資料格式', '资料格式'],
      Url: ['Url', '網址', '网址'],
      Jpeg: ['JPEG', 'JPEG', 'JPEG'],
      Width: ['Width', '寬度', '宽度'],
      Height: ['Height', '高度', '高度'],
      Snapshot_Size: ['Snapshot Size', '快照', '快照'],
      Raw_Y: ['RAW Y', '原始資料Y值', '原始资料Y值'],
      Raw_YUV: ['RAW YUV', '原始資料YUV值', '原始资料YUV值'],
      Raw_RGB: ['RAW RGB', '原始資料RGB值', '原始资料RGB值'],
      Mpi_Jpeg: ['MPI JPEG', 'MPI JPEG', 'MPI JPEG'],
      Mpi_RawY: ['MPI RAW Y', 'MPI 原始資料Y值', 'MPI 原始资料Y值'],
      Mpi_RawYUV: ['MPI RAW YUV', 'MPI 原始資料YUV值', 'MPI 原始资料YUV值'],
      Mpi_RawRGB: ['MPI RAW RGB', 'MPI 原始資料RGB值', 'MPI 原始资料RGB值'],
      Pos_Stop_Count_Thr: ['Positive Stop Count Threshold', '正值停止數量閾值', '正值停止数量阈值'],
      Pos_Classify_Period: ['Positive Classify Period(# frames)', '正值物體分類間隔(# frames)', '正值物体分类间隔(# frames)'],
      Neg_Classify_Period: ['Negative Classify Period(# frames)', '負值物體分類間隔(# frames)', '负值物体分类间隔(# frames)'],
      Detection_Period: ['Detection Period(# frames)', '檢測間隔(# frames)', '检测间隔(# frames)'],
      Obj_Exist_Classify_Period: ['Object Exist Classify Period(# frames)', '物體分類間隔(# frames)', '物体分类间隔(# frames)'],
      Inf_With_ObjList: ['Enable Object List Based Model Inference', '開啓基於物體清單的模型推論', '开启基于物体清单的模型推论'],
      NG_By_PD_Enabled: [
        'Can not enable Edge AI Framework since Pedestrain Detecction is already enabled!!',
        '無法開啟物體檢測,因為行人檢測已經開啟了!!',
        '无法开启物体检测,因为行人检测已经开启了!!',
      ],
    };

    $scope.toggle_mode = 0;
    $scope.toggle_button_text = "INAPP";

    $scope.toggle_inference_mode = function()
    {
      $scope.toggle_mode = $scope.toggle_mode == 0;
      if ($scope.toggle_mode) {
        $scope.toggle_button_text = "REMOTE";
        $scope.eaifPref.url='inapp';
        $scope.eaifPref.data_fmt='Y';
        $scope.eaifPref.api = "HUMAN_CLASSIFY";
        $scope.eaifPref.human_classify_model = "/system/eaif/models/classifiers/shuffleNetV2/inapp.ini";
      } else {
        $scope.toggle_button_text = "INAPP";
        $scope.eaifPref.url='http://192.168.10.87:80';
        $scope.eaifPref.data_fmt='JPEG';
        $scope.eaifPref.api = "HUMAN_CLASSIFY";
        $scope.eaifPref.human_classify_model = "shuffleNetV2";
      }
    }

    $scope.update_api_tag = function ()
    {
      if ($scope.eaifPref.api == "DETECT") {
        $(".classify_tag").removeClass("show").addClass("hide");
        $(".detection_tag").removeClass("hide").addClass("show");
      } else {
        $(".classify_tag").removeClass("hide").addClass("show");
        $(".detection_tag").removeClass("show").addClass("hide");
      }
      if ($scope.eaifPref.api == "FACEDET") {
        $scope.canShow = false;
        $scope.eaifPref.data_fmt = "MPI_RGB";
      } else if ($scope.eaifPref.api == "HUMAN_CLASSIFY") {
        $scope.canShow = true;
        $scope.eaifPref.data_fmt = "MPI_Y";
      } else {
        $scope.eaifPref.api = "HUMAN_CLASSIFY"
        $scope.canShow = true;
        $scope.eaifPref.data_fmt = "MPI_Y";
      }
    }

    $scope.getNumber = function (num) {
      var temp = [];
      for (var j = 0; j < num; j++) {
        temp.push(j);
      }
      return temp;
    };

    $scope.eaifPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_EAIF_CONF,
      cmd_type: 'set',
      api:"HUMAN_CLASSIFY",
      classify_cv_model:"C4",
      classify_model:"shuffleNetV2",
      data_fmt:"MPI_RGB",
      detect_model:"yolov5",
      detection_period:5,
      enabled:0,
      face_detect_model:"\/system\/eaif\/models\/facereco\/inapp_mtcnn.ini",
      face_name:"none",
      face_reco_model:"\/system\/eaif\/models\/facereco\/inapp_facereco.ini",
      facereco_roi_ex:60,
      facereco_roi_ey:70,
      facereco_roi_sx:30,
      facereco_roi_sy:30,
      human_classify_model:"\/system\/eaif\/models\/classifiers\/shuffleNetV2\/inapp.ini",
      inf_cmd:"NONE",
      inf_with_obj_list:1,
      neg_classify_period:25,
      obj_exist_classify_period:0,
      obj_life_th:16,
      pos_classify_period:100,
      pos_stop_count_th:3,
      snapshot_height:720,
      snapshot_width:1280,
      target_idx:0,
      url:"inapp",
      video_chn_idx:0,
    };

    $scope.pdPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_PD_CONF,
      cmd_type: 'set',
      enabled: 0,
      min_aspect_ratio_w: 1,
      min_aspect_ratio_h: 5,
      max_aspect_ratio_w: 1,
      max_aspect_ratio_h: 1,
      min_size: 0,
      max_size: 100,
      video_chn_idx: 0,
    };

    $scope.target_idx = {"dev":0,"chn":0,"win":0};
    $scope.ele = ["face_reco","detect", "classify", "classify_cv", 'human_classify'];
    $scope.ele_value = [
    $scope.eaifPref.face_reco_model,
    $scope.eaifPref.detect_model,
    $scope.eaifPref.classify_model,
    $scope.eaifPref.classify_cv_model];

    $scope.update_target_idx = function()
    {
      $scope.eaifPref.target_idx = 
        (($scope.target_idx.dev&0xff)) |
        (($scope.target_idx.chn&0xff) << 8) |
        (($scope.target_idx.win&0xff) << 16);
    }
    $scope.load_target_idx = function()
    {
      $scope.target_idx.dev = ($scope.eaifPref.target_idx) & 0xff;
      $scope.target_idx.chn = ($scope.eaifPref.target_idx >> 8) & 0xff;
      $scope.target_idx.win = ($scope.eaifPref.target_idx >> 16) & 0xff;
    }

    $scope.range = function (min, max) {
      var input = [];
      for (var i = min; i < max; i++) {
        input.push(i);
      }
      return input;
    };

    $scope.canApply = true;

    $scope.fdStatus = {
      enabled: false,
    }

    $scope.hdStatus = {
      enabled: false,
    }

    $scope.checkFdStatus = function () {
      if ($scope.eaifPref.enabled == 1 && $scope.eaifPref.api == 'FACEDET') {
        $scope.fdStatus.enabled = true;
        console.log('Face detection is enabled');
      } else {
        $scope.fdStatus.enabled = false;
        console.log('Face detection is not enabled');
      }
    }

    $scope.checkHdStatus = function () {
      if ($scope.eaifPref.enabled == 1 && $scope.eaifPref.api == 'HUMAN_CLASSIFY') {
        $scope.hdStatus.enabled = true;
        console.log('Human detection is enabled');
      } else {
        $scope.hdStatus.enabled = false;
        console.log('Human detection is not enabled');
      }
    }

    $scope.getPdStatus = function () {
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
      }).then(function (resp) {
        console.log('post getdata success: ' + 'AGTX_CMD_PD_CONF');
        console.log(resp);
        $scope.pdPref = resp.data;
      })
    }

    $scope.checkPdStatus = function () {
      $scope.getPdStatus();
      if ($scope.pdPref.enabled == 1) {
        $scope.canApply = false;
        $scope.cmdStatus = $scope.msg_t.NG_By_PD_Enabled[$scope.lang];
        console.log('PD is enabled');
      } else {
        $scope.canApply = true;
        $scope.cmdStatus = $scope.msg_t.noTxt[$scope.lang];
        console.log('PD is not enabled');
      }
    }

    $scope.sendData = function () {
      $scope.eaifPref.cmd_type = 'set';
      if ($scope.fdStatus.enabled && window.location.href.substr(window.location.href.lastIndexOf('/') + 1) == "iva_fd.html") {
        $scope.eaifPref.enabled = 1;
        $scope.eaifPref.api = 'FACEDET';
        $scope.eaifPref.data_fmt = "MPI_RGB";
      } else if ($scope.hdStatus.enabled && window.location.href.substr(window.location.href.lastIndexOf('/') + 1) == "iva_hd.html") {
        $scope.eaifPref.enabled = 1;
        $scope.eaifPref.api = 'HUMAN_CLASSIFY';
        $scope.eaifPref.data_fmt = "MPI_Y";
      } else if ($scope.fdStatus.enabled == false && $scope.hdStatus.enabled == false) {
        $scope.eaifPref.enabled = 0;
      }
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.eaifPref, function (key, value) {
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
      console.log('get data: ' + 'AGTX_CMD_EAIF_CONF');
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
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_EAIF_CONF');
          $scope.eaifPref = resp.data;
          $scope.canApply = true;
          if ($scope.eaifPref.enabled ) {
            $scope.eaifPref.enabled = 1;
          } else {
            $scope.eaifPref.enabled = 0;
          }
          $scope.checkFdStatus();
          $scope.checkHdStatus();
          $scope.Result = resp;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          $scope.load_target_idx()
          //$scope.update_api_tag(0);
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_EAIF_CONF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
        }
      );
    };
    $scope.getPdStatus();
    $scope.getData(0);
    //$scope.update_api_tag();
  },
]);
