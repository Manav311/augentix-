/* message */
var msg = {
  /* common iva */
  Tamper_Detection: ['Tamper Detection', '入侵檢測', '入侵检测'],
  Motion_Detection: ['Motion Detection', '移動檢測', '移动检测'],
  Automatic_ROI: ['Automatic ROI', '自動目標檢測', '自动目标检测'],
  LightOnOff_Detection: ['LightOnOff Detection', '開關燈檢測', '开关灯检测'],
  Object_Detection: ['Object Detection', '物體檢測', '物体检测'],
  Pedestrian_Detection: ['Pedestrian Detection', '行人檢測', '行人检测'],
  Regional_Motion_Sensor: [
    'Regional Motion Sensor',
    '區域運動傳感器',
    '区域运动传感器',
  ],
  Electric_Fence: ['Electric Fence', '電子圍離', '电子围篱'],
  Pet_Feeding_Monitor: ['Pet Feeding Monitor', '竉物飲食監控', '宠物饮食监控'],
  Pan_Tilt_Zoom: ['Pan Tilt Zoom', '數碼平移/倾斜/變焦', '数码平移/倾斜/变焦'],
  Baby_Monitor: ['Baby Monitor', '嬰兒安全監控', '婴儿安全监控'],
  Edge_AI_Framework:['Edge AI Framework', '邊緣人工智慧應用', '边缘人工智慧应用'],
  Human_Detection: ['Human Detection', '人形檢測', '人形检测'],
  Face_Detection: ['Face Detection', '人臉檢測', '人臉检测'],
  Face_Recognition: ['Face Recognition', '臉部辨識', '脸部辨识',],
  Loud_Sound_Detection: [
    'Loud Sound Detection',
    '突發噪音檢測',
    '突发噪音检测',
  ],
  Video_Channel: ['Video Channel', '視頻通道', '视频通道'],
  Video_Layout: ['Video Layout', '視頻布局', '视频布局'],
  AudioSetting: ['Audio Setting', '音頻設置', '音频设置'],
  Fall_Detection: ['Fall Detection', '跌倒檢測', '跌倒检测'],
  Door_Keeper: ['Door Keeper', '門鈴監控', '门铃监控'],
  Two_Axis_EIS: ['2-Axis EIS', '2軸電子防手震', '2轴电子减震'],
};
/* iva menu */
var IvaMenu = [
  { txt: msg.Tamper_Detection, href: 'iva_td.html', page_item: 'page-item' },
  { txt: msg.Motion_Detection, href: 'iva_md.html', page_item: 'page-item' },
  { txt: msg.Object_Detection, href: 'iva_od.html', page_item: 'page-item' },
  {
    txt: msg.Pedestrian_Detection,
    href: 'iva_pd.html',
    page_item: 'page-item',
  },
  { txt: msg.Automatic_ROI, href: 'iva_aroi.html', page_item: 'page-item' },
  {
    txt: msg.LightOnOff_Detection,
    href: 'iva_ld.html',
    page_item: 'page-item',
  },
  {
    txt: msg.Regional_Motion_Sensor,
    href: 'iva_rms.html',
    page_item: 'page-item',
  },
  { txt: msg.Electric_Fence, href: 'iva_ef.html', page_item: 'page-item' },
  {
    txt: msg.Pet_Feeding_Monitor,
    href: 'iva_pfm.html',
    page_item: 'page-item',
  },
  { txt: msg.Baby_Monitor, href: 'iva_bm.html', page_item: 'page-item' },
  { txt: msg.Fall_Detection, href: 'iva_fld.html', page_item: 'page-item' },
  { txt: msg.Door_Keeper, href: 'iva_dk.html', dropdown: 'page-item' },
];

