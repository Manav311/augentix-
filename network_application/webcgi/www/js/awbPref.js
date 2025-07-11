app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.msg_t = {
      augentix: ['Augentix', '多方科技', '多方科技'],
      noTxt: ['', '', ''],
      Auto: ['Auto', '自動', '自动'],
      Read_setting: ['Read out from device', '讀取設定', '读取设定'],
      Apply: ['Apply', '應用', '应用'],
      Reboot: ['Reboot', '重新開機', '重新开机'],
      Loading: ['Loading...', '讀取中...', '读取中...'],
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      NoRespond: ['Device no respond!!', '設備無反應!!', '设备无反应!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
      Image_Preference: ['Image Preference', '圖像設置', '图像设置'],
      AWB_Preference: ['AWB Preference', '白平衡設置', '白平衡设置'],
      Stitching: ['Stitching', '畫面縫合', '画面缝合'],
      Advanced_Setting: ['Advanced Setting', '高級設置', '高级设置'],
      White_Balance: ['White Balance', '白平衡', '白平衡'],
      Auto_Tracking_White_Balance: [
        'Auto Tracking White Balance',
        '自動白平衡',
        '自动白平衡',
      ],
      Manual_White_Balance: [
        'Manual White Balance',
        '手動白平衡',
        '手动白平衡',
      ],
      Red_Gain: ['Red Gain', '紅色增益', '红色增益'],
      Blue_Gain: ['Blue Gain', '藍色增益', '蓝色增益'],
    };

    var vlc;
    var itemId;
    
    $scope.awbPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_AWB_PREF,
      cmd_type: 'set',
      mode: 0, //"manual"
      color_temp: 2, //"5000K"
      r_gain: 50,
      b_gain: 50,
    };
    $scope.Yxxx = 333;
    $scope.cmd = 'cmd';
    $scope.Result = 'result';
    $scope.AutoMode = function () {
      return $scope.awbPref.mode == 1;
    };

    $scope.Option = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_SYS_FEATURE_OPTION,
      cmd_type: 'reply',
      rval: 0,
      stitch_support: 0,
    };
    //$scope.Option.rval = 1
    $scope.Disrval = function () {
      if ($scope.Option.stitch_support == 1) return false;
      else return true;
    };
    
    $scope.sendData = function () {
      $scope.awbPref.cmd_type = 'set';
      $scope.cmd = '/cgi-bin/msg.cgi';

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.awbPref),
      }).then(
        function (resp) {
          console.log('post success: ' + 'AGTX_CMD_AWB_PREF');
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
          console.log('post error: ' + 'AGTX_CMD_AWB_PREF');
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
          "cmd_id": Cmd.AGTX_CMD_AWB_PREF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_AWB_PREF');
          $scope.awbPref = resp.data;
          $scope.canApply = true;
          $scope.Result = resp;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_AWB_PREF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.getOption = function () {
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('Get option name');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
          'Cache-Control': 'no-cache',
        },
        data: JSON.stringify({
          "master_id": 1,
          "cmd_id": Cmd.AGTX_CMD_SYS_FEATURE_OPTION,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post success: ' + 'AGTX_CMD_SYS_FEATURE_OPTION');
          $scope.Option = resp.data;
          $scope.Option.stitch_support = Number(resp.data.stitch_support);
          $scope.Disrval();
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_SYS_FEATURE_OPTION');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getOption();
    $scope.getData(1);
    window.langId = $scope.lang;
  },
]);
