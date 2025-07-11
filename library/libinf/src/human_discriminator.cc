#ifdef LIBEAI
#include "human_discriminator_internal.h"

#include "mpi_dev.h"
#include "inf_classifier.h"
#include "inf_log.h"
#include "inf_utils.h"

static constexpr bool VERBOSE = false;

static void release_inference_result(InfResult& result)
{
	free(result.cls);
	free(result.prob);
}

static void dump_selected_objects(uint32_t timestamp, const std::list<int32_t>& selected_ids)
{
	if (VERBOSE) {
		std::cout << "{" << timestamp << ": ";
		std::for_each(selected_ids.cbegin(), selected_ids.cend(), [](int32_t id) {
			std::cout << id << ',';
		});
		std::cout << "}" << std::endl;
	}
}


HumanDiscriminator::HumanDiscriminator(MPI_WIN win,
                                       int scene_width, int scene_height, int snapshot_width, int snapshot_height,
                                       InfModelCtx&& classifier)
	: _snapshot_width(snapshot_width), _snapshot_height(snapshot_height),
	  _width_ratio(static_cast<float>(snapshot_width) / scene_width),
	  _height_ratio(static_cast<float>(snapshot_height) / scene_height),
	  _win(win),
	  _interested_objects(MakeEventSource<EAI_HD, ObjectGroup>()),
	  _model_ctx(classifier)
{
	// HumanDiscriminator take ownership of model
	classifier.info = nullptr;
	classifier.model = nullptr;

	(void) dump_selected_objects;
	Observe(_interested_objects, [=] (ObjectGroup group) {
		dump_selected_objects(group.first, group.second);
		inspectObjects(group.first, group.second);
	});
}

HumanDiscriminator::~HumanDiscriminator()
{
	if (isModelReady()) {
		Inf_ReleaseModel(&_model_ctx);
	}
}

void HumanDiscriminator::inspectObjects(uint32_t timestamp, const std::list<int32_t>& object_ids)
{
	MPI_IVA_OBJ_LIST_S obj_list;
	int err = MPI_IVA_getBitStreamObjList(_win, timestamp, &obj_list);
	if (err != MPI_SUCCESS) {
		inf_log_err("FAILED to get OD list, err=%d", err);
		return;
	}

	MPI_VIDEO_FRAME_INFO_S frame{0};
	frame.width = _snapshot_width;
	frame.height = _snapshot_height;
	frame.type = MPI_SNAPSHOT_Y;

	err = MPI_DEV_getWinFrame(_win, &frame, 1000);
	if (err != MPI_SUCCESS) {
		inf_log_err("FAILED to get win frame, err=%d", err);
		return;
	}
	
	InfImage scene{
		.w = static_cast<int>(frame.width),
		.h = static_cast<int>(frame.height),
		.c = frame.depth == 24 ? 3 : 1,
		.data = nullptr,
		.buf_owner = 1,
		.dtype = frame.depth == 24 ? Inf8UC3 : Inf8UC1,
	};

	{
		VideoFrameGuard frame_holder(_win, &frame);
		scene.data = reinterpret_cast<uint8_t *>(malloc(frame.size));
		if (scene.data == nullptr) {
			inf_log_err("out of memory to process snapshot image");
			return;
		}

		memcpy(scene.data, frame.uaddr, frame.size);
	}
	ImageHolder image_holder(&scene);

	std::map<int32_t, MPI_RECT_POINT_S> rect_table;
	for (int i = 0; i < obj_list.obj_num; ++i) {
		MPI_IVA_OBJ_ATTR_S& obj = obj_list.obj[i];
		rect_table[obj.id] = obj.rect;
	}

	std::for_each(object_ids.begin(), object_ids.end(), [=, &rect_table] (int32_t id) {
		auto it = rect_table.find(id);
		if (it == rect_table.end()) {
			if (VERBOSE) {
				std::cout << "object #" << id << " is out of scene" << std::endl;
			}
			return;
		}

		auto& rect = it->second;
		InfImage obj_image{0};
		int sx = static_cast<int>(rect.sx * _width_ratio);
		int sy = static_cast<int>(rect.sy * _height_ratio);
		int ex = static_cast<int>(rect.ex * _width_ratio);
		int ey = static_cast<int>(rect.ey * _height_ratio);
		Inf_ImcropResize(&scene, sx, sy, ex, ey, &obj_image, ex-sx+1, ey-sy+1);
		ImageHolder obj_image_holder(&obj_image);

		InfResult result = runInference(id, obj_image);
		if (result.prob_num > 0) {
			synchronizedSession([=, &result] {
				_score_cache.put(id, updateHistory(_score_cache.get(id), result.prob[0]));
				uint32_t access_mark;
				if (_obj_queue.empty()) {
					access_mark = timestamp;
				} else {
					access_mark = _obj_queue.back().timestamp;
				}
				_last_snapshot_timestamp.put(id, access_mark * 10);
				if (VERBOSE) {
					std::cout << "last access #" << id << ": " << access_mark << std::endl;
				}
			});
		}
		release_inference_result(result);
	});
}

