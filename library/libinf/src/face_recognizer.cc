#ifdef LIBEAI
#include "face_recognizer_internal.h"

#include <cmath>
#include <vector>
#include <algorithm>

#include "mpi_dev.h"
#include "inf_log.h"
#include "inf_face.h"
#include "inf_face_internal.h"

static constexpr bool VERBOSE = false;
static constexpr bool STEAL_FACE_IMAGE = false;

static inline MPI_RECT_POINT_S intersection(const MPI_RECT_POINT_S& rect1, const MPI_RECT_POINT_S& rect2)
{
	MPI_RECT_POINT_S ret {
		.sx = std::max(rect1.sx, rect2.sx),
		.sy = std::max(rect1.sy, rect2.sy),
		.ex = std::min(rect1.ex, rect2.ex),
		.ey = std::min(rect1.ey, rect2.ey)
	};

	ret.ex = std::max(ret.ex, ret.sx);
	ret.ey = std::max(ret.ey, ret.sy);
	return ret;
}

static inline uint32_t rect_area(const MPI_RECT_POINT_S& rect)
{
	if (rect.ex > rect.sx && rect.ey > rect.sy) {
		return (rect.ex - rect.sx) * (rect.ey - rect.sy);
	}
	return 0;
}

static inline void dump_rect(const MPI_RECT_POINT_S& rect)
{
	std::cout << "(" << rect.sx << "," << rect.sy << ")-(" << rect.ex << "," << rect.ey << ")";
}

const MPI_RECT_POINT_S FaceRecognizer::INVALID_REGION{-1, -1, -1, -1};
const FaceRecognizer::IdentityHistory FaceRecognizer::EMPTY_HISTORY{std::string(), std::string(), std::string()};
const FaceRecognizer::ConfidenceHistory FaceRecognizer::EMPYT_CONF_HISTORY{-1, -1, -1};

FaceRecognizer::FaceRecognizer(MPI_WIN win,
                               int scene_width, int scene_height, int snapshot_width, int snapshot_height,
                               InfModelCtx&& fr_model)
	: _snapshot_width(snapshot_width), _snapshot_height(snapshot_height),
	  _scene_width(scene_width), _scene_height(scene_height),
	  _width_ratio(static_cast<float>(snapshot_width) / scene_width),
	  _height_ratio(static_cast<float>(snapshot_height) / scene_height),
	  _win(win),
	  _model_ctx(fr_model)
{
	fr_model.info = nullptr;
	fr_model.model = nullptr;
}

FaceRecognizer::~FaceRecognizer()
{
	if (_model_ctx.model != nullptr) {
		Inf_ReleaseModel(&_model_ctx);
	}
}

void FaceRecognizer::runIdentifyService()
{
	std::unique_lock<std::mutex> lock{_queue_mutex};
	while (_service_running.load()) {
		_queue_changed.wait(lock, [=] { return !_service_running.load() || !_face_queue.empty(); });
		if (!_service_running.load()) {
			break;
		}

		int32_t obj_id;
		InfImage face_image;
		std::tie(obj_id, face_image) = _face_queue.front();
		_face_queue.pop_front();
		lock.unlock();

		ImageHolder image_holder(&face_image);
		MPI_RECT_POINT_S roi{0, 0, static_cast<INT16>(face_image.w-1), static_cast<INT16>(face_image.h-1)};
		InfDetList results{0};
		int err = Inf_InvokeFaceRecoRoi(&_model_ctx, &face_image, &roi, &results);
		if (err) {
			inf_log_err("FAILED to invoke Inf_InvokeFaceRecoRoi, err=%d", err);
		} else {
			doInIdentitySession([=, &results] {
				_identity_cache.put(obj_id, updateHistory(_identity_cache.get(obj_id),
				                                          _model_ctx.info->labels.data[results.data[0].cls[0]]));
				_identity_conf.put(obj_id, updateConfidences(_identity_conf.get(obj_id), results.data[0].confidence));
				_last_identify_time.put(obj_id, _sensed_timestamp.load() * 10);
			});
		}
		Inf_ReleaseDetResult(&results);
		lock.lock();
	}
}

