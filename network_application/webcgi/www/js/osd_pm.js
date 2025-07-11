app.controller('HC_Ctrl', [
  '$scope',
  '$location',
  '$http',
  function ($scope, $location, $http) {
    $scope.lang = getLangFromUrlPath($location);
    $scope.LMenu = LMenu;
    $scope.MAX_BIND_PER_CHN = 4;
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
      OSD_Setting: ['OSD Setting', 'OSD設置', 'OSD设置'],
      Privacy_Mask: ['Privacy Mask', '隱私遮罩', '隐私遮罩'],
      Enable_Privacy_Mask: [
        'Enable Privacy Mask',
        '起用隱私遮罩',
        '起用隐私遮罩',
      ],
      Alpha: ['Transparency level', '透明度', '透明度'],
      ID: ['ID', '號碼', '号码'],
      BLACK: ['BLACK', '黑色', '黑色'],
      RED: ['RED', '紅色', '红色'],
      GREEN: ['GREEN', '綠色', '绿色'],
      BLUE: ['BLUE', '藍色', '蓝色'],
      YELLOW: ['YELLOW', '黃色', '黄色'],
      PURPLE: ['PURPLE', '紫色', '紫色'],
      ORANGE: ['ORANGE', '橘色', '橘色'],
      WHITE: ['WHITE', '白色', '白色'],
      Start: ['Start', '開始', '开始'],
      End: ['End', '結束', '结束'],
      Color: ['Color', '顏色', '颜色'],
      Thickness: ['Thickness', '框寬', '框宽'],
      Grid: ['Grid', '柵格', '栅格'],
      Mask: ['Mask', '遮擋', '遮挡'],
      Rectangle_Position: ['Rectangle Position', '框的位置', '框的位置'],
      Rectangle_Editor: ['Rectangle Editor', '編輯框', '编辑框'],
      Active_Mask: ['Active Mask', '激活遮擋', '激活遮挡'],
      Setup_Mask: ['Setup Mask', '使用遮擋', '使用遮挡'],
      Clear_Mask: ['Clear Mask', '清除遮擋', '清除遮挡'],
      Draw_Rectangle: ['Draw Region', '畫區域', '画区域'],
      Invalid_region_cnt: [
        'Enabled OSD region number in OSD setting & Privacy Mask is more than 4!!',
        '開啟的OSD區域數在OSD設置與隱私遮罩總和超過四個!!',
        '开启的OSD区域数在OSD设置与隐私遮罩总和超过四个!!',
      ],
    };
    $scope.osd_pmPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_OSD_PM_CONF,
      cmd_type: 'set',
      conf: [
        /*我是註解, 這是第1路video streaming*/
        {
          param: [
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
          ],
        },

        /*我是註解, 這是第2路video streaming*/
        {
          param: [
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
          ],
        },

        /*我是註解, 這是第3路video streaming, 但沒有設定 */
        {
          param: [
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
          ],
        },

        /*我是註解, 這是第4路video streaming, 但沒有設定 */
        {
          param: [
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
            {
              enabled: 0,
              alpha: 0,
              color: 0,
              start_x: 0,
              start_y: 0,
              end_x: 0,
              end_y: 0,
            },
          ],
        },
      ],
    };

    $scope.osdPref = {
      master_id: 1,
      cmd_id: Cmd.AGTX_CMD_OSD_CONF,
      cmd_type: 'set',
      strm: [
        /*我是註解, 這是第1路video streaming*/
        {
          region: [
            {
              enabled: 1,
              start_x: 96,
              start_y: 1,
              type: 'TEXT',
              type_spec: 'Augentix',
            },
            {
              enabled: 1,
              start_x: 89,
              start_y: 95,
              type: 'INFO',
              type_spec: 'YYYY-MM-DD HH:mm:ss',
            },
            {
              enabled: 0,
              start_x: 1,
              start_y: 1,
              type: 'IMAGE',
              type_spec: '/system/mpp/font/LOGO_Augentix_v2.imgayuv',
            },
            {
              enabled: 0,
              start_x: 0,
              start_y: 0,
              type: 'TEXT',
              type_spec: '',
            },
          ],
        },

        /*我是註解, 這是第2路video streaming*/
        {
          region: [
            {
              enabled: 1,
              start_x: 96,
              start_y: 1,
              type: 'TEXT',
              type_spec: 'Augentix',
            },
            {
              enabled: 1,
              start_x: 89,
              start_y: 95,
              type: 'INFO',
              type_spec: 'YYYY-MM-DD HH:mm:ss',
            },
            {
              enabled: 0,
              start_x: 1,
              start_y: 1,
              type: 'IMAGE',
              type_spec: '/system/mpp/font/LOGO_Augentix_v2.imgayuv',
            },
            {
              enabled: 0,
              start_x: 0,
              start_y: 0,
              type: 'TEXT',
              type_spec: '',
            },
          ],
        },

        /*我是註解, 這是第3路video streaming, 但沒有設定 */
        {
          region: [
            {
              enabled: 1,
              start_x: 96,
              start_y: 1,
              type: 'TEXT',
              type_spec: 'Augentix',
            },
            {
              enabled: 1,
              start_x: 89,
              start_y: 95,
              type: 'INFO',
              type_spec: 'YYYY-MM-DD hh:mm:ss',
            },
            {
              enabled: 0,
              start_x: 1,
              start_y: 1,
              type: 'IMAGE',
              type_spec: '/system/mpp/font/LOGO_Augentix_v2.imgayuv',
            },
            {
              enabled: 0,
              start_x: 0,
              start_y: 0,
              type: 'TEXT',
              type_spec: '',
            },
          ],
        },

        /*我是註解, 這是第4路video streaming, 但沒有設定 */
        {
          region: [
            {
              enabled: 1,
              start_x: 96,
              start_y: 1,
              type: 'TEXT',
              type_spec: 'Augentix',
            },
            {
              enabled: 1,
              start_x: 89,
              start_y: 95,
              type: 'INFO',
              type_spec: 'YYYY-MM-DD %H:%m:%s',
            },
            {
              enabled: 0,
              start_x: 1,
              start_y: 1,
              type: 'IMAGE',
              type_spec: '/system/mpp/font/LOGO_Augentix_v2.imgayuv',
            },
            {
              enabled: 0,
              start_x: 0,
              start_y: 0,
              type: 'TEXT',
              type_spec: '',
            },
          ],
        },
      ],
    };

    $scope.img = document.getElementById('u400');
    $scope.img.src = '../img/u400.png';
    $scope.preview = document.getElementById('preview');
    $scope.preview.width = 300;
    $scope.preview.height = 170;

    $scope.color_dict = {
      0: 'BLACK',
      1: 'RED',
      2: 'GREEN',
      3: 'BLUE',
      4: 'YELLOW',
      5: 'PURPLE',
      6: 'ORANGE',
      7: 'WHITE',
    };

    var canvas_preivew = 0;

    $scope.toggle_canvas = function () {
      if (canvas_preivew) {
        $('#preview_div').hide();
        $('#vlc_div').show();
        $('#preview_btn').text('Canvas Preview');
        canvas_preivew = 0;
      } else {
        $('#vlc_div').hide();
        $('#preview_div').show();
        $('#preview_btn').text('VLC Preview');
        canvas_preivew = 1;
      }
    };

    $scope.color_num = 8;

    $scope.osd_pmJs = {
      chn_nums: 1,
    };

    $scope.osd_pm_options = {
      stream_cnt: [1, 2, 3, 4],
      max_bind_list: [0, 1, 2, 3],
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
      /* 		var OSD1 = document.getElementById("playerPlugin");
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

    var regionNumb = 0;
    var x;
    var y;
    var width;
    var height;
    $scope.append_rectangle_marker = function () {
      x = parseFloat(rectangle_x.value);
      y = parseFloat(rectangle_y.value);
      width = parseFloat(rectangle_width.value);
      height = parseFloat(rectangle_height.value);
      if (isNaN(x) || isNaN(y) || isNaN(width) || isNaN(height)) return;
      console.log('modifying markers editor...');
      var markers = JSON.parse(markers_editor.value);
      $scope.do_if_array(markers, function () {
        rectangle_x.value = rectangle_y.value = rectangle_width.value = rectangle_height.value =
          '';
        markers.push({
          shape: 'rectangle',
          x: x,
          y: y,
          width: width,
          height: height,
          fill: drawing_color.value,
        });
        markers_editor.value = JSON.stringify(markers, null, 2);
        markers_editor.scrollIntoView(false);
      });
      console.log('In append function:' + markers);
      console.log('End modification.' + regionNumb);
    };

    $scope.append_zero_editor = function () {
      x = parseFloat(rectangle_x.value);
      y = parseFloat(rectangle_y.value);
      width = parseFloat(rectangle_width.value);
      height = parseFloat(rectangle_height.value);
      if (isNaN(x) || isNaN(y) || isNaN(width) || isNaN(height)) return;
      console.log('modifying markers editor...');

      var markers = JSON.parse(markers_editor.value);
      $scope.do_if_array(markers, function () {
        rectangle_x.value = rectangle_y.value = rectangle_width.value = rectangle_height.value =
          '';
        markers.push({
          shape: 'rectangle',
          x: 0,
          y: 0,
          width: 0,
          height: 0,
          fill: drawing_color.value,
        });
        markers_editor.value = JSON.stringify(markers, null, 2);
        markers_editor.scrollIntoView(false);
      });
      console.log('End modification.' + regionNumb);
    };

    /* 将html中已有的显示在视频插件上 */
    $scope.getdata_from_html = function () {
      var i = 0;
      var j = 0;
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          if ($scope.osd_pmPref.conf[i].param[j].enabled == 1) {
            var parsex = Math.round(
              ($scope.osd_pmPref.conf[i].param[j].start_x * 1080) / 100
            );
            var parsey = Math.round(
              ($scope.osd_pmPref.conf[i].param[j].start_y * 720) / 100
            );
            var parsewidth =
              Math.round(
                ($scope.osd_pmPref.conf[i].param[j].end_x * 1080) / 100
              ) - parsex;
            var parseheigt =
              Math.round(
                ($scope.osd_pmPref.conf[i].param[j].end_y * 720) / 100
              ) - parsey;
            var getx = parseFloat(parsex);
            var gety = parseFloat(parsey);
            var getwidth = parseFloat(parsewidth);
            var getheight = parseFloat(parseheigt);
            if (
              isNaN(getx) ||
              isNaN(gety) ||
              isNaN(getwidth) ||
              isNaN(getheight)
            )
              return;
            var markers = JSON.parse(markers_editor.value);
            do_if_array(markers, function () {
              rectangle_x.value = rectangle_y.value = rectangle_width.value = rectangle_height.value =
                '';
              markers.push({
                shape: 'rectangle',
                x: getx,
                y: gety,
                width: getwidth,
                height: getheight,
                fill: drawing_color.value,
              });
              markers_editor.value = JSON.stringify(markers, null, 2);
              markers_editor.scrollIntoView(false);
            });
            console.log(markers);
          }
        }
      }
    };

    $scope.setup_markers_from_html = function () {
      var OSD = document.getElementById('playerPlugin').content.OSD;
      try {
        var markers = JSON.parse(markers_editor.value);
        $scope.do_if_array(markers, function () {
          /* console.log(markers); */
          OSD.SetupMarkers(markers);
          $scope.update_active_markers();
          plugin.scrollIntoView();
        });
      } catch (e) {
        console.log('setup_markers_from_html error');
      }
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

    $scope.setup_markers_from_editor = function () {
      var OSD = document.getElementById('playerPlugin').content.OSD;
      try {
        var markers = JSON.parse(markers_editor.value);
        /* console.log(markers); */
        if (regionNumb >= 0 && regionNumb < 4) {
          $scope.osd_pmPref.conf[0].param[regionNumb].enabled = 1;
          $scope.osd_pmPref.conf[0].param[regionNumb].start_x = Math.round(
            (x / 1080) * 100
          );
          $scope.osd_pmPref.conf[0].param[regionNumb].start_y = Math.round(
            (y / 720) * 100
          );
          $scope.osd_pmPref.conf[0].param[regionNumb].end_x = Math.round(
            ((x + width) / 1080) * 100
          );
          $scope.osd_pmPref.conf[0].param[regionNumb].end_y = Math.round(
            ((y + height) / 720) * 100
          );
        }
        /* regionNumb++; */
        if (regionNumb++ > 4) regionNumb = 0;
        $scope.do_if_array(markers, function () {
          /* console.log(markers); */
          OSD.SetupMarkers(markers);
          $scope.update_active_markers();
          plugin.scrollIntoView();
        });
      } catch (e) {
        console.log('setup_markers_from_html error');
      }
    };

    $scope.setup_rectangle = function () {
      $scope.append_rectangle_marker();
      $scope.setup_markers_from_editor();
    };

    $scope.clear_Data = function () {
      var i = 0;
      var j = 0;
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          $scope.osd_pmPref.conf[i].param[j].enabled = 0;
          $scope.osd_pmPref.conf[i].param[j].alpha = 0;
          $scope.osd_pmPref.conf[i].param[j].start_x = 0;
          $scope.osd_pmPref.conf[i].param[j].start_y = 0;
          $scope.osd_pmPref.conf[i].param[j].end_x = 0;
          $scope.osd_pmPref.conf[i].param[j].end_y = 0;
          /* console.log("this is my test and i want to know the param[i]:" + $scope.osd_pmPref.conf[0].param[j]); */
        }
      }
    };

    $scope.clear_rectangle = function () {
      $scope.reset_markers_editor(false);
      $scope.setup_markers_from_editor();
      $scope.clear_Data();
    };


    $scope.getNumber = function (num) {
      var temp = [];
      for (var j = 0; j < num; j++) {
        temp.push(j);
      }
      return temp;
    };

    $scope.drawFill = function (ctx, x0, y0, x1, y1, cr, alp) {
      //var img = new Image();
      var w = $scope.preview.width;
      var h = $scope.preview.height;
      color = color_list[cr] + alp + ')';
      ctx.fillStyle = color;
      ctx.fillRect(
        (x0 / 100) * w,
        (y0 / 100) * h,
        (x1 / 100) * w,
        (y1 / 100) * h
      );
    };

    $scope.updateCanvas = function () {
      //var img = new Image();
      var ctx = $scope.preview.getContext('2d');
      //img.src = "../img/u400.png";
      ctx.drawImage($scope.img, 0, 0); //putImageData
      var i, j;
      for (i = 0; i < $scope.MAX_BIND_PER_CHN; i++) {
        if ($scope.osd_pmPref.conf[0].param[i].enabled) {
          var x0 = $scope.osd_pmPref.conf[0].param[i].start_x;
          var y0 = $scope.osd_pmPref.conf[0].param[i].start_y;
          var x1 = $scope.osd_pmPref.conf[0].param[i].end_x;
          var y1 = $scope.osd_pmPref.conf[0].param[i].end_y;
          var w = x1 - x0;
          var h = y1 - y0;
          var cr = $scope.osd_pmPref.conf[0].param[i].color;
          var alp = $scope.osd_pmPref.conf[0].param[i].alpha / 7;
          if (w < 0 || h < 0) continue;
          $scope.drawFill(ctx, x0, y0, w, h, cr, alp);
        }
      }
      //$scope.img.style.visibility = 'hidden';
      $scope.preview.style.width = '60%';
      $scope.img.style.width = '0%';
    };

    $scope.copyConf = function () {
      var i, j;
      var MAX_CHN_NUM = 4;
      for (i = 1; i < MAX_CHN_NUM; i++) {
        for (j = 0; j < $scope.MAX_BIND_PER_CHN; j++) {
          $scope.osd_pmPref.conf[i].param[j].enabled =
            $scope.osd_pmPref.conf[0].param[j].enabled;
          $scope.osd_pmPref.conf[i].param[j].start_x =
            $scope.osd_pmPref.conf[0].param[j].start_x;
          $scope.osd_pmPref.conf[i].param[j].start_y =
            $scope.osd_pmPref.conf[0].param[j].start_y;
          $scope.osd_pmPref.conf[i].param[j].end_x =
            $scope.osd_pmPref.conf[0].param[j].end_x;
          $scope.osd_pmPref.conf[i].param[j].end_y =
            $scope.osd_pmPref.conf[0].param[j].end_y;
          $scope.osd_pmPref.conf[i].param[j].color =
            $scope.osd_pmPref.conf[0].param[j].color;
          $scope.osd_pmPref.conf[i].param[j].alpha =
            $scope.osd_pmPref.conf[0].param[j].alpha;
        }
      }
    };

    $scope.update = function () {
      //$scope.updateCanvas();
      $scope.copyConf();
      $scope.isValidRegionCnt();
    };

    $scope.Result = 'result';

    $scope.canApply = true;

    $scope.isValidRegionCnt = function () {
      var i;
      var enabled_cnt = 0;

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
          "cmd_id": Cmd.AGTX_CMD_OSD_CONF,
          "cmd_type": "get",
        }),
      }).then(function (resp) {
        console.log('post getdata success: ' + 'AGTX_CMD_OSD_CONF');
        console.log(resp);

        $scope.osdPref = resp.data;
      });

      for (i = 0; i < $scope.osdPref.strm[0].region.length; i++) {
        if ($scope.osdPref.strm[0].region[i].enabled) {
          enabled_cnt++;
        }
      }

      for (i = 0; i < $scope.osd_pmPref.conf[0].param.length; i++) {
        if ($scope.osd_pmPref.conf[0].param[i].enabled) {
          enabled_cnt++;
        }
      }

      console.log('Total enabled count: ' + enabled_cnt);

      if (enabled_cnt <= 4) {
        $scope.canApply = true;
        $scope.cmdStatus = $scope.msg_t.noTxt[$scope.lang];
        console.log('Vaild enable number');
      } else {
        $scope.canApply = false;
        $scope.cmdStatus = $scope.msg_t.Invalid_region_cnt[$scope.lang];
        console.log('Invaild enable number');
      }
    };


    $scope.sendData = function () {
      /* $scope.updateCanvas(); */
      $scope.osd_pmPref.cmd_type = 'set';
      var i, j;
      for (i = 0; i < $scope.osd_pmPref.conf.length; i++) {
        for (j = 0; j < $scope.osd_pmPref.conf[i].param.length; j++) {
          if ($scope.osd_pmPref.conf[i].param[j].enabled)
            $scope.osd_pmPref.conf[i].param[j].enabled = 1;
          else $scope.osd_pmPref.conf[i].param[j].enabled = 0;
        }
      }

      $scope.cmd = '/cgi-bin/msg.cgi';
      var postData = JSON.stringify($scope.osd_pmPref);
      for (i = 0; i < $scope.osd_pmPref.conf.length; i++) {
        for (j = 0; j < $scope.osd_pmPref.conf[i].param.length; j++) {
          $scope.osd_pmPref.conf[i].param[j].enabled =
            $scope.osd_pmPref.conf[i].param[j].enabled == 1;
        }
      }

      console.log('enter submitForm');
      $http({
        method: 'post',
        url: $scope.cmd,
        headers: {
          'Content-Type': 'application/json',
        },
        data: postData,
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
      // Load OSD at first to aviod wrong loop in inValidCnt()
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
          "cmd_id": Cmd.AGTX_CMD_OSD_CONF,
          "cmd_type": "get",
        }),
      }).then(function (resp) {
        console.log('post getdata success: ' + 'AGTX_CMD_OSD_CONF');
        console.log(resp);

        $scope.osdPref = resp.data;
      });
      //
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
          "cmd_id": Cmd.AGTX_CMD_OSD_PM_CONF,
          "cmd_type": "get",
        }),
      }).then(
        function (resp) {
          console.log('post getdata success: ' + 'AGTX_CMD_OSD_PM_CONF');
          console.log(resp);
          $scope.osd_pmPref = resp.data;
          $scope.canApply = true;


          /* Get enabled channels and convert value to bool from integer */
          var i, j, chn_enabled;
          for (i = 0; i < $scope.osd_pmPref.conf.length; i++) {
            for (j = 0; j < $scope.osd_pmPref.conf[i].param.length; j++) {
              $scope.osd_pmPref.conf[i].param[j].enabled =
                $scope.osd_pmPref.conf[i].param[j].enabled == 1;
              /* console.log($scope.osd_pmPref.conf[i].param[j].enabled); */
              if ($scope.osd_pmPref.conf[i].param[j].enabled == true) {
                var parsex = Math.round(
                  ($scope.osd_pmPref.conf[i].param[j].start_x * 1080) / 100
                );
                var parsey = Math.round(
                  ($scope.osd_pmPref.conf[i].param[j].start_y * 720) / 100
                );
                var parsewidth =
                  Math.round(
                    ($scope.osd_pmPref.conf[i].param[j].end_x * 1080) / 100
                  ) - parsex;
                var parseheigt =
                  Math.round(
                    ($scope.osd_pmPref.conf[i].param[j].end_y * 720) / 100
                  ) - parsey;
                var getx = parseFloat(parsex);
                var gety = parseFloat(parsey);
                var getwidth = parseFloat(parsewidth);
                var getheight = parseFloat(parseheigt);
                if (
                  isNaN(getx) ||
                  isNaN(gety) ||
                  isNaN(getwidth) ||
                  isNaN(getheight)
                )
                  return;
              }

              if (
                $scope.osd_pmPref.conf[i].param[j].start_x != 0 ||
                $scope.osd_pmPref.conf[i].param[j].start_y != 0 ||
                $scope.osd_pmPref.conf[i].param[j].end_x != 0 ||
                $scope.osd_pmPref.conf[i].param[j].end_y != 0
              )
                regionNumb++;
            }
          }
          $scope.Result = resp;
          /* $scope.updateCanvas(); */
          if (hide_msg != 1) {
            $scope.cmdStatus = $scope.msg_t.ReadOK[$scope.lang];
          } else {
            $scope.cmdStatus = '';
          }
        },
        function (resp) {
          console.log('post error: ' + 'AGTX_CMD_OSD_PM_CONF');
          console.log(resp);
          $scope.canApply = true;
          $scope.cmdStatus = $scope.msg_t.ReadFail[$scope.lang];
          //$scope.Result = "Post Error";
        }
      );
    };
    $scope.getData(0);
    //$scope.updateCanvas();
    window.langId = $scope.lang;
  },
]);