/* iva drop down menu */
var IvaDropDown = [
  { txt: msg.Tamper_Detection, href: 'iva_td.html', dropdown: 'dropdown-item' },
  { txt: msg.Motion_Detection, href: 'iva_md.html', dropdown: 'dropdown-item' },
  { txt: msg.Object_Detection, href: 'iva_od.html', dropdown: 'dropdown-item' },
  /*{
    txt: msg.Pedestrian_Detection,
    href: 'iva_pd.html',
    dropdown: 'dropdown-item',
  },*/
  { txt: msg.Automatic_ROI, href: 'iva_aroi.html', dropdown: 'dropdown-item' },
  /*{
    txt: msg.LightOnOff_Detection,
    href: 'iva_ld.html',
    dropdown: 'dropdown-item',
  },
  {
    txt: msg.Regional_Motion_Sensor,
    href: 'iva_rms.html',
    dropdown: 'dropdown-item',
  },*/
  { txt: msg.Electric_Fence, href: 'iva_ef.html', dropdown: 'dropdown-item' },
  {
    txt: msg.Pet_Feeding_Monitor,
    href: 'iva_pfm.html',
    dropdown: 'dropdown-item',
  },
  { txt: msg.Baby_Monitor, href: 'iva_bm.html', dropdown: 'dropdown-item' },
  { txt: msg.Fall_Detection, href: 'iva_fld.html', dropdown: 'dropdown-item' },
  { txt: msg.Door_Keeper, href: 'iva_dk.html', dropdown: 'dropdown-item' },
  { txt: msg.Face_Detection, href: 'iva_fd.html', dropdown: 'dropdown-item' },
  { txt: msg.Human_Detection, href: 'iva_hd.html', dropdown: 'dropdown-item' },
  { txt: msg.Face_Recognition, href: 'iva_fr.html', dropdown: 'dropdown-item' },
];

/* Video_menu */
var VideoMenu = [
  { txt: msg.Video_Channel, href: 'video.html', page_item: 'page-item' },
  //{'txt':msg.Video_Layout,'href':'video_layout.html','page_item':'page-item'},
  { txt: msg.Pan_Tilt_Zoom, href: 'ptz.html', page_item: 'page-item' },
  { txt: msg.Two_Axis_EIS, href: 'eis.html', page_item: 'page-item' },
];

/* Audio_menu */
var AudioMenu = [
  //	{'txt':msg.AudioSetting,'href':'audio.html','page_item':'page-item'},
  {
    txt: msg.Loud_Sound_Detection,
    href: 'iaa_lsd.html',
    page_item: 'page-item',
  },
];

/* common Menu */
var LMenu = [
  {
    txt: ['Home', '首頁', '首页'],
    href: '../index.html',
    hide: false,
  },
  {
    txt: ['General', '常規設置', '常规设置'],
    href: 'dateTime.html',
    hide: false,
  },
  {
    txt: ['Preview', '預覽', '预览'],
    href: 'preview.html',
    hide: false,
  },
  {
    txt: ['Network Setting', '網絡設置', '网络设置'],
    href: 'networkInterface.html',
    hide: false,
  },
  {
    txt: ['Video Channel Control', '視頻通道控制', '视频通道控制'],
    href: 'video.html',
    hide: false,
  },
  {
    txt: ['Camera Setting', '攝像頭設置', '摄像头设置'],
    href: 'imagePreference.html',
    hide: false,
  },
  { txt: ['OSD Setting', 'OSD設置', 'OSD设置'], href: 'osd.html', hide: false },
  {
    txt: ['IVA Setting', 'IVA設置', 'IVA设置'],
    href: 'iva_td.html',
    hide: false,
  },
  {
    txt: ['IAA Setting', 'IAA設置', 'IAA设置'],
    href: 'iaa_lsd.html',
    hide: false,
  },
  { txt: ['Event', '事件設置', '事件设置'], href: 'event.html', hide: true },
  {
    txt: ['Relay Server Settings', '中繼服務器設置', '中继服务器设置'],
    href: 'relay_server_settings.html',
    hide: false,
  },
  {
    txt: ['System Information', '系統信息', '系统信息'],
    href: 'system.html',
    hide: false,
  },
  {
    txt: ['Password', '密碼設置', '密码设置'],
    href: 'passwd.html',
    hide: false,
  },
  {
    txt: ['Logout', '登出', '注销'],
    href: 'logout.html',
    hide: false,
  },
];