void FaceRecognizer::startServiceWith(const EventsT<OdEvent>& source, uint32_t sensitivity)
{
	_service_running = true;

	_fr_task = std::thread([=] {
		std::cout << "Face identifying background thread is running..." << std::endl;
		runIdentifyService();
		std::cout << "Face identifying background thread exited." << std::endl;
	});

	InfFaceReco *fr_inf_model = reinterpret_cast<InfFaceReco *>(_model_ctx.model);
	InfModelCtx fd_model{fr_inf_model->m_face_detect->m_config, fr_inf_model->m_face_detect.get()};
	FaceDetector *face_detector = new FaceDetector(_win, _scene_width, _scene_height,
	                                               _snapshot_width, _snapshot_height, std::move(fd_model));
	
	if (VERBOSE) {
		Observe(face_detector->event(), [] (const OdEvent& event) {
			const MPI_RECT_POINT_S *rect = &event.rect;
			printf("[%u] obj(%d)<%d>, rect: (%d, %d) - (%d, %d), mv: (%d, %d), attrs=%s\n",
					event.timestamp, event.id, event.life,
					rect->sx, rect->sy, rect->ex, rect->ey,
					event.mv.x, event.mv.y,
					json_object_to_json_string(event.attrs));
		});
	}

	EventsT<std::pair<int32_t, OdEvent>> host_face_pair = Transform(
		face_detector->event(),[=] (const OdEvent& event) {
			return std::make_pair(faceBelongsTo(event.rect), event);
		}
	);

	_interested_faces = Filter(host_face_pair, [=] (const std::pair<int32_t, OdEvent>& host_face) {
		int32_t host_id = host_face.first;
		if (host_id < 0) {
			return false;
		}

		int hist_size = 0;
		int64_t last_identify_time = 0;
		doInIdentitySession([=, &hist_size, &last_identify_time] {
			hist_size = historySize(_identity_cache.get(host_id));
			last_identify_time = _last_identify_time.get(host_id);
		});

		if (hist_size == 0) {
			return true;
		}

		int64_t time_passes_in_ms = _sensed_timestamp.load() * 10 - last_identify_time;
		switch (hist_size) {
		case 1:
			return time_passes_in_ms > _priority_for_once;
		case 2:
			return time_passes_in_ms > _priority_for_twice;
		case 3:
			return time_passes_in_ms > _priority_for_steady;
		default:
			return false;
		}
	});

	Observe(_interested_faces, [=] (const std::pair<int32_t, OdEvent>& host_face) {
		const OdEvent& event = host_face.second;
		int32_t host_id = host_face.first;
		if (STEAL_FACE_IMAGE) {
			InfImage face_image = *event.snapshot;
			face_image.buf_owner = 1;
			event.snapshot->buf_owner = 0;
			event.snapshot->data = nullptr;
			doInQueueSession([=] {
				_face_queue.emplace_back(host_id, face_image);
			});
		} else {
			InfImage image = *event.snapshot;
			size_t image_size = GetImageSize(*event.snapshot);
			image.buf_owner = 1;
			image.data = new uint8_t[image_size];
			memcpy(image.data, event.snapshot->data, image_size);
			doInQueueSession([=] {
				_face_queue.emplace_back(host_id, image);
			});
		}
	});

	auto stop_fd = [=] { return !_service_running.load(); };
	// run face detector in background
	_fd_task = std::thread([=] {
		face_detector->publish(sensitivity, true, stop_fd);
		face_detector->detachModel();
		delete face_detector;
	});

	_upstream_event_cont = MakeContinuation(source, [=] (const OdEvent& event) {
		if (VERBOSE) {
			std::cout << "found object #" << event.id << " @" << event.timestamp << std::endl;
		}

		if (event.timestamp > _sensed_timestamp.load()) {
			_sensed_timestamp = event.timestamp;
		}
		doInRegionSession([=, &event] {
			_obj_region_cache.put(event.id, event.rect);
		});

		doInIdentitySession([=, &event] {
			auto fr_json = build_fr_attrs(_identity_cache.get(event.id), _identity_conf.get(event.id));
			if (fr_json != nullptr) {
				json_object_object_add(event.attrs, ATTRIBUTE_ROOT, fr_json);
			}
		});
		_event << event;
	});
}

void FaceRecognizer::stopService()
{
	_service_running = false;
	doInQueueSession([] {});
	if (_fr_task.joinable()) {
		_fr_task.join();
	}
	if (_fd_task.joinable()) {
		_fd_task.join();
	}

	while (!_face_queue.empty()) {
		InfImage image;
		std::tie(std::ignore, image) = _face_queue.front();
		_face_queue.pop_front();
		Inf_Imrelease(&image);
	}
}

