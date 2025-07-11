app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    /* $scope.IvaDropDown = setIvaDropdown(Iva_Index.td); */
    $scope.AudioMenu = setAudioPage(Audio_Index.lsd);
    $scope.CurPage = msg.Loud_Sound_Detection;
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
      Loud_Sound_Detection: [
        'Loud Sound Detection',
        '突發噪音檢測',
        '突发噪音检测',
      ],
      Enable_loud_sound_detection: [
        'Enable loud sound detection',
        '開啓突發噪音檢測',
        '开启突发噪音检测',
      ],
      Volume: [
        'Minimal trigger volume(dB):',
        '最小觸發音量(dB):',
        '最小触发音量(dB):',
      ],
      Duration: [
        'Maximal trigger duration(s):',
        '最大觸發持續時間(秒):',
        '最大触发持续时间(秒):',
      ],
      Suppression: ['Suppression(s):', '抑制時間(秒):', '抑制时间(秒):'],
      DurInvalid: [
        'Duration should between 0 ~ 100 and maximum number of decimal digits is 3!!',
        '最大觸發持續時間應介於0到100之間且小數位最多三位!!',
        '最大触发持续应介于0到100之间且小数位最多三位!!',
      ],
      SupInvalid: [
        'Suppression should between 0 ~ 100 and maximum number of decimal digits is 3!!',
        '抑制時間應介於0到100之間且小數位最多三位!!',
        '抑制时间应介于0到100之间且小数位最多三位!!',
      ],
    };

    $scope.lsdPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_LSD_CONF,
      cmd_type: 'set',
      enabled: 0,
      audio_dev_idx: 0,
      volume: 95,
      duration: 1.0,
      suppression: 2.0,
    };
    $scope.Result = 'result';

    $scope.isValidDurationValue = function () {
      //duration should not bigger than 100 & smaller than 0
      if (
        $scope.lsdPref.duration < 0 ||
        $scope.lsdPref.duration > 100 ||
        typeof $scope.lsdPref.duration != 'number'
      ) {
        console.log('duration: ' + $scope.lsdPref.duration);
        $scope.cmdStatus = $scope.msg_t.DurInvalid[$scope.lang];
        return false;
      }
      return true;
    };

    $scope.isValidSuppressionValue = function () {
      //suppression should not bigger than 100 & smaller than 0
      if (
        $scope.lsdPref.suppression < 0 ||
        $scope.lsdPref.suppression > 100 ||
        typeof $scope.lsdPref.suppression != 'number'
      ) {
        console.log('suppression: ' + $scope.lsdPref.suppression);
        $scope.cmdStatus = $scope.msg_t.SupInvalid[$scope.lang];
        return false;
      }
      return true;
    };

    $scope.sendData = function () {
      if (!$scope.isValidDurationValue()) {
        console.log('Wrong duration value');
        return;
      }

      if (!$scope.isValidSuppressionValue()) {
        console.log('Wrong suppression value');
        return;
      }

      $scope.lsdPref.cmd_type = 'set';
      if ($scope.lsdPref.enabled) $scope.lsdPref.enabled = 1;
      else $scope.lsdPref.enabled = 0;
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.lsdPref.enabled = $scope.lsdPref.enabled == 1;

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.lsdPref),
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
          "cmd_id": Cmd.AGTX_CMD_LSD_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_LSD_CONF');
          resp.data.duration = resp.data.duration.toFixed(3);
          resp.data.suppression = resp.data.suppression.toFixed(3);
          resp.data.duration = parseFloat(resp.data.duration);
          resp.data.suppression = parseFloat(resp.data.suppression);
          $scope.lsdPref = resp.data;
          $scope.canApply = true;
          $scope.Result = resp;
          $scope.lsdPref.enabled = $scope.lsdPref.enabled == 1;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_LSD_CONF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
        }
      );
    };
    $scope.getData(1);
  },
]);
