app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.msg_t = {
      Read_setting: ['Read out from device', '讀取設定', '读取设定'],
      Apply: ['Apply', '應用', '应用'],
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      NoRespond: ['Device no respond!!', '設備無反應!!', '设备无反应!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
    };
    $scope.LMenu = LMenu;
    $scope.IvaMenu = setIvaPage(-1);

    var vdbgEnum = {
      od: 0,
      td: 1,
      md: 2,
      ef: 3,
      ld: 4,
      rms: 5,
      aroi: 6,
      fld: 7,
      expo: 8,
      debug: 9,
    };

    $scope.vdbgPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_VDBG_CONF,
      cmd_type: 'set',
      enabled: 0,
      ctx: 0,
    };

    $scope.vdbgGetCtxVal = function () {
      var ctx =
        ($scope.vdbgJs.od << vdbgEnum.od) |
        ($scope.vdbgJs.td << vdbgEnum.td) |
        ($scope.vdbgJs.md << vdbgEnum.md) |
        ($scope.vdbgJs.ef << vdbgEnum.ef) |
        ($scope.vdbgJs.ld << vdbgEnum.ld) |
        ($scope.vdbgJs.rms << vdbgEnum.rms) |
        ($scope.vdbgJs.aroi << vdbgEnum.aroi) |
        ($scope.vdbgJs.fld << vdbgEnum.fld) |
        ($scope.vdbgJs.expo << vdbgEnum.expo) |
        ($scope.vdbgJs.debug << vdbgEnum.debug);
      return ctx;
    };

    $scope.vdbgGetJsCtxVal = function () {
      $scope.vdbgJs.od =
        $scope.vdbgPref.ctx & (1 << vdbgEnum.od) ? true : false;
      $scope.vdbgJs.td =
        $scope.vdbgPref.ctx & (1 << vdbgEnum.td) ? true : false;
      $scope.vdbgJs.md =
        $scope.vdbgPref.ctx & (1 << vdbgEnum.md) ? true : false;
      $scope.vdbgJs.ef =
        $scope.vdbgPref.ctx & (1 << vdbgEnum.ef) ? true : false;
      $scope.vdbgJs.ld =
        $scope.vdbgPref.ctx & (1 << vdbgEnum.ld) ? true : false;
      $scope.vdbgJs.rms =
        $scope.vdbgPref.ctx & (1 << vdbgEnum.rms) ? true : false;
      $scope.vdbgJs.aroi =
        $scope.vdbgPref.ctx & (1 << vdbgEnum.aroi) ? true : false;
      $scope.vdbgJs.fld =
        $scope.vdbgPref.ctx & (1 << vdbgEnum.fld) ? true : false;
      $scope.vdbgJs.expo =
        $scope.vdbgPref.ctx & (1 << vdbgEnum.expo) ? true : false;
      $scope.vdbgJs.debug =
        $scope.vdbgPref.ctx & (1 << vdbgEnum.debug) ? true : false;
    };

    $scope.vdbgJs = {
      od: 0,
      md: 0,
      td: 0,
      rms: 0,
      ld: 0,
      ef: 0,
      aroi: 0,
      fld: 0,
      expo: 0,
      debug: 0,
    };

    $scope.Result = 'result';

    $scope.sendData = function () {
      $scope.vdbgPref.ctx = $scope.vdbgGetCtxVal();
      $scope.vdbgPref.cmd_type = 'set';
      if ($scope.vdbgPref.enabled) $scope.vdbgPref.enabled = 1;
      else $scope.vdbgPref.enabled = 0;
      $scope.cmd =  '/cgi-bin/msg.cgi';
      var postData = JSON.stringify($scope.vdbgPref, function (key, value) {
        if (key === '$$hashKey') {
          return undefined;
        }
        return value;
      });
      $scope.vdbgPref.enabled = $scope.vdbgPref.enabled == 1;
      console.log(JSON.stringify($scope.vdbgPref));
      console.log(JSON.stringify($scope.vdbgJs));
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
          "cmd_id": Cmd.AGTX_CMD_VDBG_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_VDBG_CONF');
          $scope.vdbgPref = resp.data;
          $scope.vdbgPref.enabled = $scope.vdbgPref.enabled == 1;
          $scope.Result = resp;
          $scope.vdbgGetJsCtxVal();
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_VDBG_CONF');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.getData();
  },
]);
