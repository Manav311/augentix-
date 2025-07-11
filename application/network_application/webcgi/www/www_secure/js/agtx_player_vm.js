const CHN_MAIN = 0;
const CHN_SUB = 1;
const ENC_H264 = 0;
const ENC_H265 = 1;
const MODE_NORMAL = 0;
const MODE_FR = 1; // face recognition

window.augentixPlayerViewModel = {
  utils: null,
  core: null,
  snapshot: null,
  recording: null,
};

window.augentixPlayerViewModel.utils = {
  removeEventListeners: function(oldDOM) {
    const newDOM = oldDOM.cloneNode(true);
    oldDOM.parentNode.replaceChild(newDOM, oldDOM);
  },
  getVideoEncode: async function() {
    console.log("get video encode");
    const videoEncode = await fetch('/cgi-bin/msg.cgi', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        "master_id": 1,
        "cmd_id": Cmd.AGTX_CMD_VIDEO_STRM_CONF,
        "cmd_type": "get",
      }),
    })
      .then(resp => resp.json())
      .then(data => data.video_strm_list.map(x => x.venc_type));
    console.log(videoEncode);
    return videoEncode;
  }
};

window.augentixPlayerViewModel.core = {
  _config: {
    rootId: 'agtx-player-preview',
    langId: 0,
    url: '',
    mainUrl: '',
    subUrl: '',
    mainRelayUrl: '',
    subRelayUrl: '',
    channel: CHN_SUB, // #26044 set sub stream as default
    clientWidth: 640,
    clientHeight: 480,
    aspectRatio: 'auto',
    videoDOM: null,
    player: null,
    mode: MODE_NORMAL
  },
  _websocket: null,

  getRootId: function() {
    return this._config.rootId;
  },

  getLangId: function() {
    return this._config.langId;
  },

  setLangId: function(value) {
    this._config.langId = value;
  },

  getUrl: function() {
    return this._config.url;
  },

  setUrl: function(value) {
    this._config.url = value;
  },

  getMainUrl: function() {
    return this._config.mainUrl;
  },

  setMainUrl: function(value) {
    this._config.mainUrl = value;
  },

  getSubUrl: function() {
    return this._config.subUrl;
  },

  setSubUrl: function(value) {
    this._config.subUrl = value;
  },

  getMainRelayUrl: function() {
    return this._config.mainRelayUrl;
  },

  setMainRelayUrl: function(value) {
    this._config.mainRelayUrl = value;
  },

  getSubRelayUrl: function() {
    return this._config.subRelayUrl;
  },

  setSubRelayUrl: function(value) {
    this._config.subRelayUrl = value;
  },

  getChannel: function() {
    return this._config.channel
  },

  setChannel: function(value) {
    this._config.channel = value;
  },

  getClientWidth: function() {
    return this._config.clientWidth;
  },

  getClientHeight: function() {
    return this._config.clientHeight;
  },

  getAspectRatio: function() {
    return this._config.aspectRatio;
  },

  setAspectRatio: function(value) {
    this._config.aspectRatio = value;
  },

  getVideoDOM: function() {
    return this._config.videoDOM;
  },

  setVideoDOM: function(value) {
    this._config.videoDOM = value;
  },

  getPlayer: function() {
    return this._config.player;
  },

  setPlayer: function(value) {
    this._config.player = value;
    return this._config.player;
  },

  getWebsocket: function() {
    return this._config.websocket;
  },

  setWebsocket: function(value) {
    this._config.websocket = value;
    return this._config.websocket;
  },

  getMode: function() {
    return this._config.mode;
  },

  setMode: function(value) {
    this._config.mode = value;
  },


  init: async function(config = {}) {
    const { getVideoEncode } = window.augentixPlayerViewModel.utils;
    const self = this;

    const setUrl = self.setUrl.bind(self);
    const setChannel = self.setChannel.bind(self);
    const setPlayer = self.setPlayer.bind(self);
    const render = self.render.bind(self);
    const createRelayServer = self.createRelayServer.bind(self);
    const connectRelayServer = self.connectRelayServer.bind(self);
    const register = self.register.bind(self);
    const togglePlay = self.togglePlay.bind(self);

    const videoEncode = await getVideoEncode();
    const newConfig = {...self._config, ...config};
    self._config = newConfig;
    const mode = self.getMode();
    if (mode == MODE_FR) {
      setChannel(CHN_MAIN); // #37485 set main stream as default for face capture
    }
    const channel = self.getChannel();

    let url = self.getUrl();
    if (!url) {
      if ((channel == CHN_SUB)) {
        url = (videoEncode[CHN_SUB] == ENC_H265) ? self.getSubRelayUrl() : self.getSubUrl();
      } else {
        url = (videoEncode[CHN_MAIN] == ENC_H265) ? self.getMainRelayUrl() : self.getMainUrl();
      }
      setUrl(url);
    }

    const player = window.augentixPlayerService.createPlayer({ url });
    setPlayer(player);

    createRelayServer();
    render();
    register();

    if (videoEncode[channel] == ENC_H265) {
      connectRelayServer();
    }

    togglePlay({ player });
  },

  render: function() {
    const self = this;

    const clientWidth = self.getClientWidth();
    const clientHeight = self.getClientHeight();

    const l10n = self.l10n.bind(self);

    const rootId = self.getRootId();
    document.querySelector(`#${rootId}`).innerHTML = /*html*/`
    <div id="agtx-player-ws-alert"></div>
    <div class="container-live-view">
      <div class="controls flex flex-between">
        <div>
          <button name="toggle-play">${l10n('Play')}</button>
          <button name="toggle-muted">${l10n('Unmute')}</button>
          <button name="stream-main">${l10n('MainStream')}</button>
          <button name="stream-sub">${l10n('SubStream')}</button>
        </div>
        <div>
          <button name="ratio-4-3">${l10n('RatioNormal')}</button>
          <button name="ratio-16-9">${l10n('RatioWide')}</button>
          <button name="ratio-none">${l10n('RatioOriginal')}</button>
          <button name="ratio-auto">${l10n('RatioAuto')}</button>
        </div>
      </div>
      <div class="container-video flex flex-center bg-black" style="position: relative;">
        <video muted autoplay style="position: relative;"></video>
        <canvas id="ROIOverlay" width=${clientWidth} height=${clientHeight} style="position: absolute;"></canvas>
        <canvas hidden id="capturedFace" width=${clientWidth} * 0.25 height=${clientHeight} * 4/9 style="position: absolute;"></canvas>
      </div>
      <div class="flex flex-between">
        <div id="agtx-player-snapshot"></div>
        <div id="agtx-voice-call"></div>
      </div>
    </div>
    `;
  },

  createRelayServer: function() {
    const self = this;
    const setWebsocket = self.setWebsocket.bind(self);
    const showRelayServerLostConnectionAlert = self.showRelayServerLostConnectionAlert.bind(self);
    const dismissRelayServerLostConnectionAlert = self.dismissRelayServerLostConnectionAlert.bind(self);

    const websocket = new signalR.HubConnectionBuilder()
      .withUrl(`${window.relayAPIOrigin}/video-hub`)
      .withAutomaticReconnect()
      .configureLogging(signalR.LogLevel.Information)
      .build();
    websocket.onreconnecting(err => {
      showRelayServerLostConnectionAlert();
      console.error(err.toString());
    });
    websocket.onreconnected(_id => {
      dismissRelayServerLostConnectionAlert();
      console.log("Websocket reconnected");
    });
    websocket.on("RegisterResult", (res) => {
      console.log(`Resister result: ${res}`);
    });
    websocket.on("RemoteHostChanged", (res) => {
      console.log(`Relay source changed. New source: ${res}`);
    });
    setWebsocket(websocket);
  },

  connectRelayServer: async function() {
    const self = this;
    const showRelayServerLostConnectionAlert = self.showRelayServerLostConnectionAlert.bind(self);
    const dismissRelayServerLostConnectionAlert = self.dismissRelayServerLostConnectionAlert.bind(self);

    const websocket = self.getWebsocket();
    if (websocket.state == signalR.HubConnectionState.Connected
      || websocket.state == signalR.HubConnectionState.Connecting
      || websocket.state == signalR.HubConnectionState.Reconnecting) {
      return;
    }
    try {
      await websocket.start();
      await websocket.invoke("Register", { "remoteHost": location.hostname, "lang": this.getLangId() });
      dismissRelayServerLostConnectionAlert();
      console.log("WebSocket connected.");
    } catch (err) {
      showRelayServerLostConnectionAlert();
      console.error(err.toString());
    }
  },

  disconnectRelayServer: async function() {
    const self = this;

    const websocket = self.getWebsocket();
    if (websocket.state == signalR.HubConnectionState.Disconnected || websocket.state == signalR.HubConnectionState.Disconnecting) {
      return;
    }
    await websocket.stop();
    console.log("WebSocket disconnected.");
  },

  showRelayServerLostConnectionAlert: function() {
    const self = this;
    const l10n = self.l10n.bind(self);
    document.querySelector('#agtx-player-ws-alert').innerHTML = /*html*/`
    <div class="alert alert-warning alert-dismissible fade show" role="alert">
      <strong>${l10n('ConnectRelayServerFailTitle')}</strong>
      ${l10n('ConnectRelayServerFailContent')}
      <button type="button" class="close" data-dismiss="alert" aria-label="Close">
        <span aria-hidden="true">&times;</span>
      </button>
    </div>`;
  },

  dismissRelayServerLostConnectionAlert: function() {
    document.querySelector('#agtx-player-ws-alert').innerHTML = '';
  },

  register: function() {
    const { getVideoEncode } = window.augentixPlayerViewModel.utils;
    const self = this;

    const setVideoDOM = self.setVideoDOM.bind(self);
    const togglePlay = self.togglePlay.bind(self);
    const toggleMuted = self.toggleMuted.bind(self);
    const updateUrl = self.updateUrl.bind(self);
    const updateAspectRatio = self.updateAspectRatio.bind(self);

    const clientWidth = self.getClientWidth();
    const clientHeight = self.getClientHeight();
    const rootId = self.getRootId();
    const player = self.getPlayer();

    setVideoDOM(document.querySelector(`#${rootId} .container-live-view .container-video video`));

    const togglePlayDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="toggle-play"]`);
    togglePlayDOM.addEventListener('click', async function() {
      togglePlay({ player });
    });

    const toggleMutedDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="toggle-muted"]`);
    toggleMutedDOM.addEventListener('click', function() {
      toggleMuted({ player });
    });

    const streamMainDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="stream-main"]`);
    streamMainDOM.addEventListener('click', async function() {
      const setChannel = self.setChannel.bind(self);
      const videoEncode = await getVideoEncode();
      const url = (videoEncode[CHN_MAIN] == ENC_H265) ? self.getMainRelayUrl() : self.getMainUrl();
      setChannel(CHN_MAIN);
      updateUrl({ url });
      if (videoEncode[CHN_MAIN] == ENC_H265) {
        self.connectRelayServer();
      } else {
        self.disconnectRelayServer();
      }
    });

    const streamSubDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="stream-sub"]`);
    streamSubDOM.addEventListener('click', async function() {
      const setChannel = self.setChannel.bind(self);
      const videoEncode = await getVideoEncode();
      const url = (videoEncode[CHN_SUB] == ENC_H265) ? self.getSubRelayUrl() : self.getSubUrl();
      setChannel(CHN_SUB);
      updateUrl({ url });
      if (videoEncode[CHN_SUB] == ENC_H265) {
        self.connectRelayServer();
      } else {
        self.disconnectRelayServer();
      }
    });

    const ratio4by3DOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="ratio-4-3"]`);
    ratio4by3DOM.addEventListener('click', function() {
      updateAspectRatio({ aspectRatio: 'normal' });
    });

    const ratio16by9DOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="ratio-16-9"]`);
    ratio16by9DOM.addEventListener('click', function() {
      updateAspectRatio({ aspectRatio: 'wide' });
    });

    const ratioNoneDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="ratio-none"]`);
    ratioNoneDOM.addEventListener('click', function() {
      updateAspectRatio({ aspectRatio: 'none' });
    });

    const ratioAutoDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="ratio-auto"]`);
    ratioAutoDOM.addEventListener('click', function() {
      updateAspectRatio({ aspectRatio: 'auto' });
    });

    const liveViewContainerDOM = document.querySelector(`#${rootId} .container-live-view`);
    liveViewContainerDOM.style.width = `${clientWidth}px`;

    const videoContainerDOM = document.querySelector(`#${rootId} .container-live-view .container-video`);
    videoContainerDOM.style.height = `${clientHeight}px`;

    const aspectRatio = self.getAspectRatio();
    updateAspectRatio({ aspectRatio });
  },

  unregister: function() {
    const { removeEventListeners } = window.augentixPlayerViewModel.utils;

    const self = this;

    const stop = self.stop.bind(self);
    const setUrl = self.setUrl.bind(self);

    const rootId = self.getRootId();
    const player = self.getPlayer();

    const toggleMutedDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="toggle-muted"]`);
    removeEventListeners(toggleMutedDOM);

    const togglePlayDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="toggle-play"]`);
    removeEventListeners(togglePlayDOM);

    const streamMainDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="stream-main"]`);
    removeEventListeners(streamMainDOM);

    const streamSubDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="stream-sub"]`);
    removeEventListeners(streamSubDOM);

    const ratio4by3DOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="ratio-4-3"]`);
    removeEventListeners(ratio4by3DOM);

    const ratio16by9DOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="ratio-16-9"]`);
    removeEventListeners(ratio16by9DOM);

    const ratioNoneDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="ratio-none"]`);
    removeEventListeners(ratioNoneDOM);

    const ratioAutoDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="ratio-auto"]`);
    removeEventListeners(ratioAutoDOM);


    if (self.isPlaying()) {
      stop({ player });
    }
    setUrl('');
    player.destroy();
  },

  togglePlay: async function({ player }) {
    if (!player) {
      return;
    }

    const { getVideoEncode } = window.augentixPlayerViewModel.utils;
    const self = this;

    const play = self.play.bind(self);
    const stop = self.stop.bind(self);
    const l10n = self.l10n.bind(self);
    const updateUrl = self.updateUrl.bind(self);

    const rootId = self.getRootId();
    const togglePlayDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="toggle-play"]`);
    if (self.isPlaying()) {
      stop({ player });
      self.disconnectRelayServer();
      togglePlayDOM.innerText = l10n('Play');
    } else {
      const videoEncode = await getVideoEncode();
      const channel = self.getChannel();
      if (videoEncode[channel] == ENC_H265)
      {
        self.connectRelayServer();
      }
      const currentUrl = self.getUrl();
      const url = (videoEncode[channel] == ENC_H265)
        ? ((channel == CHN_MAIN) ? self.getMainRelayUrl() : self.getSubRelayUrl())
        : ((channel == CHN_MAIN) ? self.getMainUrl() : self.getSubUrl());
      if (currentUrl == url)
      {
        play({ player });
      } else {
        updateUrl({ url });
      }
      togglePlayDOM.innerText = l10n('Stop');
    }
  },

  play: function({ player }) {
    if (!player) {
      return;
    }

    const self = this;

    const videoDOM = self.getVideoDOM();

    player.attachMediaElement(videoDOM);
    window.augentixPlayerService.play(player);
  },

  stop: function({ player }) {
    if (!player) {
      return;
    }

    window.augentixPlayerService.stop(player);
    player.detachMediaElement();
  },

  isPlaying: function() {
    const self = this;
    const videoDOM = self.getVideoDOM();
    return !(videoDOM.paused);
  },

  toggleMuted: function({ player }) {
    if (!player) {
      return;
    }

    const self = this;

    if (!self.isPlaying()) {
      return;
    }

    const l10n = self.l10n.bind(self);

    player.muted = !player.muted;

    const rootId = self.getRootId();
    const toggleMutedDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="toggle-muted"]`);
    if (player.muted) {
      toggleMutedDOM.innerText = l10n('Unmute');
    } else {
      toggleMutedDOM.innerText = l10n('Mute');
    }
  },

  mute: function({ buttonDOM, player }) {
    if (!player) {
      return;
    }

    const self = this;
    const l10n = self.l10n.bind(self);

    player.muted = true;
    buttonDOM.innerText = l10n('Unmute');
  },

  unmute: function({ buttonDOM, player }) {
    if (!player) {
      return;
    }

    const self = this;
    const l10n = self.l10n.bind(self);

    player.muted = false;
    buttonDOM.innerText = l10n('Mute');
  },

  updateUrl: function({ url }) {
    const self = this;

    const setUrl = self.setUrl.bind(self);
    const setPlayer = self.setPlayer.bind(self);
    const register = self.register.bind(self);
    const unregister = self.unregister.bind(self);
    const play = self.play.bind(self);

    const rootId = self.getRootId();

    unregister();
    setUrl(url);
    const player = window.augentixPlayerService.createPlayer({ url });
    setPlayer(player);
    register();
    const togglePlayDOM = document.querySelector(`#${rootId} .container-live-view .controls button[name="toggle-play"]`);
    play({ buttonDOM: togglePlayDOM, player });
  },

  updateAspectRatio: function({ aspectRatio }) {
    const self = this;

    const setAspectRatio = self.setAspectRatio.bind(self);

    const clientWidth = self.getClientWidth();
    const clientHeight = self.getClientHeight();
    const videoDOM = self.getVideoDOM();

    setAspectRatio(aspectRatio);
    switch (aspectRatio) {
      case 'normal':
        videoDOM.setAttribute('width', clientWidth);
        videoDOM.setAttribute('height', Math.round(clientWidth * 0.75));
        videoDOM.style.objectFit = 'fill';
        break;
      case 'wide':
        videoDOM.setAttribute('width', clientWidth);
        videoDOM.setAttribute('height', Math.round(clientWidth * 0.5625));
        videoDOM.style.objectFit = 'fill';
        break;
      case 'none':
        videoDOM.setAttribute('width', clientWidth);
        videoDOM.setAttribute('height', clientHeight);
        videoDOM.style.objectFit = 'none';
        break;
      case 'auto':
      default:
        videoDOM.setAttribute('width', clientWidth);
        videoDOM.setAttribute('height', clientHeight);
        videoDOM.style.objectFit = 'contain';
        break;
    }
  },

  drawImage: function({ canvasDOM }) {
    const self = this;

    const clientWidth = self.getClientWidth();
    const clientHeight = self.getClientHeight();
    const aspectRatio = self.getAspectRatio();
    const videoDOM = self.getVideoDOM();

    const videoWidth = videoDOM.videoWidth;
    const videoHeight = videoDOM.videoHeight;

    const ctx = canvasDOM.getContext('2d');
    ctx.fillStyle = 'black';
    ctx.fillRect(0, 0, clientWidth, clientHeight);

    let scale, drawWidth, drawHeight;
    switch (aspectRatio) {
      case 'normal':
        drawWidth = clientWidth;
        drawHeight = Math.round(drawWidth * 0.75);
        break;
      case 'wide':
        drawWidth = clientWidth;
        drawHeight = Math.round(drawWidth * 0.5625);
        break;
      case 'none':
        drawWidth = videoWidth;
        drawHeight = videoHeight;
        break;
      case 'auto':
      default:
        scale = clientWidth / videoWidth;
        drawWidth = clientWidth;
        drawHeight = Math.round(videoHeight * scale);
        if (drawHeight > clientHeight) {
            scale = clientHeight / videoHeight;
            drawWidth = Math.round(videoWidth * scale);
            drawHeight = clientHeight;
        }
        break;
    }

    const dx = Math.round((clientWidth - drawWidth) * 0.5);
    const dy = Math.round((clientHeight - drawHeight) * 0.5);
    ctx.drawImage(videoDOM, dx, dy, drawWidth, drawHeight);
  },

  drawCapture: function({ canvasDOM }) {
    const self = this;

    const clientWidth = self.getClientWidth();
    const clientHeight = self.getClientHeight();
    const aspectRatio = self.getAspectRatio();
    const videoDOM = self.getVideoDOM();

    const videoWidth = videoDOM.videoWidth;
    const videoHeight = videoDOM.videoHeight;

    const ctx = canvasDOM.getContext('2d');
    ctx.fillStyle = 'black';
    ctx.fillRect(0, 0, clientWidth, clientHeight);

    let scale, drawWidth, drawHeight;
    switch (aspectRatio) {
      case 'normal':
        drawWidth = clientWidth;
        drawHeight = Math.round(drawWidth * 0.75);
        break;
      case 'wide':
        drawWidth = clientWidth;
        drawHeight = Math.round(drawWidth * 0.5625);
        break;
      case 'none':
        drawWidth = videoWidth;
        drawHeight = videoHeight;
        break;
      case 'auto':
      default:
        scale = clientWidth / videoWidth;
        drawWidth = clientWidth;
        drawHeight = Math.round(videoHeight * scale);
        if (drawHeight > clientHeight) {
            scale = clientHeight / videoHeight;
            drawWidth = Math.round(videoWidth * scale);
            drawHeight = clientHeight;
        }
        break;
    }

    const dx = Math.round((clientWidth - drawWidth) * 0.5);
    const dy = Math.round((clientHeight - drawHeight) * 0.5);
    ctx.drawImage(videoDOM, dx, dy, drawWidth, drawHeight);
    const ROIWidth = Math.round(clientWidth * 0.25);
    const ROIHeight = Math.round(clientHeight * 4/9 );
    const ROIx = Math.round((clientWidth  - ROIWidth) * 0.5);
    const ROIy = Math.round((clientHeight - ROIHeight) * 0.5);

    var MAX_WIDTH = 320;
    var MAX_HEIGHT = 320;
    var width = ROIWidth;
    var height = ROIHeight;
    if (width > height) {
        if (width > MAX_WIDTH) {
            height = Math.round(height * MAX_WIDTH / width);
            width = MAX_WIDTH;
        }
    } else {
        if (height > MAX_HEIGHT) {
            width = Math.round(width * MAX_HEIGHT / height);
            height = MAX_HEIGHT;
        }
    }

    const c = document.querySelector("#capturedFace");
    c.width = width;
    c.height = height;
    const ctx2 = c.getContext('2d');
    ctx2.drawImage(canvasDOM, ROIx, ROIy, ROIWidth, ROIHeight, 0, 0, width, height);
  },

  l10n: function(key) {
    const langId = this.getLangId();
    return window.augentixPlayerService.l10n({ key, langId });
  },
};

