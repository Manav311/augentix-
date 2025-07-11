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
      New_username: ['New username:', '新用戶名:', '新用户名:'],
      New_password: ['New password:', '新密碼:', '新密码:'],
      password_title: [
        'Not include single quotes',
        '不能包含單引號',
        '不能包含单引号',
      ],
      username_title: [
        'Only numbers letters and underline',
        '只能是數字、字母和下劃綫',
        '只能是数字、字母和下划线',
      ],
    };

    $scope.UserCfg = {
      name: '',
      pass: '',
    };
    $scope.Result = 'result';
    $scope.cmdStatus = '';
    $scope.cmd = 'cmd';

    $scope.sendData = function () {
      $scope.cmd = '/changePass.cgi';
      if ($scope.UserCfg.name.length == 0) {
        $scope.cmdStatus = 'Name empty!!';
        return;
      }
      if ($scope.UserCfg.pass.length == 0) {
        $scope.cmdStatus = 'Pass empty!!';
        return;
      }

      if ($scope.UserCfg.pass.length < 8) {
        $scope.cmdStatus = 'Pass invalid!!';
        return;
      }

	  if ($scope.UserCfg.pass.length > 32) {
        $scope.cmdStatus = 'Pass invalid!!';
        return;
      }

      if (/\s/.test($scope.UserCfg.pass)) {
        $scope.cmdStatus = "Password cannot contain spaces.";
        return ;
      }

      if ($scope.UserCfg.pass == $scope.UserCfg.name) {
        $scope.cmdStatus = "Password == name.";
        return ;
      }

      const complexityRegex = /^(?=.*[a-z])(?=.*[A-Z])(?=.*[\W_]).+$/;
      if (!complexityRegex.test($scope.UserCfg.pass)) {
        $scope.cmdStatus =  "Password must include uppercase lowercase, and one special character.";
        return;
      }

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.UserCfg),
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Result = resp;
          if (resp) {
            $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          } else {
            $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
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
  },
]);
