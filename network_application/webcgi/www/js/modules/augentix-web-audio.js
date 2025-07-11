// @ts-check

import {
  MediaRecorder,
  register as rgstr,
} from './extendable-media-recorder-6.5.10.min.js'

import {
  connect as connectPcmEncoder,
} from './extendable-media-recorder-pcm-encoder-7.0.62.min.js';

import { WaveFile } from './wavefile-11.0.0.min.js';

/**
 * @typedef {Object} AudioOptions
 * @property {number} langId
 * @property {string} websocketURL
 * @property {string} [mimeType]
 * @property {number} [timeslice]
 * @property {number} [logging]
 * @property {(this: WebSocket, ev: Event) => any} [onopen]
 * @property {(this: WebSocket, ev: CloseEvent) => any} [onclose]
 * @property {(ev: MessageEvent) => any} [onmessage]
 * @property {(this: WebSocket, ev: Event) => any} [onerror]
 */

/**
 * @typedef {Object} EncodeOptions
 * @property {string} type
 * @property {number} channelCount
 * @property {number} sampleRateIn
 * @property {number} sampleSizeIn
 */

/**
 * @typedef {Object} Message
 * @property {number} id
 * @property {string} type
 * @property {string} [mimeType]
 * @property {number[]} [data]
 * @property {number} [length]
 */

