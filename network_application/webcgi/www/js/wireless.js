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
      IP_Address: ['IP Address', 'IP 地址', 'IP 地址'],
      Get_SSID_msg:['Get SSID !!','獲取SSID !!','获取SSID !!'],
      Wireless_Network_Interface:['Wireless Network Interface','無線網路','无线网络'],
      Scan_WIFI:['Scan WIFI','搜尋WIFI','扫寻WIFI'],
      Scanning:['Scanning...','搜尋中...','扫寻中...'],
      Scanned:['Scan Finshed','搜尋完成','扫寻完成'],
      Hide_Or_Show_Password:[
        'Hide Or Show Password',
        '顯示或隱藏密碼',
        '显示或隐藏密码'
      ],
      wireless_IP: ['Wireless IP', '無線網路', '无线网络IP'],
      connected_SSID: ['Connected SSID', '連上的SSID', '连上的SSID'],
      connect_wifi: ['Connect WiFi', '連接WiFi', '连接WiFi'],
      connecting: ['Connecting...', '連接中...', '连接中...'],
      connected: ['Connection Succeeded!', '連接成功!', '连接成功!'],
      disconnect_wifi: ['Disconnected WiFi', '斷開WiFi', '断开WiFi'],
    };
    $scope.canApply = false;
    $scope.cmd = 'cmd';
    $scope.cmdStatus = '';

    $scope.wireless_ssid = {
      "ssidnumber":0,
      "SSID":[]
    };

    $scope.wlanInfo= {
      "ssid":"",
      "wifiIP":""
    };
    
    $scope.linkwifi = {
      "ssid":"",
      "psk":""
    };

    $scope.setupwifi = function() {
      $scope.canApply = false;
      $scope.cmd = "/setWifi.cgi";
      if($scope.linkwifi.ssid.length==0){
        $scope.cmdStatus="SSID empty!!";
        return;
      }
      if($scope.linkwifi.psk.length==0){
        $scope.cmdStatus="Pass empty!!";
        return;
      }
      console.log('Assing wifi setting');
      console.log($scope.linkwifi);
      $scope.cmdStatus = $scope.msg_t.connecting[$scope.lang];
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json'
        },
        data: JSON.stringify($scope.linkwifi)
      }).then(function(resp) {
        $scope.canApply = true;
        console.log('post wireless success');
        $scope.getWlanInfo();
        $scope.cmdStatus = $scope.msg_t.connected[$scope.lang];
        console.log(resp);
      }, function(resp) {
        $scope.canApply = true;
        console.log('post error');
        $scope.disconnect_wifi();
        console.log(resp);
        $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
      });
    };

    $scope.disconnect_wifi = function() {
      $scope.canApply = false;
      $scope.cmd = "/disconnectWifi.cgi";
      console.log('disconnect wifi');
      $scope.wlanInfo.wifiIP = " ";
      $http({
        method: 'get',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded'
        }
      }).then(function(resp) {
        $scope.canApply = true;
        console.log('disconnect wifi successed');
        $scope.Result = resp;
        $scope.getWlanInfo();
        console.log(resp);
        $scope.cmdStatus = $scope.msg_t.disconnect_wifi[$scope.lang];
      }, function(resp) {
        $scope.canApply = true;
        console.log('disconnect wifi error');
        console.log(resp);
        $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
      });
    };

    $scope.getSSID = function() {
        $scope.canApply = false;
        $scope.cmdStatus = $scope.msg_t.Scanning[$scope.lang];
        $scope.cmd = "/getSSID.cgi";
        console.log('get around ssid');
        $http({
            method: 'get',
            url: $scope.cmd,
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            }
        }).then(function(resp) {
            $scope.canApply = true;
            console.log('post success');
            $scope.wireless_ssid = resp.data;            
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.Scanned[$scope.lang];
        }, function(resp) {
            $scope.canApply = true;
            console.log('get around ssid error');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
        });
    }
    $scope.getWlanInfo = function() {
        $scope.cmd = "/getWlanInfo.cgi";
        console.log('get wlan info');
        $http({
            method: 'get',
            url: $scope.cmd,
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            }
        }).then(function(resp) {
            console.log('post success');
            if (resp.data.ssid ==""){
            $scope.wlanInfo.ssid="";
            $scope.wlanInfo.wifiIP="";
            } else {
             // $scope.wlanInfo = resp.data;
            $scope.wlanInfo.ssid=resp.data.ssid;
            $scope.wlanInfo.wifiIP=resp.data.wifiIP;
            }
            console.log(resp);
        }, function(resp) {
            console.log('get wlan info error');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
        });
    }

    $scope.showHidePwd = function(){
          if(document.getElementById("pwd").type=="password")
            document.getElementById("pwd").type = "text";
          else
            document.getElementById("pwd").type = "password";
    }

    
    $scope.getWPAInfo = function() {
        $scope.cmd = "/getWPASSID.cgi";
        console.log('get wpa ssid');
        $http({
            method: 'get',
            url: $scope.cmd,
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            }
        }).then(function(resp) {
            console.log('get wpainfo success');
            $scope.linkwifi = resp.data; 
            console.log(resp);
        }, function(resp) {
            console.log('get wpassid error');
            console.log(resp);
            $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
        });
    };
    $scope.getSSID();
    $scope.getWPAInfo();
    $scope.getWlanInfo();
  },
]);
