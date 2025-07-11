#ifdef LIBEAI
#include "face_detector_internal.h"

#include "mpi_dev.h"
#include "inf_log.h"
#include "inf_utils.h"
#include "inf_face.h"


FaceDetector::FaceDetector(MPI_WIN win, int scene_width, int scene_height, int snapshot_width, int snapshot_height,
                           InfModelCtx&& detector)
	: _snapshot_width(snapshot_width), _snapshot_height(snapshot_height),
	  _width_ratio(static_cast<float>(snapshot_width) / scene_width),
	  _height_ratio(static_cast<float>(snapshot_height) / scene_height),
	  _win(win),
	  _model_ctx(detector)
{
	detector.info = nullptr;
	detector.model = nullptr;
}

FaceDetector::~FaceDetector()
{
	if (is_model_ready()) {
		Inf_ReleaseModel(&_model_ctx);
	}
}

void FaceDetector::publish(uint32_t sensitivity, bool with_snapshot, std::function<bool()> unsubscribe)
{
	const int timeout_ms = 1000;
	uint32_t last_timestamp = 0;
	uint32_t timestamp;
	uint32_t next_id = 1;

	inf_log_info("FaceDetector Event Source start publish.");
	json_object *source_name = json_object_new_string("FaceDetector");
	while (!unsubscribe()) {
		int ret = MPI_DEV_waitWin(_win, &timestamp, timeout_ms);
		if (ret != MPI_SUCCESS) {
			inf_log_err("FAILED to waitWin for win: (%d, %d, %d) in %d ms. err: %d",
			            _win.dev, _win.chn, _win.win, timeout_ms, ret);
			continue;
		}

		inf_log_debug("Current timestamp: %u", timestamp);
		uint32_t time_diff_ms = (timestamp - last_timestamp) * 10;
		if (last_timestamp > 0 && time_diff_ms < sensitivity) {
			inf_log_debug("%u < %u, idle", time_diff_ms, sensitivity);
			continue;
		}

		last_timestamp = timestamp;
		MPI_VIDEO_FRAME_INFO_S frame{0};
		frame.width = _snapshot_width;
		frame.height = _snapshot_height;
		frame.type = _model_ctx.info->c == 3 ? MPI_SNAPSHOT_RGB : MPI_SNAPSHOT_Y;
		int err = MPI_DEV_getWinFrame(_win, &frame, timeout_ms);
		if (err != MPI_SUCCESS) {
			inf_log_err("FAILED to get win frame, err=%d", err);
			// see: #53403#note-9
			if (err == -EAGAIN) {
				// MAYBE last invocation is interrupted by signal
				// this is a workaround
				VideoFrameGuard guard(_win, &frame);
			}
			continue;
		}

		InfImage scene{
			.w = static_cast<int>(frame.width),
			.h = static_cast<int>(frame.height),
			.c = _model_ctx.info->c,
			.data = reinterpret_cast<uint8_t *>(malloc(frame.size)),
			.buf_owner = 1,
			.dtype = _model_ctx.info->c == 3 ? Inf8UC3 : Inf8UC1,
		};
		ImageHolder scene_holder(&scene);
		
		{
			VideoFrameGuard frame_holder(_win, &frame);
			memcpy(scene.data, frame.uaddr, frame.size);
		}
		
		InfDetList results{0};
		err = Inf_InvokeFaceDet(&_model_ctx, &scene, &results);
		if (err != 0) {
			inf_log_err("FAILED to invoke detector, err=%d", err);
			continue;
		}

		inf_log_debug("%d faces found.", results.size);
		for (int i = 0; i < results.size; ++i) {
			InfDetResult& res = results.data[i];
			OdEvent event{
				.timestamp = timestamp,
				.id = static_cast<int32_t>(next_id++),
				.life = 0,
				.rect = res.rect,
				.mv = {0},
				.snapshot = 0,
				.attrs = json_object_new_object()
			};
			rescaleBox(event.rect);
			json_object_object_add(event.attrs, "event-source", json_object_get(source_name));

			if (with_snapshot) {
				const MPI_RECT_POINT_S& face = res.rect;
				event.snapshot = new InfImage {0};
				Inf_ImcropResize(&scene, face.sx, face.sy, face.ex, face.ey, event.snapshot,
				                 face.ex - face.sx + 1, face.ey - face.sy + 1);
			}

			_emitter << event;
			json_object_put(event.attrs);
			if (event.snapshot) {
				if (event.snapshot->buf_owner) {
					Inf_Imrelease(event.snapshot);
				}
				delete event.snapshot;
			}
		}
		Inf_ReleaseDetResult(&results);
	}
	json_object_put(source_name);
	inf_log_info("FaceDetector Event Source end publish.");
}

void FaceDetector::rescaleBox(MPI_RECT_POINT_S& rect)
{
	rect.sx = static_cast<INT16>(rect.sx / _width_ratio);
	rect.ex = static_cast<INT16>(rect.ex / _width_ratio);
	rect.sy = static_cast<INT16>(rect.sy / _height_ratio);
	rect.ey = static_cast<INT16>(rect.ey / _height_ratio);
}

struct eai_fd_context *EAI_FD_create(MPI_WIN win,
                                       int scene_width, int scene_height, int snapshot_width, int snapshot_height,
                                       InfModelCtx *detector)
{
	return reinterpret_cast<struct eai_fd_context *>(
		new FaceDetector(win, scene_width, scene_height, snapshot_width, snapshot_height, std::move(*detector)));
}

void EAI_FD_dispose(struct eai_fd_context *context)
{
	if (context != nullptr) {
		delete reinterpret_cast<FaceDetector *>(context);
	}
}

EaiOdEventPublisher *EAI_FD_getPublisher(EaiFdContext *context)
{
	auto *fd_instance = reinterpret_cast<FaceDetector *>(context);
	return reinterpret_cast<EaiOdEventPublisher *>(&fd_instance->event());
}

void EAI_FD_publish(struct eai_fd_context *context, uint32_t sensitivity, bool with_snapshot, publish_unsubscribe unsubscribe)
{
	FaceDetector *fd_instance = reinterpret_cast<FaceDetector *>(context);
	fd_instance->publish(sensitivity, with_snapshot, [unsubscribe] { return unsubscribe(); });
}
#endif /* LIBEAI */
