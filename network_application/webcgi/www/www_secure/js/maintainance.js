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
      Uploading_msg: ['Uploading', '上傳中', '上传中'],
      Factory_Default: ['Factory Default', '恢復出廠值', '恢复出厂值'],
      Restore: ['Restore', '恢復', '恢复'],
      Confirm: ['Confirm', '確認', '确认'],
      ConfirmToUpgrade: [
        '(Will disable stream process!!)',
        '(將會關掉視訊!!)',
        '(将会关掉视频!!)'
      ],
      Firmware_Upgrade: ['Firmware Upgrade', '韌體上傳', '固件更新'],
      Firmware_Upgrade_msg: ['Firmware upgrading, please wait for a few minutes', '韌體更新中，請稍等數分鐘', '固件更新中，请稍等数分钟'],
      Switch2SysupdOS: ['System Update', '系統更新', '系统更新'],
      Cert_Upload: ['Upload CA certification', '上傳CA憑證', '上传CA凭证'],
      KeyUpload: ['Upload CA key', '上傳CA key', '上传CA key'],
      Cert_Uploading: ['Uploading CA certification...', '上傳CA憑證中...', '上传CA凭证中...'],
      KeyUploading: ['Uploading CA key..', '上傳CA key中...', '上传CA key中...'],
      UploadOK: ['Upload success!! ', '上傳成功!!', '上传成功'],
      UploadFail: ['Upload failed!! ', '上傳失敗!!', '上传失败'],
      Updating_CA: ['Updating CA certification', '更新CA憑證中', '更新CA凭证中'],
      Updating_CA_OK: ['Update CA certification done! Please restart your browser', '更新CA憑證成功! 請更新瀏覽器!', '更新CA凭证成功! 请更新浏览器!'],
      Updating_CA_Fail: ['Wrong CA files!! Please check your files and upload again!!', '錯誤的CA憑證! 請檢查並重新上傳!', '错误的CA凭证! 请检查并重新上传!'],
      Reset_CA_Default: ['Reset CA certification to default', '重設CA憑證回出廠值', '重设CA凭证回出厂值'],
      Reset_CA_OK: ['Reset CA certification done! Please restart your browser', '重設CA憑證成功! 請更新瀏覽器!', '重设CA凭证成功! 请更新浏览器!'],
      Reboot: ['Reboot', '重新開機', '重新开机'],
      Rebooting_msg: [
        'Rebooting. Please manually connect again after rebooting!',
        '重新開機中，請在完成開機後手動重新連接。',
        '重新开机中,请在完成开机后手动重新连接.',
      ],
      Auth_Prompt_Username: ['Please enter your username:', '請輸入帳號:', '请输入账号:'],
      Auth_Prompt_Password: ['Please enter your password:', '請輸入密碼:', '请输入密码:'],
      Auth_Error: ['Incorrect username or password, the page will reload.', '帳號或密碼錯誤，頁面即將重新載入。', '账号或密码错误，页面即将重新加载。'],
      Auth_Server_Error: [
        'Error connecting to the authentication server, please try again later.',
        '連接到認證伺服器時發生錯誤，請稍後再試。',
        '连接到认证服务器时发生错误，请稍后再试。',
      ],
      Auth_Success: ['Authentication successful!', '認證成功!', '认证成功!'],
      Auth_Title: ['Authentication Required', '需要認證', '需要认证'],
      Auth_Prompt_Username: ['Please enter your username:', '請輸入帳號:', '请输入账号:'],
      Auth_Prompt_Password: ['Please enter your password:', '請輸入密碼:', '请输入密码:'],
      Auth_Error: ['Incorrect username or password, the page will reload.', '帳號或密碼錯誤，頁面即將重新載入。', '账号或密码错误，页面即将重新加载。'],
      Auth_Server_Error: [
        'Error connecting to the authentication server, please try again later.',
        '連接到認證伺服器時發生錯誤，請稍後再試。',
        '连接到认证服务器时发生错误，请稍后再试。',
      ],
      Auth_Success: ['Authentication successful!', '認證成功!', '认证成功!'],
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

    $scope.stopStream = function () {
      $scope.cmd = '/stopStream.cgi';
      console.log('Stop Stream');
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
          console.log(resp);
          if (resp.data.rval == 0) {
            $scope.cmdStatus = 'Stop stream!!';
          } else {
            $scope.cmdStatus = 'Stop stream failed,rval=' + resp.data.rval;
          }
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = 'Stop stream failed,rval=' + resp.data.rval;
          //	$scope.Result = "Post Error";
        }
      );
    };

    $scope.alwaysHide = true;
    $scope.canUpload = true;
    $scope.sysupd = function () {
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
    $scope.confirmCARestore = false;
    $scope.confirmedUpgrade = false;
    $scope.preCADefault = function () {
      $scope.confirmCARestore = true;
    };
    function clickUpload(){
      document.getElementById("nginxUpload").click();
    }

    $scope.applyUpload = function () {
      $scope.canUpload = false;
      setTimeout(clickUpload,300);
    }

    $scope.authModalVisible = false;
    $scope.username = '';
    $scope.password = '';

    $scope.preFactoryDefault = function () {
      $scope.authModalVisible = true;

      $scope.authSuccessCallback = function () {
        $scope.confirmRestore = true;
        $scope.$apply();
      };
    };

    $scope.closeAuthModal = function () {
      $scope.authModalVisible = false;
    };

    $scope.submitAuth = async function () {
      const serverHost = window.location.hostname;
      const authUrl = `https://${serverHost}/auth`;
      const credentials = btoa(`${$scope.username}:${$scope.password}`);

      try {
        const response = await fetch(authUrl, {
          method: 'GET',
          headers: {
            'Authorization': `Basic ${credentials}`
          }
        });

        if (response.ok) {
          $scope.authModalVisible = false;
          $scope.$apply();

          if (typeof $scope.authSuccessCallback === 'function') {
            $scope.authSuccessCallback();
          }

          alert($scope.msg_t.Auth_Success[$scope.lang]);
        } else {
          alert($scope.msg_t.Auth_Error[$scope.lang]);
          location.reload();
        }
      } catch (error) {
        console.error('Error:', error);
        alert($scope.msg_t.Auth_Server_Error[$scope.lang]);
      }
    };


    $scope.preFirmwareUpload = function () {
      $scope.authModalVisible = true;

      $scope.authSuccessCallback = function () {
        $scope.stopStream();
        $scope.confirmedUpgrade = true;
        console.log("Auth verified!");
      };
    };
    $scope.confirmUpload = false;
    $scope.importSetting = function () {
      $scope.cmdStatus = $scope.msg_t.Uploading_msg[$scope.lang];
      console.log('Uploading import setting');
      $scope.canUpload = false;
    };


    $scope.uploadCert = function() {
      $scope.cmdStatus = $scope.msg_t.Cert_Uploading[$scope.lang];
      console.log('Uploading cert file');
      var file = $scope.certFile;
      console.log('file is ');
      console.dir(file);
      var uploadUrl = "/uploadCert.cgi";
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
        },
        function(resp) {
          $scope.cmdStatus = $scope.msg_t.UploadFail[$scope.lang];
	});
    };

    $scope.uploadKey = function() {
      $scope.cmdStatus = $scope.msg_t.KeyUploading[$scope.lang];
      console.log('Uploading key file');
      var file = $scope.keyFile;
      console.log('file is ');
      console.dir(file);
      var uploadUrl = "/uploadKey.cgi";
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
          $scope.linkCA();
        },
        function(resp) {
          $scope.cmdStatus = $scope.msg_t.UploadFail[$scope.lang];
	});
    };

    $scope.linkCA = function () {
      $scope.cmd = '/updateCA.cgi';
      console.log('Link CA to upload files');
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
            $scope.cmdStatus = $scope.msg_t.Updating_CA_OK[$scope.lang];
          } else {
              if (resp.data.rval == 256) {
                $scope.cmdStatus = $scope.msg_t.Updating_CA_Fail[$scope.lang];
          }
          console.log(resp);
	  }
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.Updating_CA_Fail[$scope.lang];
          //    $scope.Result = "Post Error";
        }
      );
    };

    $scope.updateCA = function () {
      $scope.uploadCert();
      $scope.uploadKey();
    }

    $scope.resetCA = function () {
      $scope.cmd = '/resetCA.cgi';
      console.log('Reset CA to default');
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
            $scope.cmdStatus = $scope.msg_t.Reset_CA_OK[$scope.lang];
          } else {
            $scope.cmdStatus = 'Reset Error,rval=' + resp.data.rval;
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //    $scope.Result = "Post Error";
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
