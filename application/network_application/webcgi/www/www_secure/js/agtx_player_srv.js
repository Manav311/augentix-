window.augentixPlayerService = {
  createPlayer: function(config) {
    const defaultConfig = {
      protocolType: 'http-flv',
      isLive: true,
      enableWorker: true,
      enableStashBuffer: false,
      autoCleanupSourceBuffer: true,
      autoCleanupMaxBackwardDuration: 60,
    };

    const newConfig = config
      ? { ...defaultConfig, ...config }
      : { ...defaultConfig };

    const mediaType = 'flv';
    const player = flvjs.createPlayer({
      type: mediaType,
      isLive: newConfig.isLive,
      url: newConfig.url,
      enableWorker: newConfig.enableWorker,
      enableStashBuffer: newConfig.enableStashBuffer,
      stashInitialSize: newConfig.stashInitialSize,
      autoCleanupSourceBuffer: newConfig.autoCleanupSourceBuffer,
      autoCleanupMaxBackwardDuration: newConfig.autoCleanupMaxBackwardDuration,
      headers: {
        'Authorization': 'Basic ' + btoa(encodeURIComponent("username:password"))
      },
    });
    return player;
  },

  play: function(player) {
    player.load();
    return player.play();
  },

  stop: function(player) {
    player.pause();
    player.unload();
  },

  mute: function(player) {
    player.muted = true;
    return player.muted;
  },

  unmute: function(player) {
    player.muted = false;
    return player.muted;
  },

  l10n: function(config = {}) {
    const self = this;
    const l10nTable = self._l10nTable;

    const key = config.key;
    const entry = l10nTable[key];
    const langId = config.langId ?? 0;

    if (!key) {
      return '';
    }

    return entry
      ? entry[langId]
      : key;
  },

  _l10nTable: {
    Preview: ['Preview', '預覽', '预览'],
    Play: ['Play', '播放', '播放'],
    Stop: ['Stop', '停止', '停止'],
    Mute: ['Mute', '靜音', '静音'],
    Unmute: ['Unmute', '取消靜音', '取消静音'],
    RatioNormal: ['4:3', '4:3', '4:3'],
    RatioWide: ['16:9', '16:9', '16:9'],
    RatioOriginal: ['1x', '1x', '1x'],
    RatioAuto: ['Auto', '自動', '自动'],
    MainStream: ['Main Stream', '主串流', '主码流'],
    SubStream: ['Sub Stream', '子串流', '子码流'],
    CaptureImage: ['Capture', '截圖', '截图'],
    Record: ['Record', '錄影', '录制'],
    Download: ['Download', '下載', '下载'],
    ConnectRelayServerFailTitle: ['Cannot Detect Relay Server!', '無法偵測到中繼服務器！', '无法侦测到中继服务器！'],
    ConnectRelayServerFailContent: [
      /*html*/`Please go to <a href="./relay_server_settings.html" target="_self">relay server settings page</a> to check status.`,
      /*html*/`請至 <a href="./relay_server_settings.html" target="_self">中繼服務器設置</a> 確認狀態。`,
      /*html*/`请至 <a href="./relay_server_settings.html" target="_self">中继服务器设置</a> 确认状态。`,
    ],
  },
};
