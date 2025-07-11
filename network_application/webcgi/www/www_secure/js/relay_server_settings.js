app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  '$sce',
  function ($scope, $location, $http, $sce) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.msg_t = {
      augentix: ['Augentix', '多方科技', '多方科技'],
      noTxt: ['', '', ''],
      Apply: ['Apply', '應用', '应用'],
      Confirm: ['Confirm', '確認', '确认'],
      Loading: ['Loading...', '讀取中...', '读取中...'],
      Processing: ['Processing...', '處理中...', '处理中...'],
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
      Get_Status: ['Get status', '取得狀態', '取得状态'],
      Get_Status_Again: ['Get status again', '重新取得狀態', '重新取得状态'],
      Register: ['Register', '註冊', '註册'],
      Unregister: ['Unregister', '註銷', '註销'],
      Start: ['Start', '啟動', '启动'],
      Stop: ['Stop', '停止', '停止'],
      Restart: ['Restart', '重啟', '重启'],
      Log_Level_Label: ['Logging level', '紀錄等級', '纪录等级'],
      Log_Levels: {
        Trace: ['Verbose', '詳細', '详细'],
        Debug: ['Debug', '除錯', '调试'],
        Info: ['Info', '資訊', '信息'],
        Warn: ['Warning', '警告', '警告'],
        Error: ['Error', '錯誤', '错误'],
      },
      Get_Logs: ['Export logs', '匯出紀錄', '导出纪录'],
      Status_Version: ['Version', '版本', '版本'],
      Status_Remote_Host: ['Registered Camera', '登錄的攝像機', '登录的摄像机'],
      Status_Remote_Host_Not_Yet_Registered: ['not yet registered', '尚未註冊', '尚未註册'],
      Status_Is_Running: ['Running status', '執行狀態', '运行状态'],
      Status_Is_Running_True: ['Running', '執行中', '运行中'],
      Status_Is_Running_False: ['Stopped', '已停止', '已停止'],
      Idle_Time_Label: ['Idle time before stop video relay (sec.)', '停止影像中繼前的閒置時間（秒）', '停止视频中继前的闲置时间（秒）'],
      Reset_Relay_Server: ['Reset relay server', '重設中繼服務器', '重设中继服务器'],
      Alert_Cannot_Detect_Relay_Server_Title: ['Cannot Detect Relay Server!', '無法偵測到中繼服務器！', '无法侦测到中继服务器！'],
      Alert_Cannot_Detect_Relay_Server_Content: [
        $sce.trustAsHtml(`
          <p>Make sure that relay server is installed and running.</p>
          <p>If you haven't install relay server, you can download the newest version from the link write down below.<br>Please check the system requirement before install.</p>
          <p>After the installation, startup relay server and check the tray icon exist on system tray.</p>
        `),
        $sce.trustAsHtml(`
          <p>請確認中繼服務器已經安裝且執行中。</p>
          <p>若中繼服務器尚未安裝，您可以從下面的連結中下載最新版的中繼服務器。<br>在安裝前請先確認系統需求。</p>
          <p>安裝完成後，請啟動中繼服務器，並確認中繼服務器的圖示有出現在系統匣上。</p>
        `),
        $sce.trustAsHtml(`
          <p>请确认中继服务器已经安装且运行中。</p>
          <p>若中继服务器尚未安装，您可以从下面的连结中下载最新版的中继服务器。<br>在安装前请先确认系统需求。</p>
          <p>安装完成后，请启动中继服务器，并确认中继服务器的图标有出现在系统托盘上</p>
        `),
      ],
      Alert_Cannot_Detect_Relay_Server_Download: ['Download relay server', '下載中繼服務器', '下载中继服务器'],
    };
    $scope.logLevels = [
      'Trace',
      'Debug',
      'Info',
      'Warn',
      'Error',
    ]

    $scope.cmd = 'cmd';
    $scope.Result = 'result';
    $scope.relayUrl = window.relayAPIOrigin;
    $scope.logUrl = $scope.relayUrl + '/log';
    $scope.downloadUrl = 'http://example.com/relay-server.html';
    $scope.cmdStatus = '';
    $scope.clearCmdStatusTimer = undefined;
    $scope.clearCmdStatus = function () {
      if ($scope.clearCmdStatusTimer)
      {
        clearTimeout($scope.clearCmdStatusTimer);
      }
      $scope.clearCmdStatusTimer = setTimeout(function () {
        $scope.cmdStatus = '';
      }, 3000);
    };
    $scope.showProcessing = function () {
      if ($scope.clearCmdStatusTimer)
      {
        clearTimeout($scope.clearCmdStatusTimer);
      }
      $scope.cmdStatus = $scope.msg_t.Processing[$scope.lang];
    };
    $scope.status = undefined;
    // $scope.status has three kinds of value witch meaning the connect state follows:
    // undefined: page loaded and not yet connected.
    // null: connection is broken/timeout.
    // JSON Object: connected. The object is looks like:
    // {
    //   "version": "1.0.0",
    //   "remoteHost": "192.168.1.100",
    //   "logLevel": "Info",
    //   "isRunning": false,
    //   "idleTime": 1
    // };
    $scope.editingIdleTime = 10;
    $scope.isIdleTimeEdited = false;
    $scope.selectedLogLevel = $scope.logLevels[2];
    $scope.isLogLevelSelected = false;
    $scope.refreshStatusTimer = undefined;
    $scope.getStatus = function () {
      if ($scope.refreshStatusTimer) {
        clearTimeout($scope.refreshStatusTimer);
      }
      // $scope.showProcessing();
      $scope.cmd = $scope.relayUrl + '/status';
      // console.log('status');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
      }).then(
        function (resp) {
          // console.log('get status success');
          // if ($scope.status == null || resp.data.remoteHost != location.hostname) {
          //   $scope.register();
          // }
          $scope.status = resp.data;
          if (!$scope.isIdleTimeEdited) {
            $scope.editingIdleTime = resp.data.idleTime;
          }
          if (!$scope.isLogLevelSelected) {
            $scope.selectedLogLevel = resp.data.logLevel;
          }
          // console.log(resp);
          // $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          // $scope.clearCmdStatus();
        },
        function (resp) {
          console.log('get status error');
          console.log(resp);
          // $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          // $scope.clearCmdStatus();
          if (resp.status == -1)
          {
            $scope.status = null;
          }
        }
      ).finally(() => { $scope.refreshStatusTimer = setTimeout($scope.getStatus, 2000) });
    };
    $scope.register = function () {
      $scope.showProcessing();
      $scope.cmd = $scope.relayUrl + '/register';
      console.log('register');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify({
          "remoteHost": location.hostname,
          "lang": $scope.lang
        }),
      }).then(
        function (resp) {
          console.log('register success');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          $scope.clearCmdStatus();
          $scope.getStatus();
        },
        function (resp) {
          console.log('register error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          $scope.clearCmdStatus();
          if (resp.status == -1)
          {
            $scope.status = null;
          }
        }
      );
    };
    $scope.unregister = function () {
      $scope.showProcessing();
      $scope.cmd = $scope.relayUrl + '/unregister';
      console.log('unregister');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
      }).then(
        function (resp) {
          console.log('unregister success');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          $scope.clearCmdStatus();
          $scope.getStatus();
        },
        function (resp) {
          console.log('unregister error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          $scope.clearCmdStatus();
          if (resp.status == -1)
          {
            $scope.status = null;
          }
        }
      );
    };
    $scope.start = function () {
      $scope.showProcessing();
      $scope.cmd = $scope.relayUrl + '/start';
      console.log('start');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
      }).then(
        function (resp) {
          console.log('start success');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          $scope.clearCmdStatus();
          $scope.getStatus();
        },
        function (resp) {
          console.log('start error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          $scope.clearCmdStatus();
          if (resp.status == -1)
          {
            $scope.status = null;
          }
        }
      );
    };
    $scope.stop = function () {
      $scope.showProcessing();
      $scope.cmd = $scope.relayUrl + '/stop';
      console.log('stop');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
      }).then(
        function (resp) {
          console.log('stop success');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          $scope.clearCmdStatus();
          $scope.getStatus();
        },
        function (resp) {
          console.log('stop error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          $scope.clearCmdStatus();
          if (resp.status == -1)
          {
            $scope.status = null;
          }
        }
      );
    };
    $scope.restart = function () {
      $scope.showProcessing();
      $scope.cmd = $scope.relayUrl + '/restart';
      console.log('restart');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
      }).then(
        function (resp) {
          console.log('restart success');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          $scope.clearCmdStatus();
          $scope.getStatus();
        },
        function (resp) {
          console.log('restart error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          $scope.clearCmdStatus();
          if (resp.status == -1)
          {
            $scope.status = null;
          }
        }
      );
    };
    $scope.setLogLevel = function () {
      $scope.showProcessing();
      $scope.cmd = $scope.relayUrl + '/log-level';
      console.log('set log level');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.selectedLogLevel),
      }).then(
        function (resp) {
          console.log('set log-level success');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          $scope.clearCmdStatus();
          $scope.getStatus();
        },
        function (resp) {
          console.log('set log-level error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          $scope.clearCmdStatus();
          if (resp.status == -1)
          {
            $scope.status = null;
          }
        }
      );
    };
    $scope.setIdleTime = function () {
      $scope.showProcessing();
      $scope.cmd = $scope.relayUrl + '/idle-time';
      console.log('set idle-time');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify(parseInt($scope.editingIdleTime)),
      }).then(
        function (resp) {
          console.log('set idle-time success');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          $scope.clearCmdStatus();
          $scope.getStatus();
        },
        function (resp) {
          console.log('set idle-time error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          $scope.clearCmdStatus();
          if (resp.status == -1)
          {
            $scope.status = null;
          }
        }
      );
    }
    $scope.confirmResetConfig = false;
    $scope.preResetConfig = function () {
      $scope.confirmResetConfig = !$scope.confirmResetConfig;
    };
    $scope.resetConfig = function() {
      $scope.confirmResetConfig = false;
      $scope.showProcessing();
      $scope.cmd = $scope.relayUrl + '/reset';
      console.log('reset config');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
      }).then(
        function (resp) {
          console.log('reset config success');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          $scope.clearCmdStatus();
          $scope.getStatus();
        },
        function (resp) {
          console.log('reset config error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          $scope.clearCmdStatus();
          if (resp.status == -1)
          {
            $scope.status = null;
          }
        }
      );
    };

    $scope.getStatus();
  },
]);
