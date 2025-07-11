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
      Device: ['Device', '設備', '设备'],
      Model_Name: ['Model Name', '設備名稱', '设备名称'],
      System_Up_Time: ['System Up Time', '運行時間', '运行时间'],
      Hardware_Ver: ['Hardware Ver.', '硬體版本:', '硬件版本:'],
      Firmware_Ver: ['Firmware Ver.', '韌體版本: ', '固件版本'],
      Network: ['Network', '網路: ', '网络'],
      Activated_Interface: ['Activated Interface', '有效界面', '激活界面'],
      Host_Name: ['Host Name', '主機名: ', '主机名'],
      MAC_Address: ['MAC Address', 'MAC地址', 'MAC地址'],
      IP_Address: ['IP Address', 'IP 地址', 'IP 地址'],
      Subnet_Mask: ['Subnet Mask', '子網掩碼', '子网掩码'],
      Defualt_Gateway: ['Defualt Gateway', '預設網關', '预设网关'],
      Primary_DNS: ['Primary DNS', '主 DNS', '主 DNS'],
      Secondary_DNS: ['Secondary DNS', '次 DNS', '次 DNS'],
      Download_Security_Log: ['Download Security Log', '下載安全日誌', '下载安全日志'],
    };

    $scope.cmd = 'cmd';
    $scope.Result = 'result';
    $scope.IPAddress = ' ';
    $scope.getIPAddress = function () {
      $scope.cmd = '/getIPAddress.cgi';
      console.log('getIPAddress.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.IPAddress = resp.data;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.productname = '';
    $scope.productname.dev_name = '';
    $scope.getProduct = function () {
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('Get product name');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify({
          "master_id": 1,
          "cmd_id": Cmd.AGTX_CMD_SYS_INFO,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post success: ' + 'AGTX_CMD_SYS_INFO');
          $scope.productname = resp.data;
          console.log('product name: ' + $scope.productname.dev_name);
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_SYS_INFO');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.UpTime = ' ';
    $scope.getUpTime = function () {
      $scope.cmd = '/upTime.cgi';
      console.log('upTime.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.UpTime = resp.data;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.NetMask = ' ';
    $scope.getNetMask = function () {
      $scope.cmd = '/getNetmask.cgi';
      console.log('getNetmask.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.NetMask = resp.data;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.MacAddress = ' ';
    $scope.getMacAddress = function () {
      $scope.cmd = '/getMAC.cgi';
      console.log('getMAC.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.MacAddress = resp.data;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.Gateway = ' ';
    $scope.getGateway = function () {
      $scope.cmd = '/getGateway.cgi';
      console.log('getGateway.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Gateway = resp.data;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.HostName = ' ';
    $scope.getHostName = function () {
      $scope.cmd = '/getHostname.cgi';
      console.log('getHostname.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.HostName = resp.data;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.Dns = {
      dns1: ' ',
      dns2: ' ',
    };
    $scope.getDns = function () {
      $scope.cmd = '/getDNS.cgi';
      console.log('getDNS.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Dns = resp.data;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getFirmwareVer = function () {
      $scope.cmd = '/getFirmwareVersion.cgi';
      console.log('getFirmwareVersion.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          $scope.frimwareVersion = resp.data;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.downloadSecurityLog = function() {
      $scope.cmd = '/packNginxLog.cgi';
      console.log('packNginxLog.cgi');
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
      }).then(
        function (resp) {
          console.log('post success');
          window.location.href = '/download_security_log';
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );          
    };

    $scope.getUpTime();
    $scope.getIPAddress();
    $scope.getHostName();
    $scope.getNetMask();
    $scope.getMacAddress();
    $scope.getGateway();
    $scope.getDns();
    $scope.getFirmwareVer();
    $scope.getProduct();
  },
]);