InfResult HumanDiscriminator::runInference(int32_t obj_id, InfImage& obj_image)
{
	if (!isModelReady()) {
		return InfResult{0};
	}

	if (VERBOSE) {
		std::cout << "Do inference for object #" << obj_id << std::endl;
	}
	MPI_IVA_OBJ_LIST_S obj_list{0};
	InfResultList results{0};
	obj_list.obj_num = 1;
	MPI_IVA_OBJ_ATTR_S& obj = obj_list.obj[0];
	obj.id = obj_id;
	obj.rect = {0, 0, static_cast<int16_t>(obj_image.w-1), static_cast<int16_t>(obj_image.h-1)};
	int err = Inf_InvokeClassify(&_model_ctx, &obj_image, &obj_list, &results);
	if (err != 0) {
		inf_log_err("FAILED to invoke classifier, err=%d", err);
		return InfResult{0};
	}

	assert(results.size == 1);
	InfResult result = results.data[0];
	if (VERBOSE) {
		std::cout << "Inference result=" << result.prob[0] << " for object #" << obj_id << std::endl;
	}
	free(results.data);
	return result;
}

std::list<int32_t> HumanDiscriminator::selectCandidates(uint32_t timestamp, const std::set<int32_t>& obj_ids)
{
	std::list<int32_t> selected_ids;
	std::copy_if(obj_ids.begin(), obj_ids.end(), std::back_inserter(selected_ids), [=] (int32_t obj_id) {
		int num_scores = historySize(_score_cache.get(obj_id));

		if (num_scores == 0) {
			return true;
		}

		int64_t time_passed_in_ms = timestamp * 10 - _last_snapshot_timestamp.get(obj_id);
		switch (num_scores) {
		case 1:
			return time_passed_in_ms > _priority_for_once;
		case 2:
			return time_passed_in_ms > _priority_for_twice;
		case 3:
			return time_passed_in_ms > _priority_for_steady;
		default:
			return false;
		}
	});
	return selected_ids;
}

void HumanDiscriminator::startServiceWith(const EventsT<OdEvent>& source)
{
	InfFloatList& conf_thresh = const_cast<InfFloatList&>(_model_ctx.info->conf_thresh);
	for (int i = 0; i < conf_thresh.size; ++i) {
		conf_thresh.data[i] = 0;
	}

	_hd_task = std::thread([=] {
		std::unique_lock<std::mutex> lock{_mutex};
		std::cout << "[" << (int)_win.chn << " " << (int)_win.win << "] HD background thread is running..." << std::endl;
		while (!_stop_service) {
			_queue_changed.wait(lock, [=]{ return _stop_service || !_obj_queue.empty(); });
			if (_stop_service) {
				break;
			}

			uint32_t timestamp = 0;
			std::set<int32_t> known_obj_ids;
			while (!_obj_queue.empty()) {
				auto obj = _obj_queue.front();
				_obj_queue.pop();
				known_obj_ids.emplace(obj.id);
				timestamp = std::max(timestamp, obj.timestamp);
			}
			std::list<int32_t> candidate_ids{std::move(selectCandidates(timestamp, known_obj_ids))};
			lock.unlock();
			if (!candidate_ids.empty()) {
				_interested_objects << std::make_pair(timestamp, candidate_ids);
			}
			lock.lock();
		}
	});

	_upstream_event_cont = MakeContinuation(source, [=] (const OdEvent& event) {
		if (VERBOSE) {
			std::cout << "found object #" << event.id << " @" << event.timestamp << std::endl;
		}
		synchronizedSession([=, &event] {
			OdEvent obj = event;
			obj.snapshot = nullptr;
			obj.attrs = nullptr;
			_obj_queue.emplace(obj);
			if (_obj_queue.size() >= 10) {
				if (VERBOSE) {
					std::cout << "_obj_queue too large, pop object = " << _obj_queue.size() << std::endl;
				}
				_obj_queue.pop();
			}
			auto hd_json = buildAttrs(_score_cache.get(event.id));
			if (hd_json != nullptr) {
				json_object_object_add(event.attrs, "hd", hd_json);
			}
		}, true);

		// Is this safe simply publish up-stream event? Or we should clone event object and publish?
		_event << event;
	});
}

