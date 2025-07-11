app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.AudioMenu = setAudioPage(Audio_Index.audio);
    $scope.CurPage = msg.AudioSetting;
    $scope.msg_t = {
      augentix: ['Augentix', '多方科技', '多方科技'],
      noTxt: ['', '', ''],
      Read_setting: ['Read out from device', '讀取設定', '读取设定'],
      Apply: ['Apply', '應用', '应用'],
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      NoRespond: ['Device no respond!!', '設備無反應!!', '设备无反应!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
      EnableAudio: ['Enable Audio', '開啓音頻', '开启音频'],
      ExternalMICGain: [
        'External MIC gain',
        '麥克風外部增益',
        '麦克风外部增益',
      ],
      AudioCodec: ['Audio CODEC', '音頻編碼', '音频编码'],
      AudioSetting: ['Audio Setting', '音頻設置', '音频设置'],
      samplingfreq: ['sampling frequency', '取樣頻率', '取样频率'],
      samplingbit: ['sampling bit', '取樣大小', '取样大小'],
    };

    $scope.AudioSet = {
      master_id: 11,
      enabled: 1,
      cmd_id: Cmd.AGTX_CMD_AUDIO_CONF,
      cmd_type: 'set',
      gain: 1,
      sampling_bit: 16,
      sampling_frequency: 8000,
      codec: 'ALAW',
    };

    $scope.cmd = 'cmd';
    $scope.Result = 'result';
    $scope.enableAudio = function () {
      return $scope.AudioSet.enabled == 0;
    };

    $scope.enableSampleBit = function () {
      return $scope.AudioSet.enabled == 0 || $scope.AudioSet.codec != 'G726';
    };

    $scope.sendData = function () {
      $scope.AudioSet.cmd_type = 'set';
      $scope.cmd = '/cgi-bin/msg.cgi';

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.AudioSet),
      }).then(
        function (resp) {
          console.log('post success: ' + 'AGTX_CMD_AUDIO_CONF');
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
          console.log('post error' + 'AGTX_CMD_AUDIO_CONF');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          //	$scope.Result = "Post Error";
        }
      );
    };
    $scope.getData = function (hide_msg) {
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
          "cmd_id": Cmd.AGTX_CMD_AUDIO_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success' + 'AGTX_CMD_AUDIO_CONF');
          $scope.AudioSet = resp.data;
          $scope.Result = resp;
          $scope.AudioSet.enabled = $scope.AudioSet.enabled == 1;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error' + 'AGTX_CMD_AUDIO_CONF');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getData(1);
  },
]);
