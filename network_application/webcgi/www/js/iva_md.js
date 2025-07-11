app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.IvaDropDown = setIvaDropdown(Iva_Index.md);
    //$scope.IvaMenu = setIvaPage(Iva_Index.md);
    $scope.CurPage = msg.Motion_Detection;
    $scope.msg_t = {
      augentix: ['Augentix', '多方科技', '多方科技'],
      noTxt: ['', '', ''],
      Auto: ['Auto', '自動', '自动'],
      Read_setting: ['Read out from device', '讀取設定', '读取设定'],
      Apply: ['Apply', '應用', '应用'],
      Reboot: ['Reboot', '重新開機', '重新开机'],
      Switch2SysupdOS: ['System Update', '系統更新', '系统更新'],
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
      Loading: ['Loading...', '讀取中...', '读取中...'],
      NoRespond: ['Device no respond!!', '設備無反應!!', '设备无反应!!'],
      WriteOK: ['Write success!! ', '寫入成功!!', '写入成功'],
      ReadFail: ['Read failed!!', '讀取失敗!!', '读取失败!!'],
      WriteFail: ['Write failed!! ', '寫入失敗!!', '写入失败'],
      Sensitivity: ['Sensitivity:', '敏感度:', '敏感度:'],
      Start: ['Start', '起始', '起始'],
      End: ['End', '結束', '结束'],
      Width: ['Width', '寬度', '宽度'],
      Height: ['Height', '高度', '高度'],
      Coord: ['Coordinate', '座標', '座标'],
      Tamper_Detection: ['Tamper Detection', '入侵檢測', '入侵检测'],
      Motion_Detection: ['Motion Detection', '移動檢測', '移动检测'],
      Automatic_ROI: ['Automatic ROI', '自動目標檢測', '自动目标检测'],
      LightOnOff_Detection: [
        'LightOnOff Detection',
        '開關燈檢測',
        '开关灯检测',
      ],
      Edge_AI_Framework: [
        'Edge AI Framework',
        '邊緣人工智慧應用',
        '边缘人工智慧应用',
      ],
      Object_Detection: ['Object Detection', '物體檢測', '物体检测'],
      Pedestrian_Detection: ['Pedestrian Detection', '行人檢測', '行人检测'],
      Regional_Motion_Sensor: [
        'Regional Motion Sensor',
        '區域運動傳感器',
        '区域运动传感器',
      ],
      Electric_Fence: ['Electric Fence', '電子圍離', '电子围篱'],
      Enable_motion_detection: [
        'Enable motion detection',
        '開啓移動檢測',
        '开启移动检测',
      ],
      Active_regions: ['Active regions :', '有效區域數:', '有效区域数:'],
      Region: ['Region', '區域', '区域'],
      Enable_skip_shake: [
        'Skip shaking object',
        '無視抖動中物體',
        '无视抖动中物体',
      ],
      Color: ['Color', '顏色', '颜色'],
      Thickness: ['Thickness', '框寬', '框宽'],
      Grid: ['Grid', '柵格', '栅格'],
      Rectangle_Position: ['Rectangle Position', '框的位置', '框的位置'],
      Rectangle_Editor: ['Rectangle Editor', '編輯框', '编辑框'],
      Active_Markers: ['Active Markers', '激活遮擋', '激活遮挡'],
      Setup_Markers: ['Setup Markers', '使用遮擋', '使用遮挡'],
      Clear_Markers: ['Clear Markers', '清除遮擋', '清除遮挡'],
      Draw_Rectangle: ['Draw Rectangle', '畫矩形', '画矩形'],
    };

    $scope.mdPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_MD_CONF,
      cmd_type: 'set',
      rgn_cnt: 1,
      en_skip_shake: 1,
      max_spd: 0,
      /* metadata reference from region 0 */
      /* 定值 */
      sens: 255,
      /* 連結到 sens */
      min_spd: 10,
      /* 定值 */
      mode: 'ENERGY',
      rgn_list: [
        /* 定值 */ {
          ex: 10,
          /* 值域 0-100，表示畫面的百分比 */
          ey: 10,
          /* 值域 0-100，表示畫面的百分比 */
          id: 0,
          /* 表示第幾個 region */
          sx: 0,
          /* 值域 0-100，表示畫面的百分比 */
          sy: 0,
          /* 值域 0-100，表示畫面的百分比 */
          max_spd: 255,
          /* 定值 */
          sens: 100,
          /* 連結到 sens */
          min_spd: 5,
          /* 定值 */
          mode: 'ENERGY',
          /* 定值 */
        },
      ],
      enabled: 1,
      /* 連結到 enabled */
      video_chn_idx: 0,
      /* 定值 */
    };
    var vlc;
    var itemId;


    $scope.canApply = false;
    $scope.accept_line = function (context, x1, y1, x2, y2) {
      line_x1.value = x1;
      line_y1.value = y1;
      line_x2.value = x2;
      line_y2.value = y2;
    };

    $scope.accept_rectangle = function (context, x, y, width, height) {
      rectangle_x.value = x;
      rectangle_y.value = y;
      rectangle_width.value = width;
      rectangle_height.value = height;
    };

    $scope.accept_polygon = function (context, points) {
      var pts = [];
      for (var i = 0; i < points.length; ++i) pts[i] = points[i];
      polygon_points.value = JSON.stringify(pts);
    };

    $scope.start_drawing = function (shape) {
      console.log(
        'thickness="%s", <%s>',
        drawing_thickness.value,
        typeof drawing_thickness.value
      );
      /* var OSD1 = document.getElementById("playerPlugin");
		console.log(OSD1); */
      var OSD = document.getElementById('playerPlugin').content.OSD;
      console.log('hello world !!! hello World！！！');
      var thickness = parseFloat(drawing_thickness.value);
      var grid = parseInt(drawing_grid.value);
      if (isNaN(thickness)) {
        thickness = 1;
        drawing_thickness.value = thickness;
      }
      if (isNaN(grid)) {
        grid = 1;
        drawing_grid.value = grid;
      }
      var callback = null;
      if (shape == 'line') callback = accept_line;
      else if (shape == 'rectangle' || shape == '*rectangle')
        callback = $scope.accept_rectangle;
      else if (shape == 'polygon' || shape == '*polygon')
        callback = accept_polygon;

      if (
        !OSD.StartDrawing(
          shape,
          drawing_color.value,
          thickness,
          grid,
          null,
          callback
        )
      ) {
        alert('User drawing is NOT initiated!');
      }
      console.log('Ending!!');
    };

    $scope.cancel_drawing = function () {
      document.getElementById('playerPlugin').content.OSD.CancelDrawing();
    };

    $scope.is_array = function (obj) {
      return 'push' in obj && typeof obj.push == 'function';
    };

    $scope.do_if_array = function (target, task) {
      if ($scope.is_array(target)) {
        task();
      }
    };

    var rx;
    var ry;
    var rwidth;
    var rheight;
    $scope.testnumber = 3;
    $scope.append_rectangle_marker = function () {
      rx = parseFloat(rectangle_x.value);
      ry = parseFloat(rectangle_y.value);
      rwidth = parseFloat(rectangle_width.value);
      rheight = parseFloat(rectangle_height.value);
      if (isNaN(rx) || isNaN(ry) || isNaN(rwidth) || isNaN(rheight)) return;
      console.log('modifying markers editor...');
      var markers = JSON.parse(markers_editor.value);
      $scope.do_if_array(markers, function () {
        rectangle_x.value = rectangle_y.value = rectangle_width.value = rectangle_height.value =
          '';
        markers.push({
          shape: 'rectangle',
          x: rx,
          y: ry,
          width: rwidth,
          height: rheight,
          fill: drawing_color.value,
        });
        markers_editor.value = JSON.stringify(markers, null, 2);
        markers_editor.scrollIntoView(false);
      });
      console.log('End modification.');
    };

    $scope.reset_markers_editor = function (as_active_markers) {
      if (as_active_markers) {
        markers_editor.value = active_markers.value;
      } else {
        markers_editor.value = '[]';
      }
    };

    $scope.update_active_markers = function () {
      var OSD = document.getElementById('playerPlugin').content.OSD;
      var valid_markers = OSD.Markers;
      var markers = [];
      for (var i = 0; i < valid_markers.length; ++i) {
        markers[i] = valid_markers[i];
      }

      active_markers.value = JSON.stringify(markers, null, 2);
    };

    var chnn = 0;
    $scope.testbind = 'hello this is test bind!!';
    $scope.setup_markers_from_editor = function () {
      var OSD = document.getElementById('playerPlugin').content.OSD;
      try {
        var markers = JSON.parse(markers_editor.value);
        /* console.log(markers); */
        if (chnn >= 0 && chnn < 64) {
          $scope.mdPref.rgn_list[chnn].id = chnn;
          $scope.mdPref.rgn_list[chnn].sx = Math.round((rx / 1080) * 100);
          $scope.mdPref.rgn_list[chnn].sy = Math.round((ry / 720) * 100);
          $scope.mdPref.rgn_list[chnn].ex = Math.round(
            ((rx + rwidth) / 1080) * 100
          );
          $scope.mdPref.rgn_list[chnn].ey = Math.round(
            ((ry + rheight) / 720) * 100
          );
        }
        chnn++;
        $scope.mdPref.rgn_list.sx = Math.round((rx / 1080) * 100);
        console.log($scope.rgn_list.sx);
        $scope.testbind = 'testbind modified in setup markers!!';
        console.log($scope.testbind);
        console.log('only vaf rx:' + rx);
        console.log('setup markers from editor ey:');
        console.log($scope.mdPref.rgn_list.ey);
        console.log('scope rheight:' + rheight);
        $scope.do_if_array(markers, function () {
          /* console.log(markers); */
          /* OSD.SetupMarkers(markers); */

          $scope.update_active_markers();
          plugin.scrollIntoView();
        });
      } catch (e) {
        console.log('Ending!!');
      }
    };

    $scope.setup_rectangle = function () {
      $scope.append_rectangle_marker();
      $scope.setup_markers_from_editor();
    };

    $scope.clear_rectangle = function () {
      $scope.reset_markers_editor();
      $scope.setup_markers_from_editor();
    };

    $scope.init = function () {
      window.drawing_color = document.getElementById('drawing_color');
      window.drawing_thickness = document.getElementById('drawing_thickness');
      window.drawing_grid = document.getElementById('drawing_grid');
      window.line_x1 = document.getElementById('line_x1');
      window.line_y1 = document.getElementById('line_y1');
      window.line_x2 = document.getElementById('line_x2');
      window.line_y2 = document.getElementById('line_y2');
      window.rectangle_x = document.getElementById('rectangle_x');
      window.rectangle_y = document.getElementById('rectangle_y');
      window.rectangle_width = document.getElementById('rectangle_width');
      window.rectangle_height = document.getElementById('rectangle_height');
      window.polygon_points = document.getElementById('polygon_points');
      window.markers_editor = document.getElementById('markers_editor');
      window.active_markers = document.getElementById('active_markers');
      window.plugin = document.getElementById('playerPlugin');
    };

    $scope.Result = 'result';
    $scope.check_coordinate = function () {
      for (i = 0; i < $scope.mdPref.rgn_list.length; i++) {
        if (
          $scope.mdPref.rgn_list[i].sx < 0 ||
          typeof $scope.mdPref.rgn_list[i].sx != 'number'
        )
          $scope.mdPref.rgn_list[i].sx = 0;
        if ($scope.mdPref.rgn_list[i].sx > 99)
          $scope.mdPref.rgn_list[i].sx = 99;
        if (
          $scope.mdPref.rgn_list[i].sy < 0 ||
          typeof $scope.mdPref.rgn_list[i].sy != 'number'
        )
          $scope.mdPref.rgn_list[i].sy = 0;
        if ($scope.mdPref.rgn_list[i].sy > 99)
          $scope.mdPref.rgn_list[i].sy = 99;
        if (
          $scope.mdPref.rgn_list[i].ex < 1 ||
          typeof $scope.mdPref.rgn_list[i].ex != 'number'
        )
          $scope.mdPref.rgn_list[i].ex = 1;
        if ($scope.mdPref.rgn_list[i].ex > 100)
          $scope.mdPref.rgn_list[i].ex = 100;
        if (
          $scope.mdPref.rgn_list[i].ey < 1 ||
          typeof $scope.mdPref.rgn_list[i].ey != 'number'
        )
          $scope.mdPref.rgn_list[i].ey = 1;
        if ($scope.mdPref.rgn_list[i].ey > 100)
          $scope.mdPref.rgn_list[i].ey = 100;
        if ($scope.mdPref.rgn_list[i].sx >= $scope.mdPref.rgn_list[i].ex) {
          $scope.mdPref.rgn_list[i].sx = $scope.mdPref.rgn_list[i].ex - 1;
        }
        if ($scope.mdPref.rgn_list[i].sy >= $scope.mdPref.rgn_list[i].ey) {
          $scope.mdPref.rgn_list[i].sy = $scope.mdPref.rgn_list[i].ey - 1;
        }
      }
    };
    $scope.region_cnt_change = function () {
      if ($scope.mdPref.rgn_cnt < 1 || typeof $scope.mdPref.rgn_cnt != 'number')
        $scope.mdPref.rgn_cnt = 1;
      if ($scope.mdPref.rgn_cnt > 63) $scope.mdPref.rgn_cnt = 63;

      if ($scope.mdPref.rgn_cnt > $scope.mdPref.rgn_list.length) {
        // Add array
        var len = $scope.mdPref.rgn_list.length;
        var reg_cnt = $scope.mdPref.rgn_cnt - len;
        var i;
        for (i = 0; i < reg_cnt; i++) {
          $scope.mdPref.rgn_list.push({
            id: len + i,
            sx: 0,
            sy: 0,
            ex: 10,
            ey: 10,
            max_spd: 255,
            sens: 100,
            min_spd: 5,
            mode: 'ENERGY',
          });
        }
      }
      if ($scope.mdPref.rgn_cnt < $scope.mdPref.rgn_list.length) {
        // Remove array
        len = $scope.mdPref.rgn_list.length;
        reg_cnt = len - $scope.mdPref.rgn_cnt;
        for (i = 0; i < reg_cnt; i++) {
          $scope.mdPref.rgn_list.pop();
        }
      }
    };

    $scope.sendData = function () {
      $scope.check_coordinate();
      $scope.mdPref.cmd_type = 'set';
      if ($scope.mdPref.rgn_cnt > 0) {
        $scope.mdPref.max_spd = $scope.mdPref.rgn_list[0].max_spd;
        $scope.mdPref.min_spd = $scope.mdPref.rgn_list[0].min_spd;
        $scope.mdPref.sens = $scope.mdPref.rgn_list[0].sens;
        $scope.mdPref.mode = $scope.mdPref.rgn_list[0].mode;
      }
      if ($scope.mdPref.enabled) $scope.mdPref.enabled = 1;
      else $scope.mdPref.enabled = 0;
      if ($scope.mdPref.en_skip_shake) $scope.mdPref.en_skip_shake = 1;
      else $scope.mdPref.en_skip_shake = 0;
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.mdPref.enabled = $scope.mdPref.enabled == 1;
      $scope.mdPref.en_skip_shake = $scope.mdPref.en_skip_shake == 1;

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.mdPref, function (key, value) {
          if (key === '$$hashKey') {
            return undefined;
          }
          return value;
        }),
      }).then(
        function (resp) {
          console.log('post success');
          $scope.Result = resp;
          if (resp.data.rval == 0) {
            $scope.cmdStatus = $scope.msg_t.WriteOK[$scope.lang];
          } else {
            $scope.cmdStatus =
              $scope.msg_t.WriteFail[$scope.lang] + ' rval=' + resp.data.rval;
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.WriteFail[$scope.lang];
          //	$scope.Result = "Post Error";
        }
      );
    };
    $scope.getData = function (hide_msg) {

      $scope.canApply = false;
      $scope.cmdStatus = $scope.msg_t.Loading[$scope.lang];
      $scope.cmd = '/cgi-bin/msg.cgi';
      console.log('get data');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify({
          "master_id": 1,
          "cmd_id": Cmd.AGTX_CMD_MD_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_MD_CONF');
          $scope.mdPref = resp.data;
          /* 				if (chnn >= 0 && chnn < 64) {
									$scope.mdPref.rgn_list.id = chnn;
									$scope.mdPref.rgn_list.sx = rx / 1080 * 100;
									$scope.mdPref.rgn_list.sy = ry / 720 * 100;
									$scope.mdPref.rgn_list.ex = (rx + rwidth) / 1080 * 100;
									$scope.mdPref.rgn_list.ey = (ry + rheight) / 720 * 100;
								} */
          $scope.mdPref.enabled = $scope.mdPref.enabled == 1;
          $scope.mdPref.en_skip_shake = $scope.mdPref.en_skip_shake == 1;
          $scope.region_cnt_change();
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
          console.log('post error: ' + 'AGTX_CMD_MD_CONF');
          console.log(resp);
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getData(1);
    window.langId = $scope.lang;
  },
]);