window.augentixPlayerViewModel.snapshot = {
  _config : {
    rootId: 'agtx-player-snapshot',
    core: null,
    mode: 0,
  },

  getConfig: function() {
    return this._config;
  },

  setConfig: function(value) {
    this._config = value;
  },

  getMode: function() {
    return this._config.mode;
  },

  setMode: function(value) {
    this._config.mode = value;
  },

  getRootId: function() {
    return this._config.rootId;
  },

  getCore: function() {
    return this._config.core;
  },

  updateConfig: function(config = {}) {
    const self = this;

    const setConfig = self.setConfig.bind(self);

    const newConfig = {...self.getConfig(), ...config};
    setConfig(newConfig);
  },

  init: function(config = {}) {
    const self = this;

    const updateConfig = self.updateConfig.bind(self);
    const render = self.render.bind(self);
    const register = self.register.bind(self);
    const mode = self.getMode();

    updateConfig(config);
    const core = self.getCore();
    const clientWidth = core.getClientWidth();
    const clientHeight = core.getClientHeight();

    render();
    register({ clientWidth, clientHeight });
  },

  render: function() {
    const self = this;

    const core = self.getCore();
    const l10n = core.l10n.bind(core);

    const rootId = self.getRootId();
    const mode = self.getMode();
    document.querySelector(`#${rootId}`).innerHTML = /*html*/`
    <div class="container-snapshot">
      <div class="controls">
        <button name="capture" id="captureBtn">${l10n('CaptureImage')}</button>
        <button name="captureFace" id="captureFaceBtn" style="display: none">${l10n('CaptureImage')}</button>
      </div>
      <div class="container-image bg-black display-none"></div>
    </div>
    `;
    if (mode === MODE_FR) {
      const c = document.querySelector("#ROIOverlay");
      const ctx = c.getContext('2d');
      const clientWidth = core.getClientWidth();
      const clientHeight = core.getClientHeight();
      const ROIWidth = Math.round(clientWidth * 0.25);
      const ROIHeight = Math.round(clientHeight * 4/9);
      const ROIx = Math.round((clientWidth - ROIWidth) * 0.5);
      const ROIy = Math.round((clientHeight - ROIHeight) * 0.5);
      ctx.strokeStyle = "rgba(0,162,232,1)";
      ctx.lineWidth = 6;
      ctx.strokeRect(ROIx, ROIy, ROIWidth, ROIHeight, );
      function drawCircle(x, y, radiusX, radiusY, rotation) {
        ctx.strokeStyle = "rgba(0,162,232,0.25)";
        ctx.beginPath();
        ctx.ellipse(x, y, radiusX, radiusY, rotation, 0, 2 * Math.PI);
        ctx.stroke();
      }
      var deg = 90;
      var rad = deg * (Math.PI / 180.0);
      drawCircle(clientWidth * 0.5, clientHeight * 0.5, ROIWidth *0.4, ROIHeight * 0.3, rad);
      document.querySelector("#captureBtn").style = "display: none;";
    }
  },

  register: function({ clientWidth, clientHeight }) {
    const self = this;

    const capture = self.capture.bind(self);
    const captureFace = self.captureFace.bind(self);

    const rootId = self.getRootId();

    const containerDOM = document.querySelector(`#${rootId} .container-snapshot .container-image`);
    containerDOM.style.width = `${clientWidth}px`;
    containerDOM.style.height = `${clientHeight}px`;


    const captureDOM = document.querySelector(`#${rootId} .container-snapshot .controls button[name="capture"]`);
    captureDOM.addEventListener('click', function() {
      capture({ clientWidth, clientHeight, containerDOM });
    });

    const captureFaceDOM = document.querySelector(`#${rootId} .container-snapshot .controls button[name="captureFace"]`);
    captureFaceDOM.addEventListener('click', function() {
      window.capturedImage = captureFace({ clientWidth, clientHeight});
    });
  },

  capture: function({ clientWidth, clientHeight, containerDOM }) {
    const self = this;

    const download = self.download.bind(self);

    const core = self.getCore();
    const drawImage = core.drawImage.bind(core);

    const canvasDOM = document.createElement('canvas');
    canvasDOM.width = clientWidth;
    canvasDOM.height = clientHeight;

    drawImage({ canvasDOM });
    const imgDOM = document.createElement('img');
    imgDOM.src = canvasDOM.toDataURL('image/jpeg', 0.8);
    containerDOM.textContent = '';
    containerDOM.appendChild(imgDOM);

    const url = imgDOM.src;
    download({ url });
  },

  captureFace: function({ clientWidth, clientHeight}) {
    const self = this;

    const core = self.getCore();
    const drawCapture = core.drawCapture.bind(core);

    const canvasDOM = document.createElement('canvas');
    canvasDOM.width = clientWidth;
    canvasDOM.height = clientHeight;

    drawCapture({ canvasDOM });
    const c = document.querySelector("#capturedFace");
    const imgDOM = document.createElement('img');
    imgDOM.src = c.toDataURL('image/jpeg', 0.8);
    var capturefile = dataURItoBlob(imgDOM.src);
    console.log(capturefile);

    const newFile = new File( [capturefile], "captured.jpg", { type: 'text/plain' });
    return newFile;

    function dataURItoBlob(dataURI) {
      // convert base64/URLEncoded data component to raw binary data held in a string
      var byteString;
      if (dataURI.split(',')[0].indexOf('base64') >= 0)
          byteString = atob(dataURI.split(',')[1]);
      else
          byteString = unescape(dataURI.split(',')[1]);
      // separate out the mime component
      var mimeString = dataURI.split(',')[0].split(':')[1].split(';')[0];
      // write the bytes of the string to a typed array
      var ia = new Uint8Array(byteString.length);
      for (var i = 0; i < byteString.length; i++) {
          ia[i] = byteString.charCodeAt(i);
      }
      return new Blob([ia], {type:mimeString});
    }
  },

  download: function({ url }) {
    const createFileName = (date) => {
      const u = (date.getFullYear()).toString().padStart(4, '0');
      const v = (date.getMonth() + 1).toString().padStart(2, '0');
      const w = (date.getDate()).toString().padStart(2, '0');

      const x = (date.getHours()).toString().padStart(2, '0');
      const y = (date.getMinutes()).toString().padStart(2, '0');
      const z = (date.getSeconds()).toString().padStart(2, '0');

      return `snapshot_${u}${v}${w}_${x}${y}${z}.jpg`;
    };

    const downloadDOM = document.createElement('a');
    downloadDOM.target = '_blank';
    downloadDOM.href = url;
    downloadDOM.download = createFileName(new Date());
    downloadDOM.click();
  },
};

