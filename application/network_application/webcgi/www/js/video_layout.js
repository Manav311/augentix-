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
      Channel: ['Channel', '通道', '通道'],
      Codec_Type: ['Codec Type', '編碼格式', '编码格式'],
      Resolution: ['Resolution', '畫面解析度', '画面解析度'],
      Frame_Rate: ['Frame Rate', '幀數:', '帧数:'],
      Rate_Control: ['Rate Control', '碼率控制模式', '码率控制模式'],
      CBR_Bit_Rate: ['CBR Bit Rate', 'CBR 編碼帶寬', 'CBR 编码带宽'],
      VBR_Max_Bit_Rate: ['VBR Max Bit Rate', 'VBR 最大帶寬', 'VBR 最大带宽'],
      VBR_Image_Quality: ['VBR_Image_Quality', 'VBR 畫面品質', 'VBR 畫面品質'],
      Flip: ['Flip', '畫面翻轉', '画面翻转'],
      Mirror: ['Mirror', '畫面鏡像', '画面镜像'],
      Stream_count: ['Stream count', '視訊流通道數', '视频流通道数'],
      Stream_Index: ['Stream Index', '視訊流通道號', '視訊流通道號'],
      Global_setting: ['Global setting', '整體設置', '整体设置'],
      Count: ['Count', '數', '数'],
      Enable_Layout: ['Enable Video Layout', 'layout_en', 'layout_en'],
      Layout: ['Layout', 'layout', 'layout'],
      Window: ['Window', 'window_num', 'window_num'],
      Layout_Count: ['Layout Count', 'layout Count', 'layout Count'],
      Video_Layout: ['Video Layout', 'video_layout', 'video_layout'],
      Window_Count: ['Window Count', 'window_num', 'window_num'],
      Width: ['Width', '寬度', '宽度'],
      Height: ['Height', '高度', '高度'],
      POS: ['POS', '位置', '位置'],
      ROI: ['ROI', '顯示', '显示'],
      View_Type: ['View Type', '顯示格式', '显示格式'],
    };
    $scope.getNumber = function (num) {
      var temp = [];
      for (var j = 0; j < num; j++) {
        temp.push(j);
      }
      return temp;
    };
    $scope.VideoMenu = setPage(
      Video_Index.video_layout,
      Video_Index,
      VideoMenu
    );
    $scope.cmdStatus = '123';
    $scope.videoLayoutCtx = {
      max_window_cnt: 2,
    };
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
              path_bmp: 1,
              pos_height: 606,
              pos_width: 1024,
              pos_x: 0,
              pos_y: 0,
              roi_height: 1024,
              roi_width: 1024,
              roi_x: 0,
              roi_y: 0,
              update_fps: 10,
              view_type: 'PANORMA',
              window_idx: 0,
            },
            {
              path_bmp: 1,
              pos_height: 418,
              pos_width: 512,
              pos_x: 256,
              pos_y: 606,
              roi_height: 1024,
              roi_width: 1024,
              roi_x: 0,
              roi_y: 0,
              update_fps: 20,
              view_type: 'PANORMA',
              window_idx: 1,
            },
          ],
        },
        {
          video_strm_idx: 1,
          window_num: 2,
          window_array: [
            {
              path_bmp: 1,
              pos_height: 606,
              pos_width: 1024,
              pos_x: 0,
              pos_y: 0,
              roi_height: 1024,
              roi_width: 1024,
              roi_x: 0,
              roi_y: 0,
              update_fps: 10,
              view_type: 'PANORMA',
              window_idx: 0,
            },
            {
              path_bmp: 1,
              pos_height: 418,
              pos_width: 512,
              pos_x: 256,
              pos_y: 606,
              roi_height: 1024,
              roi_width: 1024,
              roi_x: 0,
              roi_y: 0,
              update_fps: 20,
              view_type: 'PANORMA',
              window_idx: 1,
            },
          ],
        },
      ],
    };

    $scope.change_max_win_cnt = function () {
      var layout_cnt = $scope.videoLayoutPref.layout_num;
      for (var i = 0; i < layout_cnt; i++) {
        $scope.videoLayoutCtx.max_window_cnt = MAX(
          $scope.videoLayoutCtx.max_window_cnt,
          $scope.videoLayoutPref.video_layout[i].window_num
        );
      }
    };

    $scope.change_layout_cnt = function () {
      if ($scope.videoLayoutPref.layout_num) {
        var layout_cnt = $scope.videoLayoutPref.layout_num - 1;
        while (
          typeof $scope.videoLayoutPref.video_layout[layout_cnt] ==
            'undefined' &&
          layout_cnt >= 0
        ) {
          layout_cnt = layout_cnt - 1;
        }
        $scope.videoLayoutPref.layout_num = layout_cnt + 1;
      }
    };

    $scope.cmd = 'cmd';
    $scope.Result = 'result';
    $scope.sendData = function () {
      $scope.videoLayoutPref.cmd_type = 'set';
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
    };
    $scope.getData = function () {
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('get data');
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
          if (typeof resp.data.videoLayoutPref != 'undefined') {
            $scope.videoLayoutPref = resp.data;
            $scope.Result = resp;
          }
          $scope.cmdStatus = '';
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_VIDEO_LAYOUT_CONF');
          console.log(resp);
          $scope.cmdStatus = 'Get setting error!';
          //$scope.Result = "Post Error";
        }
      );
    };

    $scope.getData();
    //console.log($scope.resTable);
    //console.log($scope.video_option_total_stream_list);
  },
]);