export class AugentixWebAudio {
  #Error = {
    INVALID_WS_URL: 'Invalid WebSocket server URL.',
    NO_AUDIO_TRACK: 'Audio track not found.',
    NO_MEDIA_STREAM: 'Media stream not found.',
    NO_WS_CONNECTION: 'No WebSocket connection.',
    UNSUPPORT_CHANNEL_COUNT: 'Expect mono audio, but got stereo.',
  };

  #LogLevel = {
    OFF: 0,
    WARN: 1,
    INFO: 2,
    DEBUG: 3,
    TRACE: 4,
    ALL: 5,
  };

  #L10nTable = {
    Call: ['Call', '通話', '通话'],
    HangUp: ['Hang Up', '掛斷', '挂断'],
  };

  /** @type { AudioOptions } */
  #options = {
    langId: 0,
    websocketURL: '',
    mimeType: 'audio/pcma',
    timeslice: 800,
    logging: this.#LogLevel.INFO,
    onopen: null,
    onclose: null,
    onmessage: null,
    onerror: null,
  };

  /** @type { WebSocket | null } */
  #ws = null;

  /** @type { MediaStream | null } */
  #stream = null;

  #mediaRecorder = null;

  get langId() {
    return this.#options.langId;
  }

  /**
   * @param {number} value
   */
  set langId(value) {
    this.#options.langId = value;
  }

  get mimeType() {
    return this.#options.mimeType;
  }

  /**
   * @param {string} value
   */
  set mimeType(value) {
    this.#options.mimeType = value;
  }

  get timeslice() {
    return this.#options.timeslice;
  }

  /**
   * @param {number} value
   */
  set timeslice(value) {
    this.#options.timeslice = value;
  }

  get websocketURL() {
    return this.#options.websocketURL;
  }

  /**
   * @param {string} value
   */
  set websocketURL(value) {
    this.#options.websocketURL = value;
  }

  /**
   * @param {AudioOptions} options
   */
  constructor(options) {
    if (options) {
      this.#options = { ...this.#options, ...options };
    }

    this.render();
  }

  /**
   * @param {AudioOptions} options
   */
  static async create(options) {
    const o = new AugentixWebAudio(options);
    await o.init();
    return o;
  }

  warn() {
    if (this.#options.logging >= this.#LogLevel.WARN) {
      console.warn.apply(console, ['[WARN]', ...arguments]);
    }
  }

  info() {
    if (this.#options.logging >= this.#LogLevel.INFO) {
      console.info.apply(console, ['[INFO]', ...arguments]);
    }
  }

  /**
   * @param {string} key
   */
  l10n(key) {
    const entry = this.#L10nTable[key];

    if (!key) {
      return '';
    }

    return entry
      ? entry[this.#options.langId]
      : key;
  }

  render() {
    const l10n = this.l10n.bind(this);

    /** @type {HTMLElement} */
    const root = document.querySelector('#agtx-voice-call');
    root.innerHTML = /*html*/`
      <input type="radio" id="g711a" name="codec" value="g711a" />
      <label for="g711a">A-law</label>
      <input type="radio" id="g711u" name="codec" value="g711u" />
      <label for="g711u">μ-law</label>
      <button type="button" id="startAudioCall">${l10n('Call')}</button>
      <button type="button" id="stopAudioCall">${l10n('HangUp')}</button>
    `;

    /** @type {HTMLInputElement} */
    const g711aRadio = document.querySelector('#g711a');
    /** @type {HTMLInputElement} */
    const g711uRadio = document.querySelector('#g711u');
    switch (this.#options.mimeType) {
      case 'audio/pcmu':
        g711uRadio.checked = true;
        break;
      case 'audio/pcma':
      default:
        g711aRadio.checked = true;
        break;
    }
  }

  async init() {
    await rgstr(await connectPcmEncoder());

    /** @type {HTMLInputElement} */
    const g711aRadio = document.querySelector('#g711a');
    g711aRadio.addEventListener('click', () => {
      this.#options.mimeType = 'audio/pcma';
    });

    /** @type {HTMLInputElement} */
    const g711uRadio = document.querySelector('#g711u');
    g711uRadio.addEventListener('click', () => {
      this.#options.mimeType = 'audio/pcmu';
    });

    /** @type {HTMLButtonElement} */
    const startCallBtn = document.querySelector('#startAudioCall');
    startCallBtn.addEventListener('click',
      this.startAudioCall.bind(this),
    );

    /** @type {HTMLButtonElement} */
    const stopCallBtn = document.querySelector('#stopAudioCall');
    stopCallBtn.addEventListener('click',
      this.stopAudioCall.bind(this),
    );
  }

  /**
   * @throws {Error} If no WebSocket connection is found.
   */
  sendPing() {
    if (!this.#ws) {
      throw new Error(this.#Error.NO_WS_CONNECTION);
    }
    const type = 'ping';
    const obj  = { type };
    this.sendMessage(obj);
  }

  /**
   * @throws {Error} If no WebSocket connection is found.
   */
  sendPong() {
    if (!this.#ws) {
      throw new Error(this.#Error.NO_WS_CONNECTION);
    }
    const type = 'pong';
    const obj  = { type };
    this.sendMessage(obj);
  }

  onOpen() {
    this.info(`WSS server ${this.#options.websocketURL} is connected.`);
    this.notifyCallStart();
  };

  onClose() {
    this.info(`WSS server ${this.#options.websocketURL} is disconnected.`);
  }

  /**
   * @param {MessageEvent} event
   */
  onMessage(event) {
    if (typeof(event.data) !== 'string') {
      throw new Error('Unknown message from server.');
    }

    let message = null;
    try {
      message = JSON.parse(event.data);
    } catch (error) {
      throw new Error('Unknown message from server.');
    }

    if (message.type === 'ping') {
      this.sendPong();
    }
  };

  /**
   * @throws {Error} If no WebSocket URL is found.
   */
  openWebSocket() {
    this.closeWebSocket();

    const url = this.#options.websocketURL;
    if (!url) {
      throw new Error(this.#Error.INVALID_WS_URL);
    }

    this.#ws = new WebSocket(url);
    this.#ws.onopen = this.onOpen.bind(this);
    this.#ws.onclose = this.onClose.bind(this);
    this.#ws.onmessage = this.onMessage.bind(this);
    this.#ws.onerror = this.#options.onerror;
  }

  closeWebSocket() {
    if (this.#ws) {
      this.#ws.close();
      this.#ws = null;
    }
  }

  /**
   * @param {Blob} blob
   * @param {EncodeOptions} options
   * @throws {Error} If number of channels is not 1.
   */
  async encode(blob, {
    type, channelCount, sampleRateIn, sampleSizeIn,
  }) {
    if (channelCount !== 1) {
      throw new Error(this.#Error.UNSUPPORT_CHANNEL_COUNT);
    }

    const bitDepth = sampleSizeIn.toString();
    const samples = new Int16Array(await blob.arrayBuffer());
    const wav = new WaveFile();
    wav.fromScratch(channelCount, sampleRateIn, bitDepth, samples);

    const sampleRateOut = 8000;
    wav.toSampleRate(sampleRateOut);

    let headerSize = 0;
    if (type === 'audio/pcma') {
      wav.toALaw();
      headerSize = 60;
    } else if (type === 'audio/pcmu') {
      wav.toMuLaw();
      headerSize = 60;
    } else {
      headerSize = 44;
    }

    const newBuffer = wav.toBuffer().slice(headerSize);
    const newBlob = new Blob([newBuffer], { type });

    return newBlob;
  }

  notifyCallStart() {
    const type = 'call_start';
    const obj = { type };
    this.sendMessage(obj);
  }

  notifyCallEnd() {
    const type = 'call_end';
    const obj = { type };
    this.sendMessage(obj);
  }

  /**
   * @param {Blob} blob
   * @param {{type: string}} options
   */
  async sendData(blob, { type: mimeType }) {
    const arrayBuffer = await blob.arrayBuffer();
    const arr = new Uint8Array(arrayBuffer);

    const type = 'data_available';
    const data = [...arr];
    const length = blob.size;
    const obj = {
      type,
      mimeType,
      data,
      length,
    };
    this.sendMessage(obj);
  }

  /**
   * @param {Object} obj
   */
  sendMessage(obj) {
    const id = Date.now();
    const newObj = { id, ...obj };
    const message = JSON.stringify(newObj);
    this.#ws.send(message);
    this.info('Send message', newObj);
  }

  /**
   * @throws {Error} If no media stream is found.
   * @throws {Error} If no audio track is found.
   */
  getUserAudioSettings() {
    if (!this.#stream) {
      throw new Error(this.#Error.NO_MEDIA_STREAM);
    }

    const [ firstAudioTrack ] = this.#stream.getAudioTracks();
    if (!firstAudioTrack) {
      throw new Error(this.#Error.NO_AUDIO_TRACK);
    }

    return firstAudioTrack.getSettings();
  }

  /**
   * @throws {Error} If no WebSocket connection is found.
   * @throws {Error} If no media stream is found.
   */
  async startAudioCall() {
    // Initialize user media stream
    try {
      this.#stream = await navigator.mediaDevices.getUserMedia({
        audio: true,
      });
    } catch (error) {
      throw new Error(this.#Error.NO_MEDIA_STREAM);
    }

    // Initialize the audio recorder
    this.#mediaRecorder = new MediaRecorder(this.#stream, {
      mimeType: 'audio/pcm',
    });

    this.#mediaRecorder.ondataavailable = async({ data }) => {
      const type = this.#options.mimeType;
      const {
        // @ts-ignore
        channelCount,
        sampleRate: sampleRateIn,
        sampleSize: sampleSizeIn,
      } = this.getUserAudioSettings();
      const options = {
        type, channelCount, sampleRateIn, sampleSizeIn,
      };
      const blob = await this.encode(data, options);
      await this.sendData(blob, { type });
    };

    this.#mediaRecorder.start(this.#options.timeslice);

    // Open WebSocket connection
    this.openWebSocket();
  }

  /**
   * @throws {Error} If no WebSocket connection is found.
   * @throws {Error} If no user audio settings is found.
   */
  stopAudioCall() {
    this.#mediaRecorder.ondataavailable = async({ data }) => {
      const type = this.#options.mimeType;
      const {
        // @ts-ignore
        channelCount,
        sampleRate: sampleRateIn,
        sampleSize: sampleSizeIn,
      } = this.getUserAudioSettings();
      const options = {
        type, channelCount, sampleRateIn, sampleSizeIn,
      };
      const blob = await this.encode(data, options);
      await this.sendData(blob, { type });
      this.notifyCallEnd();

      if (this.#mediaRecorder.state === 'inactive') {
        this.#mediaRecorder = null;
        this.closeWebSocket();
      }
    };
    this.#mediaRecorder.stop();

    this.#stream.getAudioTracks().forEach((track) => {
      track.stop();
    });
  }
}
