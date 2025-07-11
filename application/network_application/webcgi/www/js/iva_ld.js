app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.IvaDropDown = setIvaDropdown(Iva_Index.ld);
    //$scope.IvaMenu = setIvaPage(Iva_Index.ld);
    $scope.CurPage = msg.LightOnOff_Detection;
    $scope.msg_t = {
      augentix: ['Augentix', '多方科技', '多方科技'],
      noTxt: ['', '', ''],
      Auto: ['Auto', '自動', '自动'],
      Read_setting: ['Read out from device', '讀取設定', '读取设定'],
      Apply: ['Apply', '應用', '应用'],
      Loading: ['Loading...', '讀取中...', '读取中...'],
      Reboot: ['Reboot', '重新開機', '重新开机'],
      Switch2SysupdOS: ['System Update', '系統更新', '系统更新'],
      ReadOK: ['Read success!!', '讀取成功!!', '读取成功!!'],
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
      Enable_lightonoff_detection: [
        'Enable Light-On-Off detection',
        '開啓入侵檢測',
        '开启入侵检测',
      ],
      Region: ['Region', '區域', '区域'],
      Trigger_cond: ['Trigger Cond', '激活條件', '激活条件'],
      Light_on: ['LIGHT_ON', '開燈', '开灯'],
      Light_off: ['LIGHT_OFF', '關燈', '关灯'],
      Light_both: ['BOTH', '同時', '同时'],
      Color: ['Color', '顏色', '颜色'],
      Thickness: ['Thickness', '框寬', '框宽'],
      Grid: ['Grid', '柵格', '栅格'],
      Rectangle_Position: ['Rectangle Position', '框的位置', '框的位置'],
      Rectangle_Editor: ['Rectangle Editor', '編輯框', '编辑框'],
      Active_Markers: ['Active Markers', '激活遮擋', '激活遮挡'],
      Setup_Markers: ['Setup Markers', '使用遮擋', '使用遮挡'],
      Clear_Markers: ['Clear Markers', '清除遮擋', '清除遮挡'],
      Draw_Rectangle: ['Draw Region', '畫區域', '画区域'],
    };

    $scope.ldPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_LD_CONF,
      cmd_type: 'set',
      enabled: 0,
      sensitivity: 80,
      det_region: {
        start_x: 0,
        start_y: 0,
        end_x: 100,
        end_y: 100,
      },
      trigger_cond: 'BOTH',
    };
    
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
        $scope.ldPref.det_region.start_x = Math.round((rx / 1080) * 100);
        $scope.ldPref.det_region.start_y = Math.round((ry / 720) * 100);
        $scope.ldPref.det_region.end_x = Math.round(
          ((rx + rwidth) / 1080) * 100
        );
        $scope.ldPref.det_region.end_y = Math.round(
          ((ry + rheight) / 720) * 100
        );
        $scope.do_if_array(markers, function () {
          $scope.update_active_markers();
          plugin.scrollIntoView();
        });
      } catch (e) {
        console.log('update_active_markers error.');
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

    $scope.check_coordinate = function () {
      if (
        $scope.ldPref.det_region.start_x < 0 ||
        typeof $scope.ldPref.det_region.start_x != 'number'
      )
        $scope.ldPref.det_region.start_x = 0;
      if ($scope.ldPref.det_region.start_x > 99)
        $scope.ldPref.det_region.start_x = 99;
      if (
        $scope.ldPref.det_region.start_y < 0 ||
        typeof $scope.ldPref.det_region.start_y != 'number'
      )
        $scope.ldPref.det_region.start_y = 0;
      if ($scope.ldPref.det_region.start_y > 99)
        $scope.ldPref.det_region.start_y = 99;
      if (
        $scope.ldPref.det_region.end_x < 1 ||
        typeof $scope.ldPref.det_region.end_x != 'number'
      )
        $scope.ldPref.det_region.end_x = 1;
      if ($scope.ldPref.det_region.end_x > 100)
        $scope.ldPref.det_region.end_x = 100;
      if (
        $scope.ldPref.det_region.end_y < 1 ||
        typeof $scope.ldPref.det_region.end_y != 'number'
      )
        $scope.ldPref.det_region.end_y = 1;
      if ($scope.ldPref.det_region.end_y > 100)
        $scope.ldPref.det_region.end_y = 100;
      if ($scope.ldPref.det_region.start_x >= $scope.ldPref.det_region.end_x) {
        $scope.ldPref.det_region.start_x = $scope.ldPref.det_region.end_x - 1;
      }
      if ($scope.ldPref.det_region.start_y >= $scope.ldPref.det_region.end_y) {
        $scope.ldPref.det_region.start_y = $scope.ldPref.det_region.end_y - 1;
      }
    };
    
    $scope.sendData = function () {
      $scope.ldPref.cmd_type = 'set';
      if ($scope.ldPref.enabled) $scope.ldPref.enabled = 1;
      else $scope.ldPref.enabled = 0;
      $scope.cmd = '/cgi-bin/msg.cgi';
      $scope.ldPref.enabled = $scope.ldPref.enabled == 1;
      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: JSON.stringify($scope.ldPref, function (key, value) {
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
              $scope.msg_t.WriteFail[$scope.lang] + ',rval=' + resp.data.rval;
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
          "cmd_id": Cmd.AGTX_CMD_LD_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_LD_CONF');
          $scope.ldPref = resp.data;
          $scope.canApply = true;
          $scope.ldPref.enabled = $scope.ldPref.enabled == 1;
          $scope.Result = resp;
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
          console.log(resp);
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_LD_CONF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getData();
    window.langId = $scope.lang;
  },
]);