int32_t FaceRecognizer::faceBelongsTo(const MPI_RECT_POINT_S& face_rect)
{
	std::vector<std::pair<int32_t, float>> id_overlap_mapping;
	doInRegionSession([=, &id_overlap_mapping] {
		id_overlap_mapping.reserve(_obj_region_cache.size());
		_obj_region_cache.visit_cache(
			[=, &face_rect, &id_overlap_mapping] (int32_t obj_id, const MPI_RECT_POINT_S& rect) {
				auto overlap_ratio = intersectionRatio(rect, face_rect);
				if (VERBOSE) {
					std::cout << "Test OR #" << obj_id << " ";
					dump_rect(rect);
					std::cout << " -> " << overlap_ratio << std::endl;
				}
				if (overlap_ratio >= _face_overlap_ratio) {
					id_overlap_mapping.emplace_back(obj_id, overlap_ratio);
				}
			});
	});
	if (id_overlap_mapping.empty()) {
		return -1;
	}

	std::sort(id_overlap_mapping.begin(), id_overlap_mapping.end(),
		[] (const auto& item1, const auto& item2) {
			return item2.second > item1.second;
		});
	if (VERBOSE) {
		std::cout << "face ";
		dump_rect(face_rect);
		std::cout << " belongs to "
				  << "#" << id_overlap_mapping[0].first << ", OR: " << id_overlap_mapping[0].second << std::endl;
	}
	return id_overlap_mapping[0].first;
}

bool FaceRecognizer::isValidRegion(const MPI_RECT_POINT_S& region)
{
	return region.sx != INVALID_REGION.sx
	       && region.sy != INVALID_REGION.sy
	       && region.ex != INVALID_REGION.ex
	       && region.ey != INVALID_REGION.ey;
}

float FaceRecognizer::intersectionRatio(const MPI_RECT_POINT_S& test, const MPI_RECT_POINT_S& target)
{
	auto overlap = intersection(test, target);
	return static_cast<float>(rect_area(overlap)) / rect_area(target);
}

int FaceRecognizer::historySize(const IdentityHistory& hist)
{
	if (std::get<0>(hist).empty()) {
		return 0;
	}

	if (std::get<1>(hist).empty()) {
		return 1;
	}
	
	if (std::get<2>(hist).empty()) {
		return 2;
	}

	return 3;
}

FaceRecognizer::IdentityHistory FaceRecognizer::updateHistory(const IdentityHistory& hist, std::string identity)
{
	std::string id1, id2, id3;
	std::tie(id1, id2, id3) = hist;
	if (id1.empty()) {
		return std::make_tuple(identity, id2, id3);
	}

	if (id2.empty()) {
		return std::make_tuple(id1, identity, id3);
	}

	if (id3.empty()) {
		return std::make_tuple(id1, id2, identity);
	}

	return std::make_tuple(id2, id3, identity);
}

FaceRecognizer::ConfidenceHistory FaceRecognizer::updateConfidences(const ConfidenceHistory& hist, float conf)
{
	float conf1, conf2, conf3;
	std::tie(conf1, conf2, conf3) = hist;
	if (conf1 < 0) {
		return std::make_tuple(conf, conf2, conf3);
	}

	if (conf2 < 0) {
		return std::make_tuple(conf1, conf, conf3);
	}

	if (conf3 < 0) {
		return std::make_tuple(conf1, conf2, conf);
	}

	return std::make_tuple(conf2, conf3, conf);
}

json_object *FaceRecognizer::build_fr_attrs(const IdentityHistory& hist, const ConfidenceHistory& confs)
{
	if (hist == EMPTY_HISTORY) {
		return nullptr;
	}

	std::string name1;
	std::string name2;
	std::string name3;
	std::tie(name1, name2, name3) = hist;
	float conf1;
	float conf2;
	float conf3;
	std::tie(conf1, conf2, conf3) = confs;
	json_object *fr = json_object_new_object();
	json_object *records = json_object_new_array();
	json_object_object_add(fr, IDENTITY_KEY, records);
	char conf_text[11];
	if (!name1.empty()) {
		auto r = json_object_new_object();
		json_object_object_add(r, ID_NAME_KEY, json_object_new_string(name1.c_str()));
		snprintf(conf_text, sizeof(conf_text), "%.3f", conf1);
		json_object_object_add(r, ID_CONF_KEY, json_object_new_double_s(conf1, conf_text));
		json_object_array_add(records, r);
	}
	if (!name2.empty()) {
		auto r = json_object_new_object();
		json_object_object_add(r, ID_NAME_KEY, json_object_new_string(name2.c_str()));
		snprintf(conf_text, sizeof(conf_text), "%.3f", conf2);
		json_object_object_add(r, ID_CONF_KEY, json_object_new_double_s(conf2, conf_text));
		json_object_array_add(records, r);
	}
	if (!name3.empty()) {
		auto r = json_object_new_object();
		json_object_object_add(r, ID_NAME_KEY, json_object_new_string(name3.c_str()));
		snprintf(conf_text, sizeof(conf_text), "%.3f", conf3);
		json_object_object_add(r, ID_CONF_KEY, json_object_new_double_s(conf3, conf_text));
		json_object_array_add(records, r);
	}

	return fr;
}

