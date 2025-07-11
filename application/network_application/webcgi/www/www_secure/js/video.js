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
      Loading: ['Loading...', '讀取中...', '读取中...'],
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
      Global_setting: ['Global setting', '整體設置', '整体设置'],
    };
    $scope.VideoMenu = setPage(Video_Index.video, Video_Index, VideoMenu);
    $scope.Result1 = '';
    $scope.bitrate = [
      {
        cbr: 2000,
        vbr: 1999,
        sbr: 1998,
      },
      {
        cbr: 1997,
        vbr: 1996,
        sbr: 1995,
      },
      {
        cbr: 1994,
        vbr: 1993,
        sbr: 1992,
      },
      {
        cbr: 1991,
        vbr: 1990,
        sbr: 1989,
      },
      {
        cbr: 1988,
        vbr: 1987,
        sbr: 1986,
      },
      {
        cbr: 1985,
        vbr: 1984,
        sbr: 1983,
      },
      {
        cbr: 1982,
        vbr: 1981,
        sbr: 1980,
      },
      {
        cbr: 1979,
        vbr: 1978,
        sbr: 1977,
      },
    ];
    $scope.cbr_max = 16000;
    $scope.vbr_min = 32;
    $scope.vbr_max = 16000;
    $scope.sbr_max = 16000;
    $scope.LessOrEq = function (val) {
      return function (item) {
        return item <= val;
      };
    };

    var Codec = {
      H264: 0,
      H265: 1,
      MJPEG: 2,
    };
    var Profile = {
      BASELINE: 0,
      MAIN: 1,
      HIGH: 2,
    };

    

    $scope.video_res_option = [
    {
      res_idx:-1,
      res: [
        {
          width: 3840,
          height: 1080,
          max_frame_rate: [25, 20, 0, 0, 0, 0, 0, 0],
          frame_rate_list: [
            25,
            24,
            23,
            22,
            21,
            20,
            19,
            18,
            17,
            16,
            15,
            14,
            13,
            12,
            11,
            10,
            9,
            8,
            7,
            6,
            5,
            4,
            3,
            2,
            1,
          ],
        },
        {
          width: 2560,
          height: 720,
          max_frame_rate: [25, 25, 0, 0, 0, 0, 0, 0],
          frame_rate_list: [
            25,
            24,
            23,
            22,
            21,
            20,
            19,
            18,
            17,
            16,
            15,
            14,
            13,
            12,
            11,
            10,
            9,
            8,
            7,
            6,
            5,
            4,
            3,
            2,
            1,
          ],
        },
        {
          width: 1920,
          height: 544,
          max_frame_rate: [25, 25, 0, 0, 0, 0, 0, 0],
          frame_rate_list: [
            25,
            24,
            23,
            22,
            21,
            20,
            19,
            18,
            17,
            16,
            15,
            14,
            13,
            12,
            11,
            10,
            9,
            8,
            7,
            6,
            5,
            4,
            3,
            2,
            1,
          ],
        },
      ],
    },
    {
      res_idx:-1,
      res: [
        {
          width: 960,
          height: 272,
          max_frame_rate: [0, 10, 0, 0, 0, 0, 0, 0],
          frame_rate_list: [10, 9, 8, 7, 6, 5, 4, 3, 2, 1],
        },
        {
          width: 640,
          height: 184,
          max_frame_rate: [0, 10, 0, 0, 0, 0, 0, 0],
          frame_rate_list: [10, 9, 8, 7, 6, 5, 4, 3, 2, 1],
        },
      ],
    }
    ];

    $scope.video_venc_option = [
      {
        venc: [
        {
          codec: 'H264',
          profile: ['BASELINE', 'MAIN', 'HIGH'],
          rc_mode: ['VBR', 'CBR', 'SBR', 'CQP'],
          min_bit_rate: 64,
          max_bit_rate: 4096,
          cbr_param: {
            min_q_factor: 1,
            max_q_factor: 100,
          },
          vbr_param: {
            min_quality_range: 0,
            max_quality_range: 0,
          },
          min_gop_size: 1,
          max_gop_size: 120,
        },
        {
          codec: 'H265',
          profile: ['BASELINE', 'MAIN', 'HIGH'],
          rc_mode: ['VBR', 'CBR', 'SBR', 'CQP'],
          min_bit_rate: 64,
          max_bit_rate: 4096,
          cbr_param: {
            min_q_factor: 1,
            max_q_factor: 100,
          },
          vbr_param: {
            min_quality_range: 0,
            max_quality_range: 0,
          },
          min_gop_size: 1,
          max_gop_size: 120,
        },
        {
          codec: 'MJPEG',
          profile: ['DEFAULT'],
          rc_mode: ['VBR', 'CBR', 'SBR', 'CQP'],
          min_bit_rate: 64,
          max_bit_rate: 4096,
          cbr_param: {
            min_q_factor: 1,
            max_q_factor: 100,
          },
          vbr_param: {
            min_quality_range: 0,
            max_quality_range: 0,
          },
          min_gop_size: 1,
          max_gop_size: 120,
        },
      ],
      },
      {
        venc: [
          {
            codec: 'H264',
            profile: [
              "BASELINE",
              "MAIN",
              "HIGH"
            ],
            rc_mode: [
              "CBR",
              "VBR",
              'SBR',
              'CQP'
            ],
            min_bit_rate: 32,
            max_bit_rate: 8000,
            cbr_param: {
              "min_q_factor": 1,
              "max_q_factor": 100
            },
            vbr_param: {
              "min_quality_range": 0,
              "max_quality_range": 0
            },
            min_gop_size: 1,
            max_gop_size: 200
          },
          {
            codec: "H265",
            profile: [
              "MAIN"
            ],
            rc_mode: [
              "CBR",
              "VBR",
              'SBR',
              'CQP'
            ],
            min_bit_rate: 32,
            max_bit_rate: 8000,
            cbr_param: {
              "min_q_factor": 1,
              "max_q_factor": 100
            },
            vbr_param: {
              "min_quality_range": 0,
              "max_quality_range": 0
            },
            min_gop_size: 1,
            max_gop_size: 200
          },
          {
            codec: "MJPEG",
            profile: [
              "NONE"
            ],
            rc_mode: [
              "CBR",
              "VBR",
              'SBR',
              'CQP'
            ],
            min_bit_rate: 32,
            max_bit_rate: 8000,
            cbr_param: {
              "min_q_factor": 1,
              "max_q_factor": 100
            },
            vbr_param: {
              "min_quality_range": 0,
              "max_quality_range": 0
            },
            min_gop_size: 1,
            max_gop_size: 200
          }
        ]
      }
    ];

    $scope.video = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_VIDEO_STRM_CONF,
      cmd_type: 'set',
      video_dev_idx: 0,
      video_strm_cnt: 3,
      video_strm_list: [
        {
          video_strm_idx: 0,
          strm_en: 1,
          width: 3840,
          height: 1080,
          output_fps: 20,
          rotate: 0,
          mirr_en: 0,
          flip_en: 0,
          venc_type: 0,
          venc_profile: 0,
          rc_mode: 1,
          gop_size: 20,
          bit_rate: 4096,
          min_qp: 10,
          max_qp: 46,
          min_q_factor : 10,
          max_q_factor : 90,
          fluc_level: 0,
          scene_smooth: 0,
          regression_speed: 2,
          i_continue_weight: 0,
          i_qp_offset: -2,
          vbr_max_bit_rate: 4096,
          vbr_quality_level_index: 4,
          sbr_adjust_br_thres_pc: 70,
          sbr_adjust_step_times: 20,
          sbr_converge_frame: 5,
          cqp_i_frame_qp: 20,
          cqp_p_frame_qp: 28,
          obs: 0,
        },
        {
          video_strm_idx: 1,
          strm_en: 1,
          width: 960,
          height: 272,
          output_fps: 10,
          rotate: 0,
          mirr_en: 0,
          flip_en: 0,
          venc_type: 0,
          venc_profile: 0,
          rc_mode: 1,
          gop_size: 10,
          bit_rate: 800,
          min_qp: 10,
          max_qp: 46,
          min_q_factor : 10,
          max_q_factor : 90,
          fluc_level: 0,
          scene_smooth: 0,
          regression_speed: 2,
          i_continue_weight: 0,
          i_qp_offset: -2,
          vbr_max_bit_rate: 800,
          vbr_quality_level_index: 4,
          sbr_adjust_br_thres_pc: 70,
          sbr_adjust_step_times: 20,
          sbr_converge_frame: 5,
          cqp_i_frame_qp: 20,
          cqp_p_frame_qp: 28,
          obs: 0,
        },
      ],
    };
    $scope.is_two_strm = false;
    $scope.venc_idx = [0, 0, 0, 0, 0, 0, 0, 0];
    $scope.res_idx = [0, 3, 3, 3];
    $scope.iq_idx = [0, 0];
    $scope.total_stream = 1;
    $scope.video_option_total_stream = $scope.video_res_option.length;
    $scope.video_option_total_stream_list = [];
    $scope.get_total_stram = function () {
      $scope.total_stream = 8;
      if ($scope.total_stream > $scope.video_res_option.length) {
        $scope.total_stream = $scope.video_res_option.length;
      }
      $scope.video_option_total_stream_list = [];
      $scope.video_option_total_stream = $scope.video_res_option.length;
      if ($scope.total_stream > $scope.video.video_strm_list.length) {
        $scope.total_stream = $scope.video.video_strm_list.length;
      }
      for (i = 0; i < $scope.total_stream; i++) {
        $scope.video_option_total_stream_list.push(i + 1);
      }
      $scope.tmp = 0;
      for (i = 0; i < $scope.total_stream; i++) {
        console.log($scope.video.video_strm_list[i].strm_en);
        if ($scope.video.video_strm_list[i].strm_en) {
          $scope.tmp = i + 1;
        }
      }
      console.log($scope.tmp);
      if ($scope.total_stream > $scope.tmp) $scope.total_stream = $scope.tmp;
    };

    $scope.getMaxFps = function ($index) {
      var res_idx = $scope.res_idx;
      var total_stream = $scope.total_stream;
      if ($scope.video_res_option[$index].res[res_idx[$index]].max_frame_rate[total_stream -1] !=0){
        return $scope.video_res_option[$index].res[res_idx[$index]].max_frame_rate[total_stream -1];
      } else if ($scope.video_res_option[$index].res[res_idx[$index]].max_frame_rate[total_stream -1] == 0) {
        return $scope.video_res_option[$index].res[res_idx[$index]].max_frame_rate[total_stream -2];
      }
    }


    $scope.getValidFps = function ($index) {
      var res_idx = $scope.res_idx;
      var MaxFrame = $scope.getMaxFps($index);
      var listCount = 0;
      for (i = 0; i < $scope.video_res_option[$index].res[res_idx[$index]].frame_rate_list.length; i++) {
        if ($scope.video_res_option[$index].res[res_idx[$index]].frame_rate_list[i] <= MaxFrame) {
          listCount++;
        }
      }
      return listCount;
    }

    $scope.imgQualTable = [
      {
        min_qp: 10,
        max_qp: 46,
      },
      {
        min_qp: 10,
        max_qp: 38,
      },
      {
        min_qp: 10,
        max_qp: 30,
      },
      {
        min_qp: 8,
        max_qp: 20,
      },
      {
        min_qp: 6,
        max_qp: 12,
      },
    ];
    $scope.rc_mode_map = ['VBR', 'CBR', 'SBR', 'CQP'];
    $scope.lookup_rc_mode = function (x) {
      for (i = 0; i < $scope.rc_mode_map.length; i++) {
        if (x == $scope.rc_mode_map[i]) return i;
      }
    };
    $scope.profile_map = ['BASELINE', 'MAIN', 'HIGH'];
    $scope.lookup_profile = function (x) {
      for (i = 0; i < $scope.profile_map.length; i++) {
        if (x == $scope.profile_map[i]) return i;
      }
    };
    $scope.codec_map = ['H264', 'H265', 'MJPEG', 'JPEG'];
    $scope.lookup_codec = function (x) {
      for (i = 0; i < $scope.codec_map.length; i++) {
        if (x == $scope.codec_map[i]) return i;
      }
    };
    $scope.SetToScreen = function () {
      console.log('Video stream lenght:' + $scope.video.video_strm_list.length)
      idx = 99;
      for (i = 0; i < $scope.video.video_strm_list.length; i++) {
        $scope.bitrate[i].cbr = $scope.video.video_strm_list[i].bit_rate;
        $scope.bitrate[i].vbr =
          $scope.video.video_strm_list[i].vbr_max_bit_rate;
      }
      for (i = 0; i < $scope.video.video_strm_list.length; i++) {
        for (idx = 0; idx < $scope.video_res_option[i].res.length; idx = idx + 1) {
          if (
            $scope.video.video_strm_list[i].width ==
              $scope.video_res_option[i].res[idx].width &&
            $scope.video.video_strm_list[i].height ==
              $scope.video_res_option[i].res[idx].height
          ) {
            $scope.res_idx[i] = idx;
          }
        }
      }
      $scope.total_stream = 1;
      for (i = 1; i < $scope.video.video_strm_list.length; i++) {
        if ($scope.video.video_strm_list[i].strm_en) {
          $scope.total_stream = i + 1;
        }
      }
      $scope.get_total_stram();
    };
    $scope.change_stream_cnt = function () {
      for (i = 0; i < $scope.total_stream; i++) {
        if (
          $scope.video_res_option[i].res[$scope.res_idx[i]].max_frame_rate[
            $scope.total_stream - 1
          ] < $scope.video.video_strm_list[i].output_fps &&
          i < $scope.total_stream - 1
        ) {
          $scope.video.video_strm_list[i].output_fps =
            $scope.video_res_option[i].res[$scope.res_idx[i]].max_frame_rate[i];
        }
        if ($scope.video.video_strm_list[i].output_fps < 1) {
          $scope.video.video_strm_list[i].output_fps = 1;
        }
      }
    };
    $scope.ScreenToSet = function () {
      for (i = 0; i < $scope.video.video_strm_list.length; i++) {
        $scope.video.video_strm_list[i].bit_rate = $scope.bitrate[i].cbr;
        $scope.video.video_strm_list[i].vbr_max_bit_rate =
          $scope.bitrate[i].vbr;
        $scope.video.video_strm_list[i].flip_en =
          $scope.video.video_strm_list[0].flip_en;
        $scope.video.video_strm_list[i].mirr_en =
          $scope.video.video_strm_list[0].mirr_en;
      }
      for (i = 0; i < $scope.video.video_strm_list.length; i++) {
        if (i < $scope.total_stream) {
          $scope.video.video_strm_list[i].strm_en = 1;
        } else {
          $scope.video.video_strm_list[i].strm_en = 0;
        }
      }
      for (i = 0; i < $scope.total_stream; i++) {
        $scope.video.video_strm_list[i].width =
          $scope.video_res_option[i].res[$scope.res_idx[i]].width;
        $scope.video.video_strm_list[i].height =
          $scope.video_res_option[i].res[$scope.res_idx[i]].height;
      }
      $scope.get_total_stram();
    };

    $scope.set_default_profile = function ($index) {
      if ($scope.video.video_strm_list[$index].venc_type == Codec.H264) {
        $scope.video.video_strm_list[$index].venc_profile = Profile.HIGH;
      } else if ($scope.video.video_strm_list[$index].venc_type == Codec.H265) {
        $scope.video.video_strm_list[$index].venc_profile = Profile.MAIN;
      } else if (
        $scope.video.video_strm_list[$index].venc_type == Codec.MJPEG
      ) {
        $scope.video.video_strm_list[$index].venc_profile = Profile.HIGH;
      }
    };

    $scope.setDefaultFps = function ($index) {
      $scope.video.video_strm_list[$index].output_fps = $scope.getMaxFps($index);
    };

    $scope.cmd = 'cmd';
    $scope.Result = 'result';
    $scope.sendData = function () {
      $scope.video.cmd_type = 'set';
      $scope.ScreenToSet();
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.SetToScreen();
      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
          'Cache-Control': 'no-cache',
        },
        data: JSON.stringify($scope.video),
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
      $scope.canApply = false;
      $scope.cmdStatus = $scope.msg_t.Loading[$scope.lang];
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
          "cmd_id": Cmd.AGTX_CMD_VIDEO_STRM_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_VIDEO_STRM_CONF');
          if (typeof resp.data.video_strm_list != 'undefined') {
            $scope.video = resp.data;
            $scope.canApply = true;
            $scope.SetToScreen();
            $scope.Result = resp;
          }
          $scope.cmdStatus = '';
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_VIDEO_STRM_CONF');
          $scope.canApply = true;
          console.log(resp);
          $scope.cmdStatus = 'Get setting error!';
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getOption = function () {
      console.log('show video_res_option');
      console.log($scope.video_res_option);
      console.log('show video_res_option end');
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
          "cmd_id": Cmd.AGTX_CMD_RES_OPTION,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getoption success: ' + 'AGTX_CMD_RES_OPTION');
          console.log(resp);
          if (typeof resp.data.strm != 'undefined')
            $scope.video_res_option = resp.data.strm;
          $scope.Result = resp;
          $scope.cmdStatus = '';
          $scope.get_total_stram();
                console.log('show video_res_option');
      console.log($scope.video_res_option);
      console.log('show video_res_option end');
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_RES_OPTION');
          console.log(resp);
          $scope.cmdStatus = 'Get video res option error!';
          //$scope.Result = "Post Error";
        }
      );

      console.log('show video_venc_option');
      console.log($scope.video_venc_option);
      console.log('show video_venc_option end');
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
          "cmd_id": Cmd.AGTX_CMD_VENC_OPTION,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getoption success: ' + 'AGTX_CMD_VENC_OPTION');
          console.log(resp);
          if (typeof resp.data.strm != 'undefined')
            $scope.video_venc_option = resp.data.strm;
          $scope.Result = resp;
          $scope.cmdStatus = '';
          $scope.get_total_stram();
                console.log('how video_venc_option');
      console.log($scope.video_venc_option);
      console.log('show video_venc_option end');
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_VENC_OPTION');
          console.log(resp);
          $scope.cmdStatus = 'Get video venc option error!';
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.SetToScreen();
    $scope.getOption();
    $scope.getData();
    $scope.get_total_stram();
    //console.log($scope.resTable);
    //console.log($scope.video_option_total_stream_list);
  },
]);
