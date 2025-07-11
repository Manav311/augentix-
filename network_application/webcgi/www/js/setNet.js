var app = angular.module('HC_App', []);
app.config([
  '$httpProvider',
  function ($httpProvider) {
    //initialize get if not there
    if (!$httpProvider.defaults.headers.get) {
      $httpProvider.defaults.headers.get = {};
    }

    // Answer edited to include suggestions from comments
    // because previous version of code introduced browser-related errors

    //disable IE ajax request caching
    $httpProvider.defaults.headers.get['If-Modified-Since'] =
      'Mon, 26 Jul 1997 05:00:00 GMT';
    // extra
    $httpProvider.defaults.headers.get['Cache-Control'] = 'no-cache';
    $httpProvider.defaults.headers.get['Pragma'] = 'no-cache';
  },
]);

app.controller('HC_Ctrl', [
  '$scope',
  '$http',
  function ($scope, $http) {
    //get IP
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
    //set IP
    $scope.setIP = function () {
      $scope.cmd = '/setIP.cgi';
      console.log('setIP.cgi');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'text/plain',
        },
        data: $scope.IPAddress,
      }).then(
        function (resp) {
          console.log('post success');
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    //get Mask
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
    //set Mask
    $scope.setMask = function () {
      $scope.cmd = '/setMask.cgi';
      console.log('setMask.cgi');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'text/plain',
        },
        data: $scope.NetMask,
      }).then(
        function (resp) {
          console.log('post success');
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    //get Gateway
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
    //set Gateway
    $scope.setGateway = function () {
      $scope.cmd = '/setGateway.cgi';
      console.log('setGateway.cgi');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'text/plain',
        },
        data: $scope.Gateway,
      }).then(
        function (resp) {
          console.log('post success');
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    //get DNS
    $scope.PrimaryDNS = ' ';
    //$scope.SecondaryDNS= " ";
    $scope.getDNS = function () {
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
          $scope.PrimaryDNS = resp.data;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };

    //	$scope.PrimaryDNS= " ";
    $scope.SecondaryDNS = ' ';
    $scope.getDNS = function () {
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
          $scope.SecondaryDNS = resp.data;
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    //set DNS
    $scope.setDNS = function () {
      $scope.cmd = '/setDNS.cgi';
      console.log('setDNS.cgi');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'text/plain',
        },
        data: `${$scope.PrimaryDNS}&${$scope.SecondaryDNS}`,
      }).then(
        function (resp) {
          console.log('post success');
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getIPAddress();
    $scope.getNetMask();
    $scope.getMacAddress();
    $scope.getGateway();
  },
]);
