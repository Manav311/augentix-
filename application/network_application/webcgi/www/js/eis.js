$(document).ready(function () {
  $('input[type="number"]').blur(function () {
    v = parseInt($(this).val());
    min = parseInt($(this).attr('min'));
    max = parseInt($(this).attr('max'));

    if (v < min) {
      $(this).val(min);
    } else if (v > max) {
      $(this).val(max);
    }
    if (typeof v == 'undefined') {
      $(this).val(min);
    }
  });
});

app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.VideMenu = setVideoPage(Video_Index.eis);
    $scope.msg_t = {
      augentix: ['Augentix', '多方科技', '多方科技'],
      noTxt: ['', '', ''],
      Auto: ['Auto', '自動', '自动'],
      Read_setting: ['Read out from device', '讀取設定', '读取设定'],
      Apply: ['Apply', '應用', '应用'],
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      NoRespond: ['Device no respond!!', '設備無反應!!', '设备无反应!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
      Channel: ['Channel', '通道', '通道'],
      Codec_Type: ['Codec Type', '編碼格式', '编码格式'],
      Stream_count: ['Stream count', '視訊流通道數', '视频流通道数'],
      Stream_Index: ['Stream Index', '視訊流通道號', '視訊流通道號'],
      Global_setting: ['Global setting', '整體設置', '整体设置'],
      Enable_EIS: ['Enable EIS', '開啟電子防手震', '开启电子防手震'],
      EIS_Strength: ['EIS Strength', '電子防手震強度', '电子防手震强度'],
      EIS_Note: ['Note: The setting of EIS strength allows for a trade-off between the effectiveness of stabilization and the visibility of the jelly effect.', '備註: 電子防手震強度設定可以在穩定效果的有效性和果凍效應的可見性之間進行權衡。', '备註:电子防手震强度设置可以在稳定效果的有效性和果冻效应的可见性之间进行权衡。'],
   };
    $scope.VideoMenu = setPage(
      Video_Index.video_layout,
      Video_Index,
      VideoMenu
    );
    $scope.cmdStatus = '123';
    $scope.videoLayoutPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_VIDEO_LAYOUT_CONF,
      cmd_type: 'set',
      layout_en: 0,
      layout_num: 2,
      video_dev_idx: 0,
      video_layout: [
        {
          video_strm_idx: 0,
          window_num: 2,
          window_array: [
            {
              const_qual: 0,
              dyn_adj: 0,
              eis_en: 0,
              parent: -1,
              path_bmp: 1,
              pos_height: 1024,
              pos_width: 1024,
              pos_x: 0,
              pos_y: 0,
              priority: 0,
              roi_height: 1024,
              roi_width: 1024,
              roi_x: 0,
              roi_y: 0,
              update_fps: 30,
              view_type: "NORMAL",
              window_idx: 0
            },
            {
              const_qual: 0,
              dyn_adj: 0,
              eis_en: 0,
              parent: -1,
              path_bmp: 1,
              pos_height: 1024,
              pos_width: 1024,
              pos_x: 0,
              pos_y: 0,
              priority: 0,
              roi_height: 1024,
              roi_width: 1024,
              roi_x: 0,
              roi_y: 0,
              update_fps: 30,
              view_type: "NORMAL",
              window_idx: 0
            },
          ],
        },
        {
          video_strm_idx: 1,
          window_num: 2,
          window_array: [
            {
              const_qual: 0,
              dyn_adj: 0,
              eis_en: 0,
              parent: -1,
              path_bmp: 1,
              pos_height: 1024,
              pos_width: 1024,
              pos_x: 0,
              pos_y: 0,
              priority: 0,
              roi_height: 1024,
              roi_width: 1024,
              roi_x: 0,
              roi_y: 0,
              update_fps: 30,
              view_type: "NORMAL",
              window_idx: 0
            },
            {
              const_qual: 0,
              dyn_adj: 0,
              eis_en: 0,
              parent: -1,
              path_bmp: 1,
              pos_height: 1024,
              pos_width: 1024,
              pos_x: 0,
              pos_y: 0,
              priority: 0,
              roi_height: 1024,
              roi_width: 1024,
              roi_x: 0,
              roi_y: 0,
              update_fps: 30,
              view_type: "NORMAL",
              window_idx: 0
            },
          ],
        },
      ],
    };
    $scope.videoDevPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_VIDEO_DEV_CONF,
      cmd_type: 'set',
      video_dev_idx: 0,
      hdr_mode: 0,
      stitch_en: 1,
      eis_en: 0,
      bayer: 1,
      input_fps: 25,
      input_path_cnt: 2,
      input_path_list: [
        {
          path_idx: 0,
          path_en: 1,
          sensor_idx: 0,
          width: 1920,
          height: 1080,
          eis_strength: 60
        },
        {
          path_idx: 1,
          path_en: 1,
          sensor_idx: 1,
          width: 1920,
          height: 1080,
          eis_strength: 60
        }
      ]
    };

    $scope.getDevPath = function(pathBmp) {
      for (let i = 0; i < $scope.videoDevPref.input_path_cnt; i++) {
        if (pathBmp === Math.pow(2, i)) {
          return i;
        }
      }
      return "Invalid";
    };

    $scope.getTotalChannels = function() {
      var totalChannels = 0;
      for (var i = 0; i < $scope.videoLayoutPref.layout_num; i++) {
        totalChannels += $scope.videoLayoutPref.video_layout[i].window_num;
      }
      return totalChannels;
    };

    $scope.totalChannels = $scope.getTotalChannels();

    $scope.getChannelDevPath = function(channelIndex) {
      var cumulativeIndex = 0;
      for (var i = 0; i < $scope.videoLayoutPref.layout_num; i++) {
        for (var j = 0; j < $scope.videoLayoutPref.video_layout[i].window_num; j++) {
          if (cumulativeIndex === channelIndex) {
            var pathBmp = $scope.videoLayoutPref.video_layout[i].window_array[j].path_bmp;
            return Math.log2(pathBmp); // Assumes path_bmp is always a power of 2
          }
          cumulativeIndex++;
        }
      }
      return "Invalid";
    };

    $scope.getChannel = function(channelIndex) {
      var cumulativeIndex = 0;
      for (var i = 0; i < $scope.videoLayoutPref.layout_num; i++) {
        for (var j = 0; j < $scope.videoLayoutPref.video_layout[i].window_num; j++) {
          if (cumulativeIndex === channelIndex) {
            return $scope.videoLayoutPref.video_layout[i].window_array[j];
          }
          cumulativeIndex++;
        }
      }
      return null;
    };

    $scope.updateStrength = function(index, strength) {
      $scope.videoDevPref.input_path_list[index].eis_strength = strength;
    };

    $scope.sendData = function () {
      $scope.videoLayoutPref.cmd_type = 'set';
      $scope.videoDevPref.cmd_type = 'set';
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
          'Cache-Control': 'no-cache',
        },
        data: JSON.stringify($scope.videoLayoutPref),
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
      $scope.cmd = '/cgi-bin/msg.cgi';
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
          'Cache-Control': 'no-cache',
        },
        data: JSON.stringify($scope.videoDevPref),
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
    $scope.getData = function (hide_msg) {
      $scope.canApply = false;
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('get video layout data');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
          'Cache-Control': 'no-cache',
        },
        data: JSON.stringify({
          "master_id": 1,
          "cmd_id": Cmd.AGTX_CMD_VIDEO_LAYOUT_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_VIDEO_LAYOUT_CONF');
          $scope.videoLayoutPref = resp.data;
          $scope.Result = resp;
          $scope.canApply = true;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_VIDEO_LAYOUT_CONF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('get video dev data');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
          'Cache-Control': 'no-cache',
        },
        data: JSON.stringify({
          "master_id": 1,
          "cmd_id": Cmd.AGTX_CMD_VIDEO_DEV_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_VIDEO_DEV_CONF');
          $scope.videoDevPref = resp.data;
          $scope.Result = resp;
          $scope.canApply = true;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_VIDEO_DEV_CONF');
          console.log(resp);
          $scope.canApply = true;
          //$scope.Result = "Post Error";
        }
      );
      $scope.getTotalChannels();
    };
    $scope.$watch('videoLayoutPref', function(newValue, oldValue) {
      if (newValue !== oldValue) {
        $scope.totalChannels = $scope.getTotalChannels();
      }
    }, true);
    $scope.getData();
    //console.log($scope.resTable);
    //console.log($scope.video_option_total_stream_list);
  },
]);