void HumanDiscriminator::stopService()
{
	synchronizedSession([=] {
		_stop_service = true;
	}, true);

	if (_hd_task.joinable()) {
		_hd_task.join();
	}
}

void HumanDiscriminator::setPriorityFactor(uint32_t once, uint32_t twice, uint32_t steady)
{
	_priority_for_once = once;
	_priority_for_twice = twice;
	_priority_for_steady = steady;
}

std::tuple<uint32_t, uint32_t, uint32_t> HumanDiscriminator::getPriorityFactor()
{
	return std::make_tuple(_priority_for_once, _priority_for_twice, _priority_for_steady);
}

EaiHdContext *EAI_HD_create(MPI_WIN win,
                              int scene_width, int scene_height, int snapshot_width, int snapshot_height,
                              InfModelCtx *classifier)
{
	return reinterpret_cast<EaiHdContext *>(
		new HumanDiscriminator(win, scene_width, scene_height, snapshot_width, snapshot_height, std::move(*classifier)));
}

void EAI_HD_dispose(EaiHdContext *context)
{
	if (context != nullptr) {
		delete reinterpret_cast<HumanDiscriminator *>(context);
	}
}

void EAI_HD_startServiceWith(EaiHdContext *context, EaiOdEventPublisher *publisher)
{
	auto *hd_instance = reinterpret_cast<HumanDiscriminator *>(context);
	auto *source = reinterpret_cast<EventsT<OdEvent> *>(publisher);
	hd_instance->startServiceWith(*source);
}

EaiOdEventPublisher *EAI_HD_getPublisher(EaiHdContext *context)
{
	auto *hd_instance = reinterpret_cast<HumanDiscriminator *>(context);
	return reinterpret_cast<EaiOdEventPublisher *>(&hd_instance->event());
}

void EAI_HD_stopService(EaiHdContext *context)
{
	auto *hd_instance = reinterpret_cast<HumanDiscriminator *>(context);
	hd_instance->stopService();
}

void EAI_HD_setPriorityFactor(EaiHdContext *context, uint32_t once, uint32_t twice, uint32_t steady)
{
	auto *hd_instance = reinterpret_cast<HumanDiscriminator *>(context);
	uint32_t once_priority, twice_priority, steady_priority;
	std::tie(once_priority, twice_priority, steady_priority) = hd_instance->getPriorityFactor();
	if (once) {
		once_priority = once;
	}
	if (twice) {
		twice_priority = twice;
	}
	if (steady) {
		steady_priority = steady;
	}
	hd_instance->setPriorityFactor(once_priority, twice_priority, steady_priority);
}

void EAI_HD_getPriorityFactor(EaiHdContext *context, uint32_t *once, uint32_t *twice, uint32_t *steady)
{
	auto *hd_instance = reinterpret_cast<HumanDiscriminator *>(context);
	uint32_t once_priority, twice_priority, steady_priority;
	std::tie(once_priority, twice_priority, steady_priority) = hd_instance->getPriorityFactor();
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
