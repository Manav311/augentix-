app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.msg_t = {
      Preview: ['Preview', '預覽', '预览'],
      CaptureImage: ['Capture', '截圖', '截图'],
      Recording: ['Recording', '錄影', '录制'],
    };
    window.langId = $scope.lang;
  },
]);
