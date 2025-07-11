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
      Loading: ['Loading...', '讀取中...', '读取中...'],
      Reboot: ['Reboot', '重新開機', '重新开机'],
      Switch2SysupdOS: ['System Update', '系統更新', '系统更新'],
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      NoRespond: ['Device no respond!!', '設備無反應!!', '设备无反应!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
      OSD_Setting: ['OSD Setting', 'OSD設置', 'OSD设置'],
      Device_name: ['Device name', '設備名稱', '设备名称'],
      Date_format: ['Date format', '日期格式', '日期格式'],
      Logo: ['Logo', '圖標', '图标'],
      Iva_alarm: ['Iva alarm', '智能警報', '智能警报'],
      Visable_ASCII_characters: [
        'Except for single quotes and percentages',
        '除百分號和單引號ASCII可見字元',
        '除百分号和单引号ASCII可見字元',
      ],
      Privacy_Mask: ['Privacy Mask', '隱私遮罩', '隐私遮罩'],
      Invalid_region_cnt: [
        'Enabled OSD region number in OSD setting & Privacy Mask is more than 4!!',
        '開啟的OSD區域數在OSD設置與隱私遮罩總和超過四個!!',
        '开启的OSD区域数在OSD设置与隐私遮罩总和超过四个!!',
      ],
    };

    $scope.updateFormatFromString = function (stringFormat) {
      var formats = stringFormat.split(' ');
      if (formats[0] == 'YYYY-MM-DD') {
        $scope.type_spec.date = 0;
      } else if (formats[0] == 'MM-DD-YYYY') {
        $scope.type_spec.date = 1;
      } else if (formats[0] == 'DD-MM-YYYY') {
        $scope.type_spec.date = 2;
      } else {
        $scope.type_spec.date = 0;
      }
      if (formats.length == 1) {
        $scope.type_spec.time = 4;
      } else {
        if (formats.length == 2) {
          if (formats[1] == 'H:mm:ss') {
            $scope.type_spec.time = 2;
          } else if (formats[1] == 'HH:mm:ss') {
            $scope.type_spec.time = 3;
          } else {
            $scope.type_spec.time = 3;
          }
        } else if (formats.length == 3) {
          if (formats[2] == 'tt' && formats[1] == 'h:mm:ss') {
            $scope.type_spec.time = 0;
          } else if (formats[2] == 'tt' && formats[1] == 'hh:mm:ss') {
            $scope.type_spec.time = 1;
          } else {
            $scope.type_spec.time = 3;
          }
        } else {
          $scope.type_spec.time = 3;
        }
      }
    };

    $scope.updateDateTimeFormat = function () {
      var date_format, time_format;
      if ($scope.type_spec.date == 0) {
        date_format = 'YYYY-MM-DD';
      } else if ($scope.type_spec.date == 1) {
        date_format = 'MM-DD-YYYY';
      } else if ($scope.type_spec.date == 2) {
        date_format = 'DD-MM-YYYY';
      }
      if ($scope.type_spec.time == 0) {
        time_format = 'h:mm:ss tt';
      } else if ($scope.type_spec.time == 1) {
        time_format = 'hh:mm:ss tt';
      } else if ($scope.type_spec.time == 2) {
        time_format = 'H:mm:ss';
      } else if ($scope.type_spec.time == 3) {
        time_format = 'HH:mm:ss';
      }
      $scope.osdPref.strm[0].region[1].type_spec =
        date_format + ' ' + time_format;
    };

    $scope.type_spec = {
      date: 0,
      time: 0,
    };

    $scope.osdPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_OSD_CONF,
      cmd_type: 'set',
      strm: [
        /*我是註解, 這是第1路video streaming*/
        {
          region: [
            {
              enabled: 1,
              start_x: 96,
              start_y: 1,
              type: 'TEXT',
              type_spec: 'Augentix',
            },
            {
              enabled: 1,
              start_x: 89,
              start_y: 95,
              type: 'INFO',
              type_spec: 'YYYY-MM-DD HH:mm:ss',
            },
            {
              enabled: 0,
              start_x: 1,
              start_y: 1,
              type: 'IMAGE',
              type_spec: '/system/mpp/font/LOGO_Augentix_v2.imgayuv',
            },
            {},
          ],
        },

        /*我是註解, 這是第2路video streaming*/
        {
          region: [
            {
              enabled: 1,
              start_x: 96,
              start_y: 1,
              type: 'TEXT',
              type_spec: 'Augentix',
            },
            {
              enabled: 1,
              start_x: 89,
              start_y: 95,
              type: 'INFO',
              type_spec: 'YYYY-MM-DD HH:mm:ss',
            },
            {
              enabled: 0,
              start_x: 1,
              start_y: 1,
              type: 'IMAGE',
              type_spec: '/system/mpp/font/LOGO_Augentix_v2.imgayuv',
            },
            {},
          ],
        },

        /*我是註解, 這是第3路video streaming, 但沒有設定 */
        {
          region: [
            {
              enabled: 1,
              start_x: 96,
              start_y: 1,
              type: 'TEXT',
              type_spec: 'Augentix',
            },
            {
              enabled: 1,
              start_x: 89,
              start_y: 95,
              type: 'INFO',
              type_spec: 'YYYY-MM-DD hh:mm:ss',
            },
            {
              enabled: 0,
              start_x: 1,
              start_y: 1,
              type: 'IMAGE',
              type_spec: '/system/mpp/font/LOGO_Augentix_v2.imgayuv',
            },
            {},
          ],
        },

        /*我是註解, 這是第4路video streaming, 但沒有設定 */
        {
          region: [
            {
              enabled: 1,
              start_x: 96,
              start_y: 1,
              type: 'TEXT',
              type_spec: 'Augentix',
            },
            {
              enabled: 1,
              start_x: 89,
              start_y: 95,
              type: 'INFO',
              type_spec: 'YYYY-MM-DD %H:%m:%s',
            },
            {
              enabled: 0,
              start_x: 1,
              start_y: 1,
              type: 'IMAGE',
              type_spec: '/system/mpp/font/LOGO_Augentix_v2.imgayuv',
            },
            {},
          ],
        },
      ],
    };

    $scope.osd_pmPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_OSD_PM_CONF,
      cmd_type: 'set',
      conf: [
        /*我是註解, 這是第1路video streaming*/
        {
          param: [
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
          ],
        },

        /*我是註解, 這是第2路video streaming*/
        {
          param: [
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
          ],
        },

        /*我是註解, 這是第3路video streaming, 但沒有設定 */
        {
          param: [
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
          ],
        },

        /*我是註解, 這是第4路video streaming, 但沒有設定 */
        {
          param: [
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
          ],
        },
      ],
    };

    $scope.Result = 'result';
    var vlc;
    var itemId;

    $scope.canApply = true;

    $scope.isValidRegionCnt = function () {
      var i;
      var enabled_cnt = 0;

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
          "cmd_id": Cmd.AGTX_CMD_OSD_PM_CONF,
          "cmd_type": "get",
        }),
      }).then(function (resp) {
        console.log('post getdata success: ' + 'AGTX_CMD_OSD_PM_CONF');
        console.log(resp);

        $scope.osd_pmPref = resp.data;
      });

      for (i = 0; i < $scope.osd_pmPref.conf[0].param.length; i++) {
        if ($scope.osd_pmPref.conf[0].param[i].enabled) {
          enabled_cnt++;
        }
      }

      for (i = 0; i < $scope.osdPref.strm[0].region.length; i++) {
        if ($scope.osdPref.strm[0].region[i].enabled) {
          enabled_cnt++;
        }
      }

      console.log('Total enabled count: ' + enabled_cnt);

      if (enabled_cnt <= 4) {
        $scope.canApply = true;
        $scope.cmdStatus = $scope.msg_t.noTxt[$scope.lang];
        console.log('Vaild enable number');
      } else {
        $scope.canApply = false;
        $scope.cmdStatus = $scope.msg_t.Invalid_region_cnt[$scope.lang];
        console.log('Invaild enable number');
      }
    };

    $scope.sendData = function () {
      $scope.osdPref.cmd_type = 'set';
      var i, j;
      for (i = 0; i < $scope.osdPref.strm.length; i++) {
        for (j = 0; j < $scope.osdPref.strm[i].region.length; j++) {
          if ($scope.osdPref.strm[i].region[j].enabled)
            $scope.osdPref.strm[i].region[j].enabled = 1;
          else $scope.osdPref.strm[i].region[j].enabled = 0;
        }
      }
      /* idx 0 - Text Device Name */
      /* idx 1 - Date Time */
      /* idx 2 - Logo Image */
      /* idx 3 - Iva Alarm */
      $scope.osdPref.strm[1].region[0].type_spec =
        $scope.osdPref.strm[0].region[0].type_spec; //device name
      $scope.osdPref.strm[1].region[1].type_spec =
        $scope.osdPref.strm[0].region[1].type_spec; //time format
      $scope.osdPref.strm[2].region[0].type_spec =
        $scope.osdPref.strm[0].region[0].type_spec; //device name
      $scope.osdPref.strm[2].region[1].type_spec =
        $scope.osdPref.strm[0].region[1].type_spec; //time format
      for (j = 1; j < 3; j++) {
        for (i = 0; i < 4; i++) {
          $scope.osdPref.strm[j].region[i].start_x =
            $scope.osdPref.strm[0].region[i].start_x; //broadcast loc info to channel1/2
          $scope.osdPref.strm[j].region[i].start_y =
            $scope.osdPref.strm[0].region[i].start_y; //broadcast loc info to channel1/2
          $scope.osdPref.strm[j].region[i].enabled =
            $scope.osdPref.strm[0].region[i].enabled;
        }
      }

      for (i = 0; i < $scope.osdPref.strm.length; i++) {
        for (j = 0; j < $scope.osdPref.strm[i].region.length; j++) {
          $scope.osdPref.strm[i].region[j].enabled =
            $scope.osdPref.strm[i].region[j].enabled == 1;
        }
      }
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
          'Cache-Control': 'no-cache',
        },
        data: JSON.stringify($scope.osdPref),
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
      // Load OSD_PM at first to aviod wrong loop in inValidCnt()
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
          "cmd_id": Cmd.AGTX_CMD_OSD_PM_CONF,
          "cmd_type": "get",
        }),
      }).then(function (resp) {
        console.log('post getdata success: ' + 'AGTX_CMD_OSD_PM_CONF');
        console.log(resp);

        $scope.osd_pmPref = resp.data;
      });
      //
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
          "cmd_id": Cmd.AGTX_CMD_OSD_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_OSD_CONF');
          console.log(resp);

          $scope.osdPref = resp.data;
          $scope.canApply = true;
          var i, j;
          for (i = 0; i < $scope.osdPref.strm.length; i++) {
            for (j = 0; j < $scope.osdPref.strm[i].region.length; j++) {
              $scope.osdPref.strm[i].region[j].enabled =
                $scope.osdPref.strm[i].region[j].enabled == 1;
            }
          }
          $scope.updateFormatFromString(
            $scope.osdPref.strm[0].region[1].type_spec
          );
          $scope.Result = resp;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_OSD_CONF');
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
