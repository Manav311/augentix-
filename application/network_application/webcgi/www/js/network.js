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
      Read_setting: ['Read out from device', '讀取設定', '读取设定'],
      Apply: ['Apply', '應用', '应用'],
      Reboot: ['Reboot', '重新開機', '重新开机'],
      Loading: ['Loading...', '讀取中...', '读取中...'],
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      NoRespond: ['Device no respond!!', '設備無反應!!', '设备无反应!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
      Wired_Network_Interface: [
        'Wired Network Interface',
        '有線網路',
        '有线网络',
      ],
      Network_Mode: ['Network Mode', '網路模式', '网络模式'],
      Static_IP: ['Static IP', '靜態 IP', '静态 IP'],
      IP_Address: ['IP Address', 'IP 地址', 'IP 地址'],
      Subnet_Mask: ['Subnet Mask', '子網掩碼', '子网掩码'],
      Defualt_Gateway: ['Defualt Gateway', '預設網關', '预设网关'],
      Primary_DNS: ['Primary DNS', '主 DNS', '主 DNS'],
      Secondary_DNS: ['Secondary DNS', '次 DNS', '次 DNS'],
      Reboot_msg: [
        'Setting will be active while reboot!!',
        '設定會在重新開機後生效!!',
        '设定会在重新开机后生效!!',
      ],
      Rebooting_msg: [
        'Rebooting.Please manually connect again after rebooting!',
        '重新開機中，請在完成開機後手動重新連接。',
        '重新开机中,请在完成开机后手动重新连接.',
      ],
      Wireless_Network_Interface:['Wireless Network Interface','無線網路','无线网络'],
      Required: ['Required!!', '必填!!', '必填!!'],
      InvalidAddress: ['Invalid address!!', '不合格的地址!!', '不合格的地址!!'],
    };

    $scope.cmd = 'cmd';
    $scope.Result = 'result';
    $scope.cmdStatus = '';
    $scope.NetworkConfig = {
      Hostname: 'Augentix',
      IPAddress: '192.168.1.100',
      Netmask: '255.255.192.0',
      Gateway: '192.168.1.253',
      DNS1: '192.168.1.12',
      DNS2: '192.168.1.18',
      MAC: '12:34:56:c9:48:61 ',
      dhcp: 0,
    };

    $scope.sendData = function () {
      $scope.cmd = '/assignIP.cgi';
      console.log('Assing network setting');
      console.log($scope.NetworkConfig);
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.NetworkConfig),
      }).then(
        function (resp) {
          console.log('post wired success');
          $scope.Result = resp;
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          $scope.logout(); // Ensure session termination, but do not require re-login.
          $scope.reboot();
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
        }
      );
    };

    $scope.logout = async function () {
      var originURL = window.location.origin;
      var logoutURL = originURL + '/html/logout.html';
      return fetch(logoutURL, {
        method: 'GET',
        headers: {
          'Authorization': 'Basic logout',
        },
      });
    };

    $scope.reboot = function () {
      $scope.cmd = '/reboot.cgi';
      console.log('Reboot');
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
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.NoRespond[$scope.lang];
          //    $scope.Result = "Post Error";
        }
      );
    };

    $scope.ipForm = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;

    $scope.getData = function (hide_msg) {
      $scope.canApply = false;
      $scope.cmdStatus = $scope.msg_t.Loading[$scope.lang];
      $scope.cmd = '/netInfo.cgi';
      console.log('/netInfo.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          $scope.NetworkConfig = resp.data;
          $scope.canApply = true;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getData(1);
  },
]);
