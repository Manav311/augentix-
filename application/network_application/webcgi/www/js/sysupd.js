app.controller('HC_Ctrl', [
  '$scope',
  '$interval',
  '$location',
  '$http',
  'fileUpload',
  function ($scope, $interval, $location, $http, fileUpload) {
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
      Export_Setting: ['Export Setting', '導出設置', '导出设置'],
      Export: ['Export', '導出', '导出'],
      Import_Settings: ['Import Settings', '導入設置', '导入设置'],
      Upload: ['Upload', '上傳', '上传'],
      Factory_Default: ['Factory Default', '恢復出廠值', '恢复出厂值'],
      Restore: ['Restore', '恢復', '恢复'],
      Confirm: ['Confirm', '確認', '确认'],
      Firmware_Upgrade: ['Firmware Upgrade', '韌體上傳', '固件更新'],
      Firmware_Upgrade_msg: ['Firmware upgrading, please wait for a few minutes', '韌體更新中，請稍等數分鐘', '固件更新中，请稍等数分钟'],
      Switch2SysupdOS: ['System Update', '系統更新', '系统更新'],
      Reboot: ['Reboot', '重新開機', '重新开机'],
      Rebooting_msg: [
        'Rebooting. Please manually connect again after rebooting!',
        '重新開機中，請在完成開機後手動重新連接。',
        '重新开机中,请在完成开机后手动重新连接.',
      ],
    };

    $scope.reboot = function () {
      $scope.cmd = '/reboot.cgi';
      console.log('Reboot');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Result = resp;
          $scope.cmdStatus = $scope.msg_t.Rebooting_msg[$scope.lang];
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.NoRespond[$scope.lang];
          //	$scope.Result = "Post Error";
        }
      );
    };
    $scope.canUpgrade = true;
    $scope.sysupd = function () {
      $scope.canUpgrade = false;
      $scope.cmd = '/SysupdOS.cgi';
      console.log('System update');
      $scope.cmdStatus = $scope.msg_t.Firmware_Upgrade_msg[$scope.lang];
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Result = resp;
          $scope.reboot();
          $scope.cmdStatus = $scope.msg_t.Rebooting_msg[$scope.lang];

          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          $scope.Result = 'Post Error';
          $scope.cmdStatus = $scope.msg_t.NoRespond[$scope.lang];
          console.log(resp);
        }
      );
    };
    $scope.switch2SysupdOS = function () {
      $scope.cmd = '/switch2SysupdOS.cgi';
      console.log('Switch to Sysupd OS');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Result = resp;
          $scope.cmdStatus = $scope.msg_t.Rebooting_msg[$scope.lang];
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          $scope.Result = 'Post Error';
          $scope.cmdStatus = $scope.msg_t.NoRespond[$scope.lang];
          console.log(resp);
        }
      );
    };

    $scope.UserCfg = {
      name: '',
      pass: '',
    };
    $scope.Result = 'result';
    $scope.cmdStatus = '';
    $scope.cmd = '';

    $scope.confirmRestore = false;
    $scope.preFactoryDefault = function () {
      $scope.confirmRestore = true;
    };
    $scope.confirmUpload = false;

    $scope.importSetting = function () {
      $scope.cmd = '/firmwareUpload.cgi';
      console.log('Firmware upload.');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Result = resp;
          $scope.cmdStatus = 'Upload setting file done!!';
          $scope.confirmUpload = true;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          $scope.Result = 'Post Error';
          $scope.cmdStatus =  'Upload setting file failed!!';
          console.log(resp);
        }
      );
    };


    $scope.factoryDefault = function () {
      $scope.cmd = '/setToDefault.cgi';
      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Result = resp;
          if (resp.data.rval == 0) {
            $scope.cmdStatus = 'Setting Done!!';
          } else {
            $scope.cmdStatus = 'Setting Error,rval=' + resp.data.rval;
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
    $scope.exportSetting = function () {
      document.getElementById('my_iframe').src = '/exportSetting.cgi';
    };
    $scope.importSetting();
  },
]);
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
