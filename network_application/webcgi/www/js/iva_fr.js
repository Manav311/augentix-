app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  '$q',
  function ($scope, $location, $http, $q) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.IvaDropDown = setIvaDropdown(Iva_Index.facereco)
    //$scope.IvaMenu = setIvaPage(Iva_Index.od);
    $scope.CurPage = msg.Face_Recognition;
    //$scope.CurPage = msg.Object_Detection;
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
      Enable_Face_Reco: [
        'Enable Face Recognition',
        '開啟臉部辨識',
        '开启脸部辨识',
      ],
      RegisterBySnapshot: ['Register by Snapshot', '使用快照註冊', '使用快照註册'],
      RegisterByUpload: [
        'Register by exist face photo (up to 320x320 pixel)',
        '使用已有的人臉照註冊 (上限為 320x320 pixel)',
        '使用已有的人脸照註册 (上限为 320x320 pixel)'
      ],
      SelectFile: ['Select File', '選擇檔案', '选择档案'],
      EnterFacePhotoName: ['Enter the Name', '輸入名稱', '输入名称'],
      Upload: ['Upload', '上傳', '上传'],
      PhotoUploading: ['Uploading', '上傳中', '上传中'],
      UploadOK: ['Upload success!! ', '上傳成功!!', '上传成功'],
      UploadFail: ['Upload failed!! ', '上傳失敗!!', '上传失败'],
      PhotoValidating: ['Validating Photo', '檔案確認中', '档案确认中'],
      ValidationPass: ['This photo is a valid Face model. Start registration...', '此檔案是合格的人臉模型. 開始註冊...', '此档案是合格的人脸模型. 开始註册...'],
      ValidationFailed: ['This photo is not a valid Face model!!', '此檔案不是合格的人臉模型!!', '此档案是不合格的人脸模型!!'],
      RegisterPass: ['Registration success!!', '註冊成功!!', '註册成功!!'],
      RegisterFail: ['Registration fail!!', '註冊失敗!!', '註册失敗!!'],
      ShowExample: ['Show exmaple', '顯示範例', '显示范例'],
      HideExample: ['Hide exmaple', '隱藏範例', '隐藏范例'],
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
      Detect: ['Detect', '物體檢測', '物体检测'],
      Classify: ['Classify', '物體分類', '物体分类'],
      Classify_cv: ['Classify(CV)', '物體分類(CV)', '物体分类(CV)'],
      Face_reco: ['Face Recognition', '臉部辨識', '脸部辨识'],
      Human_classify: ['Human Classify', '人形分類', '人形分类'],
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
      NG_By_PD_Enabled: [
        'Can not enable Edge AI Framework since Pedestrain Detecction is already enabled!!',
        '無法開啟物體檢測,因為行人檢測已經開啟了!!',
        '无法开启物体检测,因为行人检测已经开启了!!',
      ],
      Preview: ['Preview', '預覽', '预览'],
      CaptureImage: ['Capture', '截圖', '截图'],
      Recording: ['Recording', '錄影', '录制'],
    };

    $scope.faceStatus = {
      enabled: false,
    }

    $scope.eaifPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_EAIF_CONF,
      cmd_type: 'set',
      api:"FACERECO",
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

    $scope.faceModuleData = {
      model: null,
      FaceModelList: ["default_0.jpg","default_1.jpg",],
    };

    $scope.showHideTrigger = false;
    $scope.photoName = 'test.jpg';
    $scope.captureName = 'test.jpg';
    $scope.selectedPhoto = '';
    $scope.selectedPhotoImage = '';
    $scope.removePhotoName = '';
    $scope.canApply = true;
    $scope.canUpload = true;
    $scope.showFaceExample = false;

    $scope.uploadPhoto = function() {
      $scope.canUpload = false;
      $scope.cmdStatus = $scope.msg_t.PhotoUploading[$scope.lang];
      console.log('Uploading face photo');
      var file = $scope.faceFile;
      console.log('file is ');
      console.dir(file);
      var uploadUrl = "/uploadFacePhoto.cgi?" + $scope.photoName;
      var fd = new FormData();
      fd.append('file', file);
      console.log(fd);
      $http.post(uploadUrl, fd, {
        transformRequest: angular.identity,
        headers: {
          'Content-Type': 'text/plain'
        }
      })
        .then(function(resp) {
          $scope.cmdStatus = $scope.msg_t.UploadOK[$scope.lang];
          console.log($scope.faceModuleData);
          $scope.validateFaceModel();
        },
        function(resp) {
          $scope.cmdStatus = $scope.msg_t.UploadFail[$scope.lang];
          $scope.canUpload = true;
        });
    };

    $scope.uploadCaptured = function() {
      document.querySelector('#captureFaceBtn').click();
      $scope.cmdStatus = $scope.msg_t.PhotoUploading[$scope.lang];
      $scope.photoName = $scope.captureName;
      console.log('Uploading face photo');
      var file = capturedImage;
      console.log('file is ');
      console.dir(file);
      var uploadUrl = "/uploadFacePhoto.cgi?" + $scope.captureName;
      var fd = new FormData();
      fd.append('file', file);
      console.log(fd);
      $http.post(uploadUrl, fd, {
        transformRequest: angular.identity,
        headers: {
          'Content-Type': 'text/plain'
        }
      })
        .then(function(resp) {
          $scope.cmdStatus = $scope.msg_t.UploadOK[$scope.lang];
          console.log($scope.faceModuleData);
          $scope.validateFaceModel();
        },
        function(resp) {
          $scope.cmdStatus = $scope.msg_t.UploadFail[$scope.lang];
          $scope.canUpload = true;
        });
    };

    $scope.validateFaceModel = function () {
      $scope.cmdStatus = $scope.msg_t.PhotoValidating[$scope.lang];
      console.log('Validating face photo');
      $scope.cmd = '/validateFaceModel.cgi?' + $scope.photoName;
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
          'Cache-Control': 'no-cache',
        },
      }).then(
        function (resp) {
          if (resp.data.rval == 0) {
            console.log('Validation face model pass');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.ValidationPass[$scope.lang];
            $scope.registerFaceModel();
          } else {
            console.log('Validation face model faild');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.ValidationFailed[$scope.lang];
            $scope.removePhotoName = $scope.photoName;
            $scope.removePhoto(1);
            $scope.canUpload = true;
          }
        },
        function (resp) {
          console.log('post error: ' + $scope.cmd);
          console.log(resp);
          $scope.cmdStatus = "Validate face model error!";
          $scope.canUpload = true;
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.registerFaceModel = function () {
      console.log($scope.selectedPhoto);
      $scope.cmd = '/registerFaceModel.cgi?' + $scope.photoName;
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
          'Cache-Control': 'no-cache',
        },
      }).then(
        function (resp) {
          console.log('Register face model success');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.RegisterPass[$scope.lang];
          $scope.getFaceModelList();
          $scope.setSelectedPhoto($scope.photoName);
          $scope.setSelectedName();
          $scope.canUpload = true;
        },
        function (resp) {
          console.log('post error: ' + $scope.cmd);
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.RegisterFail[$scope.lang];
          $scope.canUpload = true;
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.removePhoto = function (hide_msg) {
      console.log( 'Remove file name is :' + $scope.removePhotoName);
      $scope.cmd = '/removeFacePhoto.cgi?' + $scope.removePhotoName;
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
          'Cache-Control': 'no-cache',
        },
      }).then(
        function (resp) {
          console.log('Remove face model success');
          console.log(resp);
          $scope.getFaceModelList();
          if (hide_msg != 1) {
            $scope.cmdStatus = "Remove face model success!";
          } else {
          }
        },
        function (resp) {
          console.log('post error: ' + $scope.cmd);
          console.log(resp);
          $scope.cmdStatus = "Remove face model error!";
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.unregisterFaceModel = function () {
      if (!$scope.selectedPhoto || $scope.selectedPhoto.length === 0) {
        console.log("No face module is selected!!");
        $scope.cmdStatus = "No face module is selected!!";
        return;
      }
      console.log($scope.selectedPhoto);
      $scope.cmdStatus = "Start removing face photos ...";
      $scope.selectedPhoto.forEach(function(photo) {
        $scope.cmd = '/unregisterFaceModel.cgi?' + photo;
        $http({
          method: 'get',
          url: $scope.cmd,
          headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
            'Cache-Control': 'no-cache',
          },
        }).then(
          function (resp) {
            console.log('Unregister face model success');
            console.log(resp);
            $scope.removePhotoName = photo;
            $scope.removePhoto();
          },
          function (resp) {
            console.log('post error: ' + $scope.cmd);
            console.log(resp);
            $scope.cmdStatus = "Unregister face model error!";
            //$scope.Result = "Post Error";
          }
        );
      });
    };

    $scope.setSelectedName = function () {
      var random = (new Date()).toString();
      $scope.selectedPhotoImage = $scope.selectedPhoto + "?cb=" + random;
    }

    $scope.setSelectedPhoto = function(photoName) {
      if (!Array.isArray(photoName)) {
        $scope.selectedPhoto = [photoName]; //set to array to prevent treat by char
      } else {
        $scope.selectedPhoto = photoName;
    }
};
    $scope.getFaceModelList = function () {
      var defered = $q.defer();
      console.log($scope.faceModuleData);
      $scope.cmd = '/getFaceModelList.cgi';
      console.log('Get Face Model List');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
          'Cache-Control': 'no-cache',
        },
      }).then(
        function (resp) {
          console.log('post face model list success');
          console.log(resp);
          $scope.faceModuleData.FaceModelList = resp.data;
          $scope.Result = resp;
          defered.resolve(resp);
          //$scope.cmdStatus = "Get Face Model List success!";
        },
        function (resp) {
          console.log('post error: ' + $scope.cmd);
          console.log(resp);
          $scope.cmdStatus = 'Get Face Model List error!';
          defered.reject(resp);
          //$scope.Result = "Post Error";
        }
      );
      return defered.promise;
    };

    $scope.precheckUpload = function () {
      $scope.getFaceModelList().then(
        function () {
          if ($scope.isOverModuleLimitation() == true){
            $scope.canUpload = false;
            return;
          } else if ($scope.isOverModuleLimitation() == false){
            $scope.uploadPhoto();
          } else {
            console.log('Function isOverModuleLimitation return unexpected value!');
          }
        }
      )
    }

    $scope.precheckCapture = function () {
      $scope.getFaceModelList().then(
        function () {
          if ($scope.isOverModuleLimitation() == true){
            $scope.canUpload = false;
            return;
          } else if ($scope.isOverModuleLimitation() == false){
            $scope.uploadCaptured ();
          } else {
            console.log('Function isOverModuleLimitation return unexpected value!');
          }
        }
      )
    }


    $scope.isOverModuleLimitation = function () {
      var faceModuleMax = 40;
      if ($scope.faceModuleData.FaceModelList.length >= faceModuleMax) {
        console.log('Face module number over limitation!');
        $scope.cmdStatus = 'Face Model is up to ' + faceModuleMax + ' modules!';
        return true;
      } else {
        console.log('Face module number does not over limitation.');
        return false;
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

    $scope.checkFaceStatus = function () {
      if ($scope.eaifPref.enabled == 1 && $scope.eaifPref.api == 'FACERECO') {
        $scope.faceStatus.enabled = true;
        console.log('Face reco is enabled');
      } else {
        $scope.faceStatus.enabled = false;
        console.log('Face reco is not enabled');
      }
    }

    $scope.sendData = function () {
      $scope.eaifPref.cmd_type = 'set';
      if ($scope.faceStatus.enabled) {
        $scope.eaifPref.enabled = 1;
        $scope.eaifPref.api = 'FACERECO';
        $scope.eaifPref.data_fmt = "MPI_RGB";
      } else {
        $scope.eaifPref.enabled = 0;
      }
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log($scope.eaifPref);
      $scope.eaifPref.enabled = $scope.eaifPref.enabled == 1;
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
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.getData = function (hide_msg) {
      var defered = $q.defer();
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
          $scope.checkFaceStatus();
          $scope.Result = resp;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          $scope.load_target_idx()
          console.log(resp);
          defered.resolve(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_EAIF_CONF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          defered.reject(resp);
        }
      );
      return defered.promise;
    };

    $scope.getPdStatus();
    $scope.getData(0).then($scope.getFaceModelList()).then($scope.isOverModuleLimitation);
    $scope.setSelectedPhoto($scope.faceModuleData.FaceModelList[0]);
    window.langId = $scope.lang;
  }
]);

function upload_check()
{
    var upl = document.getElementById("file_id");
    var reader = new FileReader();
    reader.readAsDataURL(upl.files[0]);
    reader.onload = function (e) {
      //Initiate the JavaScript Image object.
      var image = new Image();

      //Set the Base64 string return from FileReader as source.
      image.src = e.target.result;

      //Validate the File Height and Width.
      image.onload = function () {
        var height = this.height;
        var width = this.width;
        if (height > 320 || width > 320) {
          alert("Height and Width must not exceed 320px.");
          upl.value = "";
          angular.element(upl).scope().$apply(function(scope) {
            scope.canUpload = false; // Disable the upload button
          });
          return false;
        }
		angular.element(upl).scope().$apply(function(scope) {
          scope.canUpload = true; // Enable the upload button
        });
        return true;
      };
    }
};

app.directive('fileModel', [
  '$parse',
  function ($parse) {
    return {
      restrict: 'A',
      link: function (scope, element, attrs) {
        var model = $parse(attrs.fileModel);
        var modelSetter = model.assign;

        element.bind('change', function () {
          scope.$apply(function () {
            modelSetter(scope, element[0].files[0]);
          });
        });
      },
    };
  },
]);