window.augentixPlayerViewModel.recording = {
  _config : {
    rootId: 'agtx-player-recording',
    dbName: 'AugentixPlayer',
    core: null,
    mediaRecorder: null,
    requestId: 0,
    recordedSize: 0,
    sizeLimit: 0,
    downloadUrl: '',
  },

  recordSize: {
    '256M': 268435456,
    '512M': 536870912,
    '1G': 1073741824,
  },

  getConfig: function() {
    return this._config;
  },

  setConfig: function(value) {
    this._config = value;
  },

  getRootId: function() {
    return this._config.rootId;
  },

  getDbName: function() {
    return this._config.dbName;
  },

  getCore: function() {
    return this._config.core;
  },

  getMediaRecorder: function() {
    return this._config.mediaRecorder;
  },

  setMediaRecorder: function(value) {
    this.revokeMediaRecorder();
    this._config.mediaRecorder = value;
  },

  revokeMediaRecorder: function() {
    if (this._config.mediaRecorder) {
      delete this._config.mediaRecorder;
    }
  },

  getRequestId: function() {
    return this._config.requestId;
  },

  setRequestId: function(value) {
    this._config.requestId = value;
  },

  getRecordedSize: function() {
    return this._config.recordedSize;
  },

  setRecordedSize: function(value) {
    this._config.recordedSize = value;
  },

  incrementRecordedSize: function(value) {
    this._config.recordedSize += value;
  },

  resetRecordedSize: function() {
    this._config.recordedSize = 0;
  },

  getSizeLimit: function() {
    return this._config.sizeLimit;
  },

  setSizeLimit: function(value) {
    this._config.sizeLimit = value;
  },

  getDownloadUrl: function() {
    return this._config.downloadUrl;
  },

  setDownloadUrl: function(value) {
    this._config.downloadUrl = value;
  },

  readRecordedChunks: async function(sizeLimit) {
    const self = this;

    const openDB = self.openDB.bind(self);

    let newSize = 0;
    const recordedChunks = [];
    const storeName = 'recordedChunks';
    const db = await openDB(storeName);
    const tx = db.transaction(storeName);
    for await (const cursor of tx.store) {
      const blob = cursor.value.data;
      const blobSize = blob.size;
      const blobType = blob.type;

      newSize += blob.size;
      const comp = newSize - sizeLimit;
      if (comp < 0) {
        recordedChunks.push(blob);
      } else if (comp === 0) {
        recordedChunks.push(blob);
        break;
      } else if (comp > 0) {
        const slicedSize = blobSize - comp;
        const newBlob = blob.slice(0, slicedSize, blobType);
        recordedChunks.push(newBlob);
        break;
      } else {
        throw new Error('Unknown error.');
      }
    }
    await tx.done;

    return recordedChunks;
  },

  writeRecordedChunk: async function(blob) {
    const self = this;

    const openDB = self.openDB.bind(self);
    const incrementRecordedSize = self.incrementRecordedSize.bind(self);

    const storeName = 'recordedChunks';
    const db = await openDB(storeName);
    await db.add(storeName, {
      data: blob,
    });
    incrementRecordedSize(blob.size);
  },

  resetRecordedChunks: async function() {
    const self = this;

    const clearStore = self.clearStore.bind(self);

    const storeName = 'recordedChunks';
    await clearStore(storeName);
  },

  clearStore: async function(storeName) {
    const self = this;

    const openDB = self.openDB.bind(self);
    const db = await openDB(storeName);
    await db.clear(storeName);
  },

  openDB: async function(storeName) {
    const self = this;

    const { openDB } = idb;
    const dbName = self.getDbName();
    const version = 1;
    return openDB(dbName, version, {
      upgrade(db) {
        db.createObjectStore(storeName, {
          keyPath: 'id',
          autoIncrement: true,
        });
      },
    });
  },

  createRecordedVideo: async function(sizeLimit) {
    const self = this;
    const recordedChunks = await self.readRecordedChunks(sizeLimit);
    const blob = new Blob(recordedChunks, { type: 'video/webm' });
    return blob;
  },

  updateConfig: function(config = {}) {
    const self = this;
    const setConfig = self.setConfig.bind(self);

    const newConfig = {...self.getConfig(), ...config};
    setConfig(newConfig);
  },

  init: async function(config = {}) {
    const self = this;

    const updateConfig = self.updateConfig.bind(self);
    const render = self.render.bind(self);
    const register = self.register.bind(self);

    updateConfig(config);
    const core = self.getCore();
    const clientWidth = core.getClientWidth();
    const clientHeight = core.getClientHeight();

    render();
    await register({ clientWidth, clientHeight });
  },

  render: function() {
    const self = this;

    const core = self.getCore();
    const l10n = core.l10n.bind(core);

    const rootId = self.getRootId();
    const recordSize = self.recordSize;
    document.querySelector(`#${rootId}`).innerHTML = /*html*/`
    <div class="container-recording">
      <div class="controls flex flex-between">
        <div>
          <button name="toggle-record">${l10n('Record')}</button>
          <button name="download" disabled>${l10n('Download')}</button>
        </div>
        <div>
          <span>
            <input type="radio" id="record-size-256m" name="maxRecordSize" value="${recordSize['256M']}" checked>
            <label for="record-size-256m">256M</label>
          </span>

          <span>
            <input type="radio" id="record-size-512m" name="maxRecordSize" value="${recordSize['512M']}">
            <label for="record-size-512m">512M</label>
          </span>

          <span>
            <input type="radio" id="record-size-1g" name="maxRecordSize" value="${recordSize['1G']}">
            <label for="record-size-1g">1G</label>
          </span>
        </div>
      </div>
      <div class="container-video">
        <div class="recording bg-black">
          <canvas name="video"></canvas>
          <canvas name="overlay"></canvas>
        </div>
        <div class="recorded bg-black">
          <video muted autoplay></video>
        </div>
      </div>
    </div>
    `;
  },

  register: async function({ clientWidth, clientHeight }) {
    const self = this;

    const toggleRecord = self.toggleRecord.bind(self);
    const resetRecordedChunks = self.resetRecordedChunks.bind(self);
    const setSizeLimit = self.setSizeLimit.bind(self);

    const rootId = self.getRootId();

    const containerDOM = document.querySelector(`#${rootId} .container-recording`);
    containerDOM.style.width = `${clientWidth}px`;

    const recordingContainerDOM = document.querySelector(`#${rootId} .container-recording .container-video .recording`);
    recordingContainerDOM.style.width = `${clientWidth}px`;
    recordingContainerDOM.style.height = `${clientHeight}px`;
    recordingContainerDOM.style.position = 'relative';

    const videoCanvasDOM = document.querySelector(`#${rootId} .container-recording .container-video .recording canvas[name="video"]`);
    videoCanvasDOM.width = clientWidth;
    videoCanvasDOM.height = clientHeight;
    videoCanvasDOM.style.position = 'absolute';

    const overlayCanvasDOM = document.querySelector(`#${rootId} .container-recording .container-video .recording canvas[name="overlay"]`);
    overlayCanvasDOM.width = clientWidth;
    overlayCanvasDOM.height = clientHeight;
    overlayCanvasDOM.style.position = 'absolute';

    const ctx = videoCanvasDOM.getContext('2d');
    ctx.fillStyle = 'black';
    ctx.fillRect(0, 0, clientWidth, clientHeight);

    const recordedContainerDOM = document.querySelector(`#${rootId} .container-recording .container-video .recorded`);
    recordedContainerDOM.style.width = `${clientWidth}px`;
    recordedContainerDOM.style.height = `${clientHeight}px`;
    recordedContainerDOM.style.display = 'none';

    const recordedVideoDOM = document.querySelector(`#${rootId} .container-recording .container-video .recorded video`);
    recordedVideoDOM.width = clientWidth;
    recordedVideoDOM.height = clientHeight;

    const toggleRecordDOM = document.querySelector(`#${rootId} .container-recording .controls button[name="toggle-record"]`);
    toggleRecordDOM.addEventListener('click', function() {
      toggleRecord({
        clientWidth, clientHeight,
        videoCanvasDOM, overlayCanvasDOM,
        recordingContainerDOM, recordedContainerDOM, recordedVideoDOM,
      });
    });

    const maxSize256DOM = document.querySelector(`#${rootId} .container-recording #record-size-256m`);
    const maxSize512DOM = document.querySelector(`#${rootId} .container-recording #record-size-512m`);
    const maxSize1024DOM = document.querySelector(`#${rootId} .container-recording #record-size-1g`);
    maxSize256DOM.addEventListener('change', (event) => {
      setSizeLimit(event.target.value);
    });
    maxSize512DOM.addEventListener('change', (event) => {
      setSizeLimit(event.target.value);
    });
    maxSize1024DOM.addEventListener('change', (event) => {
      setSizeLimit(event.target.value);
    });

    const recordSize = self.recordSize;
    setSizeLimit(recordSize['256M']);

    await resetRecordedChunks();
  },

  toggleRecord: async function({
    clientWidth, clientHeight,
    videoCanvasDOM, overlayCanvasDOM,
    recordingContainerDOM, recordedContainerDOM, recordedVideoDOM,
  }) {
    const self = this;

    const record = self.record.bind(self);
    const stop = self.stop.bind(self);
    const resetRecordedChunks = self.resetRecordedChunks.bind(self);
    const resetRecordedSize = self.resetRecordedSize.bind(self);

    const core = self.getCore();
    const l10n = core.l10n.bind(core);

    const rootId = self.getRootId();
    const isRecording = self.isRecording();

    const toggleRecordDOM = document.querySelector(`#${rootId} .container-recording .controls button[name="toggle-record"]`);
    const downloadDOM = document.querySelector(`#${rootId} .container-recording .controls button[name="download"]`);

    const maxSize256DOM = document.querySelector(`#${rootId} .container-recording #record-size-256m`);
    const maxSize512DOM = document.querySelector(`#${rootId} .container-recording #record-size-512m`);
    const maxSize1024DOM = document.querySelector(`#${rootId} .container-recording #record-size-1g`);

    if (isRecording) {
      stop({
        clientWidth, clientHeight,
        overlayCanvasDOM,
        recordingContainerDOM, recordedContainerDOM,
      });
      toggleRecordDOM.innerText = l10n('Record');
      downloadDOM.disabled = false;
      maxSize256DOM.disabled = false;
      maxSize512DOM.disabled = false;
      maxSize1024DOM.disabled = false;
    } else {
      await resetRecordedChunks();
      resetRecordedSize();
      record({
        clientWidth, clientHeight,
        videoCanvasDOM, overlayCanvasDOM,
        recordingContainerDOM, recordedContainerDOM, recordedVideoDOM,
      });
      toggleRecordDOM.innerText = l10n('Stop');
      downloadDOM.disabled = true;
      maxSize256DOM.disabled = true;
      maxSize512DOM.disabled = true;
      maxSize1024DOM.disabled = true;
    }
  },

  record: function({
    clientWidth, clientHeight,
    videoCanvasDOM, overlayCanvasDOM,
    recordingContainerDOM, recordedContainerDOM, recordedVideoDOM,
  }) {
    const self = this;

    const setMediaRecorder = self.setMediaRecorder.bind(self);
    const setRequestId = self.setRequestId.bind(self);
    const setDownloadUrl = self.setDownloadUrl.bind(self);
    const drawOverlay = self.drawOverlay.bind(self);
    const createRecordedVideo = self.createRecordedVideo.bind(self);
    const writeRecordedChunk = self.writeRecordedChunk.bind(self);
    const updateDownloadLink = self.updateDownloadLink.bind(self);
    const stop = self.stop.bind(self);

    const core = self.getCore();
    const drawImage = core.drawImage.bind(core);

    recordingContainerDOM.style.display = 'block';
    recordedContainerDOM.style.display = 'none';

    // preview
    const drawVideoFrame = function() {
      drawImage({ canvasDOM: videoCanvasDOM });
      drawOverlay({ clientWidth, clientHeight, canvasDOM: overlayCanvasDOM });
      setRequestId(window.requestAnimationFrame(drawVideoFrame));
    };
    window.requestAnimationFrame(drawVideoFrame);

    // recording
    const stream = videoCanvasDOM.captureStream();
    const options = { mimeType: 'video/webm' };
    const mediaRecorder = new MediaRecorder(stream, options);

    mediaRecorder.ondataavailable = function(event) {
      const blob = event.data;
      const blobSize = blob.size;
      const blobType = blob.type;

      const recordedSize = self.getRecordedSize();
      const sizeLimit = self.getSizeLimit();
      const rootId = self.getRootId();

      if (!(blobSize && blobSize > 0)) {
        throw new Error('Invalid blob size.');
      }

      const newSize = recordedSize + blobSize;
      const comp = newSize - sizeLimit;
      if (comp < 0) {
        writeRecordedChunk(blob);
      } else if (comp === 0) {
        writeRecordedChunk(blob);
        stop({
          clientWidth, clientHeight,
          overlayCanvasDOM,
          recordingContainerDOM, recordedContainerDOM,
        });
      } else if (comp > 0) {
        const slicedSize = blobSize - comp;
        const newBlob = blob.slice(0, slicedSize, blobType);
        writeRecordedChunk(newBlob);
        const toggleRecordDOM = document.querySelector(`#${rootId} .container-recording .controls button[name="toggle-record"]`);
        toggleRecordDOM.click();
      } else {
        throw new Error('Unknown error.');
      }
    };

    mediaRecorder.onstop = function(_) {
      const sizeLimit = self.getSizeLimit();
      createRecordedVideo(sizeLimit).then(blob => {
        URL.revokeObjectURL(self.getDownloadUrl());
        setDownloadUrl(URL.createObjectURL(blob));
        recordedVideoDOM.src = self.getDownloadUrl();
        updateDownloadLink({ url: recordedVideoDOM.src });
      });
    };

    setMediaRecorder(mediaRecorder);

    // set time slice to 1 minutes
    const timeslice = 60000;
    mediaRecorder.start(timeslice);
  },

  stop: function({
    clientWidth, clientHeight,
    overlayCanvasDOM,
    recordingContainerDOM, recordedContainerDOM,
  }) {
    const self = this;

    const destroyMediaRecorder = self.destroyMediaRecorder.bind(self);
    const cancelVideoFrame = self.cancelVideoFrame.bind(self);
    const clearCanvas = self.clearCanvas.bind(self);

    destroyMediaRecorder();
    cancelVideoFrame();
    clearCanvas({ clientWidth, clientHeight, canvasDOM: overlayCanvasDOM });

    recordingContainerDOM.style.display = 'none';
    recordedContainerDOM.style.display = 'block';
  },

  isRecording: function() {
    const self = this;
    const mediaRecorder = self.getMediaRecorder();
    return mediaRecorder && mediaRecorder.state === 'recording';
  },

  updateDownloadLink: function({ url }) {
    const self = this;

    const download = self.download.bind(self);

    const rootId = self.getRootId();

    const { removeEventListeners }  = window.augentixPlayerViewModel.utils;
    let downloadDOM = document.querySelector(`#${rootId} .container-recording .controls button[name="download"]`);
    removeEventListeners(downloadDOM);

    downloadDOM = document.querySelector(`#${rootId} .container-recording .controls button[name="download"]`);
    downloadDOM.addEventListener('click', function() {
      const createFileName = (date) => {
        const u = (date.getFullYear()).toString().padStart(4, '0');
        const v = (date.getMonth() + 1).toString().padStart(2, '0');
        const w = (date.getDate()).toString().padStart(2, '0');

        const x = (date.getHours()).toString().padStart(2, '0');
        const y = (date.getMinutes()).toString().padStart(2, '0');
        const z = (date.getSeconds()).toString().padStart(2, '0');

        return `recording_${u}${v}${w}_${x}${y}${z}.webm`;
      };
      const filename = createFileName(new Date());
      download({ url, filename });
    });
  },

  download: function({ url, filename }) {
    const downloadDOM = document.createElement('a');
    downloadDOM.target = '_blank';
    downloadDOM.href = url;
    downloadDOM.download = filename;
    downloadDOM.click();
  },

  drawOverlay: function({ clientWidth, clientHeight, canvasDOM }) {
    const ctx = canvasDOM.getContext('2d');

    const shouldDrawOverlay = ((new Date()).getMilliseconds()) < 500;
    if (shouldDrawOverlay) {
      ctx.fillStyle = 'red';

      const circleRadius = clientWidth / 64;
      const circleX = clientWidth - 3 * circleRadius;
      const circleY = circleRadius * 3;

      const circle = new Path2D();
      circle.moveTo(circleX, circleY);
      circle.arc(circleX, circleY, circleRadius, 0, 2 * Math.PI);

      ctx.fill(circle);
    } else {
      ctx.clearRect(0, 0, clientWidth, clientHeight);
    }
  },

  clearCanvas: function({ clientWidth, clientHeight, canvasDOM }) {
    const ctx = canvasDOM.getContext('2d');
    ctx.clearRect(0, 0, clientWidth, clientHeight);
  },

  destroyMediaRecorder: function() {
    const self = this;

    let mediaRecorder = self.getMediaRecorder();
    if (!mediaRecorder) {
      return;
    }

    const setMediaRecorder = self.setMediaRecorder.bind(self);

    mediaRecorder.stop();
    mediaRecorder = null;
    setMediaRecorder(mediaRecorder);
  },

  cancelVideoFrame: function() {
    const self = this;

    let requestId = self.getRequestId();
    if (!requestId) {
      return;
    }

    const setRequestId = self.setRequestId.bind(self);

    window.cancelAnimationFrame(requestId);
    requestId = 0;
    setRequestId(requestId);
  },
};