void FaceRecognizer::setPriorityFactor(uint32_t once, uint32_t twice, uint32_t steady)
{
	_priority_for_once = once;
	_priority_for_twice = twice;
	_priority_for_steady = steady;
}

std::tuple<uint32_t, uint32_t, uint32_t> FaceRecognizer::getPriorityFactor()
{
	return std::make_tuple(_priority_for_once, _priority_for_twice, _priority_for_steady);
}

// C API
const char *const ATTRIBUTE_ROOT = "fr";
const char *const IDENTITY_KEY = "identities";
const char *const ID_NAME_KEY = "title";
const char *const ID_CONF_KEY = "confidence";

EaiFrContext *EAI_FR_create(MPI_WIN win,
                              int scene_width, int scene_height, int snapshot_width, int snapshot_height,
                              InfModelCtx *recognizer)
{
	return reinterpret_cast<EaiFrContext *>(
		new FaceRecognizer(win, scene_width, scene_height, snapshot_width, snapshot_height, std::move(*recognizer)));
}

void EAI_FR_dispose(EaiFrContext *context)
{
	if (context != nullptr) {
		delete reinterpret_cast<FaceRecognizer *>(context);
	}
}

void EAI_FR_startServiceWith(EaiFrContext *context, EaiOdEventPublisher *publisher, uint32_t sensitivity)
{
	auto *fr_instance = reinterpret_cast<FaceRecognizer *>(context);
	auto *source = reinterpret_cast<EventsT<OdEvent> *>(publisher);
	fr_instance->startServiceWith(*source, sensitivity);
}

EaiOdEventPublisher *EAI_FR_getPublisher(EaiFrContext *context)
{
	auto *fr_instance = reinterpret_cast<FaceRecognizer *>(context);
	return reinterpret_cast<EaiOdEventPublisher *>(&fr_instance->event());
}

void EAI_FR_stopService(EaiFrContext *context)
{
	auto *fr_instance = reinterpret_cast<FaceRecognizer *>(context);
	fr_instance->stopService();
}

void EAI_FR_setConfThreshold(EaiFrContext *context, float threshold)
{
	auto *fr_instance = reinterpret_cast<FaceRecognizer *>(context);
	auto *inf_model = reinterpret_cast<InfFaceReco *>(fr_instance->_model_ctx.model);
	inf_model->m_config->conf_thresh.data[0] = threshold;
}

void EAI_FR_setPriorityFactor(EaiFrContext *context, uint32_t once, uint32_t twice, uint32_t steady)
{
	auto *fr_instance = reinterpret_cast<FaceRecognizer *>(context);
	uint32_t once_priority, twice_priority, steady_priority;
	std::tie(once_priority, twice_priority, steady_priority) = fr_instance->getPriorityFactor();
	if (once) {
		once_priority = once;
	}
	if (twice) {
		twice_priority = twice;
	}
	if (steady) {
		steady_priority = steady;
	}
	fr_instance->setPriorityFactor(once_priority, twice_priority, steady_priority);
}

void EAI_FR_getPriorityFactor(EaiFrContext *context, uint32_t *once, uint32_t *twice, uint32_t *steady)
{
	auto *fr_instance = reinterpret_cast<FaceRecognizer *>(context);
	uint32_t once_priority, twice_priority, steady_priority;
	std::tie(once_priority, twice_priority, steady_priority) = fr_instance->getPriorityFactor();
	if (once) {
		*once = once_priority;
	}
	if (twice) {
		*twice = twice_priority;
	}
	if (steady) {
		*steady = steady_priority;
	}
}
#endif /* LIBEAI */