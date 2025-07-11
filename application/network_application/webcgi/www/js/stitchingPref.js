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
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      NoRespond: ['Device no respond!!', '設備無反應!!', '设备无反应!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
      Image_Preference: ['Image Preference', '圖像設置', '图像设置'],
      AWB_Preference: ['AWB Preference', '白平衡設置', '白平衡设置'],
      Stitching: ['Stitching', '畫面縫合', '画面缝合'],
      Advanced_Setting: ['Advanced Setting', '高級設置', '高级设置'],
      Stitching_distance: ['Stitching distance', '縫合距離', '缝合距离'],
    };

    var vlc;
    var itemId;
    $scope.canApply = true;

    $scope.stitchingPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_STITCH_CONF,
      cmd_type: 'set',
      video_dev_idx: 0,
      /* 定值 */
      dft_dist: 0,
      /* 連結到stitching distance */
    };
    $scope.Yxxx = 333;
    $scope.cmd = 'cmd';
    $scope.Result = 'result';

    $scope.sendData = function () {
      $scope.stitchingPref.cmd_type = 'set';
      $scope.stitchingPref.dft_dist = Number($scope.stitchingPref.dft_dist);
      $scope.cmd = '/cgi-bin/msg.cgi';
      var postData = JSON.stringify($scope.stitchingPref);
      $scope.stitchingPref.dft_dist = $scope.stitchingPref.dft_dist.toString();

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: postData,
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
          "cmd_id": Cmd.AGTX_CMD_STITCH_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_STITCH_CONF');
          $scope.stitchingPref1 = resp.data;
          $scope.stitchingPref.dft_dist = $scope.stitchingPref1.dft_dist.toString();
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
          console.log('post error: ' + 'AGTX_CMD_STITCH_CONF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getData(1);
    window.langId = $scope.lang;
  },
]);
