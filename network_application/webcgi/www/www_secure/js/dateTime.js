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
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
      date_time: ['Date & Time', '日期/時間', '日期/时间'],
      Maintenance: ['Maintenance', '維護', '维护'],
      TimeZone: [
        'Time Zone (Require manual reboot)',
        '時區 (需手動重啟)',
        '时区 (需手动重启)',
      ],
      Current_Date_Time: [
        'Device Date/Time',
        '攝像機日期/時間',
        '摄像机日期/时间',
      ],
      Load_from_Camera: ['Load from Camera', '從攝像機加載', '从摄像机加载'],
      Set_to_Camera: ['Set to Camera', '加載到攝像機', '加载到摄像机'],
      PC_Date_Time: ['PC Date/Time:', '計算機日期/時間:', '计算机日期/时间:'],
      SyncWithPC: [
        ' Sync. with computer time',
        ' 與電腦時間同步',
        ' 与电脑时间同步',
      ],
      Time_Server_IP: ['Time Server IP', '時間伺服器位址', '时间伺服器位址'],
      Interval_Update_Time: [
        ' Interval Update Time (<= 24hr)',
        ' 間隔更時間 (<= 24hr)',
        ' 间隔更新时间 (<= 24hr)',
      ],
      hr: [' hr', ' 小時', ' 小时'],
      EnableManual: [' Enable Manual Sync', ' 開啟手動同步', ' 开启手动同步'],
      EnableNTP: [' Enable NTP Sync', ' 開啟NTP同步', ' 开启NTP同步'],
      EnableDST: [
        ' Enable DST (Require manual reboot)',
        ' 開啓DST (需手動重啟)',
        ' 开启DST (需手动重启)',
      ],
      StartTime: ['Start Time', '開始時間', '开始时间'],
      EndTime: ['End Time', '結束時間', '结束时间'],
      Bias: ['Bias', '偏差', '偏差'],
      Apply: ['Save', '儲存', '储存'],
    };

    $scope.autoWhiteBalance = '1';
    $scope.redGain = 100;
    $scope.blueGain = 100;
    $scope.brightness = 100;
    $scope.contrast = 100;
    $scope.hue = 100;
    $scope.cmdStatus = $scope.msg_t.noTxt[$scope.lang];
    $scope.cmd = 'cmd';
    $scope.Result = 'result';
    $scope.SntpConf = {
      NTPserver: '',
      Interval: '',
    };
    $scope.TimeMode = {
      Manual_enabled: true,
      DST_enabled: false,
      SyncWithPC_enabled: true,
    };
    $scope.DSTSet = {
      GMT: 0,
      MonthStart: 4,
      PriorityStart: 1,
      WeekStart: 0,
      HourStart: 2,
      MonthEnd: 10,
      PriorityEnd: 2,
      WeekEnd: 0,
      HourEnd: 2,
      BiasSet: 60,
    };

    $scope.canApply = true;

    $scope.enableManual = function () {
      return $scope.TimeMode.Manual_enabled == false;
    };
    $scope.enableNTP = function () {
      return $scope.TimeMode.Manual_enabled == true;
    };
    $scope.enableDST = function () {
      return $scope.TimeMode.DST_enabled == false;
    };
    $scope.sendData = function () {
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.imgPF),
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Result = resp;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.curCamTime = ' ';
    $scope.getCamTime = function (hide_msg) {
      $scope.cmd = '/getTime.cgi';
      console.log('getTime.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.curCamTime = resp.data;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.setCamTime = function () {
      $scope.cmd = '/setTime.cgi';
      console.log('setTime.cgi');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'text/plain',
        },
        data: $scope.curCamTime,
      }).then(
        function (resp) {
          console.log('post success');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.setToPCTime = function () {
      var now = new Date();

      $scope.curCamTime =
        now.getFullYear() +
        '.' +
        (now.getMonth() + 1) +
        '.' +
        now.getDate() +
        '-' +
        now.getHours() +
        ':' +
        now.getMinutes() +
        ':' +
        now.getSeconds();
      $scope.setCamTime();
    };

    $scope.getCamTime(1);

    $scope.getCamSntpConf = function (hide_msg) {
      $scope.cmd = '/getSntpConf.cgi';
      console.log('getSntpConf.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.SntpConf = resp.data;
          if (
            $scope.SntpConf.NTPserver.length == 0 &&
            $scope.SntpConf.Interval.length == 0
          ) {
            $scope.cmdStatus = 'File: sntp.conf empty!!';
            return;
          }
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.getCamSntpConf(1);

    $scope.getEnabledConf = function (hide_msg) {
      $scope.cmd = '/getEnabledConf.cgi';
      console.log('getEnabledConf.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.TimeMode = resp.data;
          if ($scope.TimeMode.Manual_enabled == 1) {
            $scope.TimeMode.Manual_enabled = true;
          }
          if ($scope.TimeMode.Manual_enabled == 0) {
            $scope.TimeMode.Manual_enabled = false;
          }
          if ($scope.TimeMode.DST_enabled == 1) {
            $scope.TimeMode.DST_enabled = true;
            $scope.getDSTConf(1);
          }
          if ($scope.TimeMode.DST_enabled == 0) {
            $scope.TimeMode.DST_enabled = false;
            $scope.getTZ(1);
          }
          if ($scope.TimeMode.SyncWithPC_enabled == 1) {
            $scope.TimeMode.SyncWithPC_enabled = true;
          }
          if ($scope.TimeMode.SyncWithPC_enabled == 0) {
            $scope.TimeMode.SyncWithPC_enabled = false;
          }
          if (
            $scope.TimeMode.Manual_enabled.length == 0 &&
            $scope.TimeMode.DST_enabled.length == 0 &&
            $scope.TimeMode.SyncWithPC_enabled.length == 0
          ) {
            $scope.cmdStatus = 'File: timeMode.conf empty!!';
            return;
          }
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.getEnabledConf(1);

    $scope.getDSTConf = function (hide_msg) {
      $scope.cmd = '/getDSTConf.cgi';
      console.log('getDSTConf.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.DSTSet = resp.data;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.getTZ = function (hide_msg) {
      $scope.cmd = '/getTZ.cgi';
      console.log('getTZ.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.DSTSet = resp.data;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.setTimeData = function () {
      $scope.canApply = false;

      //Set Enabled Config
      $scope.cmd = '/EnabledSet.cgi';
      console.log('Assign Enabled config');
      console.log(JSON.stringify($scope.TimeMode));
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.TimeMode),
      }).then(
        function (resp) {
          console.log('post success');
          console.log(resp);
          if ($scope.TimeMode.Manual_enabled == 1) {
            if ($scope.curCamTime.length == 0) {
              $scope.cmdStatus = 'Device Time empty!!';
              $scope.canApply = true;
              return;
            }
          }
          if ($scope.TimeMode.Manual_enabled == 0) {
            if ($scope.SntpConf.NTPserver.length == 0) {
              $scope.cmdStatus = 'Time Server IP empty!!';
              $scope.canApply = true;
              return;
            }
            if ($scope.SntpConf.Interval.length == 0) {
              $scope.cmdStatus = 'Interval Update Time empty!!';
              $scope.canApply = true;
              return;
            }
            if (parseInt($scope.SntpConf.Interval) > 24) {
              $scope.cmdStatus = 'Interval Update Time should <= 24hr!!';
              $scope.canApply = true;
              return;
            }
          }
          //$scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          $scope.canApply = true;
          //$scope.Result = "Post Error";
        }
      );

      // Set TimeZone and DST
      if ($scope.TimeMode.DST_enabled == 0) {
        $scope.cmd = '/TZSet.cgi';
        console.log('Assign TZ config');
        console.log(JSON.stringify($scope.DSTSet));
        $http({
          method: 'post',
          url: $scope.cmd,
          headers: {
            'Content-Type': 'application/json',
          },
          data: JSON.stringify($scope.DSTSet),
        }).then(
          function (resp) {
            console.log('post success');
            console.log(resp);
            if ($scope.TimeMode.Manual_enabled == 1) {
              if ($scope.curCamTime.length == 0) {
                $scope.cmdStatus = 'Device Time empty!!';
                $scope.canApply = true;
                return;
              }
            }
            if ($scope.TimeMode.Manual_enabled == 0) {
              if ($scope.SntpConf.NTPserver.length == 0) {
                $scope.cmdStatus = 'Time Server IP empty!!';
                $scope.canApply = true;
                return;
              }
              if ($scope.SntpConf.Interval.length == 0) {
                $scope.cmdStatus = 'Interval Update Time empty!!';
                $scope.canApply = true;
                return;
              }
              if (parseInt($scope.SntpConf.Interval) > 24) {
                $scope.cmdStatus = 'Interval Update Time should <= 24hr!!';
                $scope.canApply = true;
                return;
              }
            }
            //$scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          },
          function (resp) {
            console.log('post error');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
            $scope.canApply = true;
            //$scope.Result = "Post Error";
          }
        );
      } else if ($scope.TimeMode.DST_enabled == 1) {
        $scope.cmd = '/DSTSet.cgi';
        console.log('Assign DST config');
        console.log(JSON.stringify($scope.DSTSet));
        $http({
          method: 'post',
          url: $scope.cmd,
          headers: {
            'Content-Type': 'application/json',
          },
          data: JSON.stringify($scope.DSTSet),
        }).then(
          function (resp) {
            console.log('post success');
            console.log(resp);
            if ($scope.TimeMode.Manual_enabled == 1) {
              if ($scope.curCamTime.length == 0) {
                $scope.cmdStatus = 'Device Time empty!!';
                $scope.canApply = true;
                return;
              }
            }
            if ($scope.TimeMode.Manual_enabled == 0) {
              if ($scope.SntpConf.NTPserver.length == 0) {
                $scope.cmdStatus = 'Time Server IP empty!!';
                $scope.canApply = true;
                return;
              }
              if ($scope.SntpConf.Interval.length == 0) {
                $scope.cmdStatus = 'Interval Update Time empty!!';
                $scope.canApply = true;
                return;
              }
              if (parseInt($scope.SntpConf.Interval) > 24) {
                $scope.cmdStatus = 'Interval Update Time should <= 24hr!!';
                $scope.canApply = true;
                return;
              }
            }
            //$scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          },
          function (resp) {
            console.log('post error');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
            $scope.canApply = true;
            //$scope.Result = "Post Error";
          }
        );
      }

      // Manual set to device time
      if (
        $scope.TimeMode.Manual_enabled == 1 &&
        $scope.TimeMode.SyncWithPC_enabled == 0
      ) {
        if ($scope.curCamTime.length == 0) {
          $scope.cmdStatus = 'Device Time empty!!';
          $scope.canApply = true;
          return;
        }
        $scope.cmd = '/setTime.cgi';
        console.log('setTime.cgi');
        $http({
          method: 'post',
          url: $scope.cmd,
          headers: {
            'Content-Type': 'text/plain',
          },
          data: $scope.curCamTime,
        }).then(
          function (resp) {
            console.log('post success');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
            $scope.canApply = true;
          },
          function (resp) {
            console.log('post error');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
            $scope.canApply = true;
            //$scope.Result = "Post Error";
          }
        );
      }

      // Manual set with computer time
      if (
        $scope.TimeMode.Manual_enabled == 1 &&
        $scope.TimeMode.SyncWithPC_enabled == 1
      ) {
        var now = new Date();
        $scope.curCamTime =
          now.getFullYear() +
          '.' +
          (now.getMonth() + 1) +
          '.' +
          now.getDate() +
          '-' +
          now.getHours() +
          ':' +
          now.getMinutes() +
          ':' +
          now.getSeconds();

        $scope.cmd = '/setTime.cgi';
        console.log('setTime.cgi');
        $http({
          method: 'post',
          url: $scope.cmd,
          headers: {
            'Content-Type': 'text/plain',
          },
          data: $scope.curCamTime,
        }).then(
          function (resp) {
            console.log('post success');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
            $scope.canApply = true;
          },
          function (resp) {
            console.log('post error');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
            $scope.canApply = true;
            //$scope.Result = "Post Error";
          }
        );
      }

      // Sync with NTP
      if ($scope.TimeMode.Manual_enabled == 0) {
        $scope.cmd = '/setSntpConf.cgi';
        console.log('Assign Sntp config');
        console.log($scope.SntpConf);
        if ($scope.SntpConf.NTPserver.length == 0) {
          $scope.cmdStatus = 'Time Server IP empty!!';
          $scope.canApply = true;
          return;
        }
        if ($scope.SntpConf.Interval.length == 0) {
          $scope.cmdStatus = 'Interval Update Time empty!!';
          $scope.canApply = true;
          return;
        }
        if (parseInt($scope.SntpConf.Interval) > 24) {
          $scope.cmdStatus = 'Interval Update Time should <= 24hr!!';
          $scope.canApply = true;
          return;
        }
        $scope.cmdStatus = 'Sync time...';
        $http({
          method: 'post',
          url: $scope.cmd,
          headers: {
            'Content-Type': 'application/json',
          },
          data: JSON.stringify($scope.SntpConf),
        }).then(
          function (resp) {
            console.log('post success');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
            $scope.canApply = true;
          },
          function (resp) {
            console.log('post error');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
            $scope.canApply = true;
            //$scope.Result = "Post Error";
          }
        );
      }
    };
  },
]);
