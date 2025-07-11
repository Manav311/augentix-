(function() {
  var ua = window.navigator.userAgent;
  var index = 0;

  var isIE = function() {
    index = ua.indexOf('MSIE ');
    if (index > 0) {
      return true;
    }

    index = ua.indexOf('Trident/');
    if (index > 0) {
      return true;
    }

    return false;
  };

  var isMsEdge = function() {
    index = ua.indexOf('Edge/');
    return index > 0;
  };

  var isSupported = function() {
    if (isIE()) {
      return false;
    }

    if (isMsEdge()) {
      var version = parseInt(ua.substring(edge + 5, ua.indexOf('.', edge)), 10);
      if (version < 80) {
        return false;
      }
    } else {
      return flvjs.isSupported();
    }
  };

  if (!isSupported()) {
    var msg = 'The current web browser does not support Augentix Player.';
    console.error(msg);
    window.alert(msg);
  }
})();
