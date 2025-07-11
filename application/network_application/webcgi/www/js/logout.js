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
      Logout: ['Logout', '登出', '注销'],
    };
    $scope.logout = function() {
      var originURL = window.location.origin;
      var logoutURL = originURL + '/html/logout.html';
      var redirectURL = originURL;

      fetch(logoutURL, {
        method: 'GET',
        headers: {
          'Authorization': 'Basic logout',
        },
      });

      setTimeout(function() {
        window.location.href = redirectURL;
      }, 100);
    };
  },
]);
