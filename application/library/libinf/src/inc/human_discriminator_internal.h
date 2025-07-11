#ifndef HUMAN_DISCRIMINATOR_INTERNAL_H_
#define HUMAN_DISCRIMINATOR_INTERNAL_H_

#include "human_discriminator.h"

#include <stdint.h>

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <tuple>
#include <set>
#include <list>
#include <map>
#include <algorithm>
#include <iterator>
#include <utility>
#include <math.h> /* for INFINITY */
// #include <fenv.h>

#include "react/Domain.h"
#include "react/Event.h"
#include "react/Observer.h"
#include "json.h"

#include "inf_image.h"
#include "inf_model.h"
#include "od_event_internal.h"
#include "lru_cache.h"

using namespace react;

REACTIVE_DOMAIN(EAI_HD, sequential_concurrent)
USING_REACTIVE_DOMAIN(EAI)

class HumanDiscriminator {
    public:
	HumanDiscriminator(MPI_WIN win, int scene_width, int scene_height, int snapshot_width, int snapshot_height,
	                   InfModelCtx &&classifier);
	~HumanDiscriminator();

	void startServiceWith(const EventsT<OdEvent> &source);
	void stopService();
	EventsT<OdEvent> &event()
	{
		return _event;
	}
	void setPriorityFactor(uint32_t once, uint32_t twice, uint32_t steady);
	std::tuple<uint32_t, uint32_t, uint32_t> getPriorityFactor();

    private:
	using SnapshotRecord = std::tuple<int32_t, uint32_t, InfImage>;
	using InfRecord = std::tuple<int32_t, uint32_t, InfResult>;
	using InfHistory = std::tuple<float, float, float>;
	using ObjectGroup = std::pair<uint32_t, std::list<int32_t> >;

	template <typename F> void synchronizedSession(F &&template_method, bool notify = false)
	{
		std::unique_lock<std::mutex> lock{ _mutex };
		template_method();
		if (notify) {
			_queue_changed.notify_one();
		}
	}

	bool isModelReady() const
	{
		return _model_ctx.model != nullptr;
	}

	static int historySize(const InfHistory &hist)
	{
		if (std::get<0>(hist) == -INFINITY) {
			return 0;
		} else if (std::get<1>(hist) == -INFINITY) {
			return 1;
		} else if (std::get<2>(hist) == -INFINITY) {
			return 2;
		} else {
			return 3;
		}
	}

	static InfHistory updateHistory(const InfHistory &hist, float new_score)
	{
		float s1;
		float s2;
		float s3;
		std::tie(s1, s2, s3) = hist;
		if (s1 == -INFINITY) {
			return std::make_tuple(new_score, s2, s3);
		}

		if (s2 == -INFINITY) {
			return std::make_tuple(s1, new_score, s3);
		}

		if (s3 == -INFINITY) {
			return std::make_tuple(s1, s2, new_score);
		}

		return std::make_tuple(s2, s3, new_score);
	}

	json_object *buildAttrs(const InfHistory &hist)
	{
		if (hist == NoScore) {
			return nullptr;
		}

		float s1;
		float s2;
		float s3;
		std::tie(s1, s2, s3) = hist;
		json_object *hd = json_object_new_object();
		json_object *scores = json_object_new_array();
		if (s1 != -INFINITY) {
			json_object_array_add(scores, json_object_new_double(s1));
		}
		if (s2 != -INFINITY) {
			json_object_array_add(scores, json_object_new_double(s2));
		}
		if (s3 != -INFINITY) {
			json_object_array_add(scores, json_object_new_double(s3));
		}
		json_object_object_add(hd, "scores", scores);
		return hd;
	}

	// result ownership transfer to caller
	InfResult runInference(int32_t obj_id, InfImage &obj_image);
	std::list<int32_t> selectCandidates(uint32_t timestamp, const std::set<int32_t> &obj_ids);
	void inspectObjects(uint32_t timestamp, const std::list<int32_t> &object_ids);

	const int _snapshot_width;
	const int _snapshot_height;
	const float _width_ratio;
	const float _height_ratio;
	const MPI_WIN _win;
	EAI_HD::EventSourceT<ObjectGroup> _interested_objects;
	InfModelCtx _model_ctx;

	EventSourceT<OdEvent> _event{ MakeEventSource<EAI, OdEvent>() };
	Continuation<EAI> _upstream_event_cont{};
	std::mutex _mutex{};
	std::condition_variable _queue_changed{};
	std::queue<OdEvent> _obj_queue{};
	bool _stop_service{ false };
	std::thread _hd_task{};

	const InfHistory NoScore{ std::make_tuple(-INFINITY, -INFINITY, -INFINITY) };
	LRUCache<int32_t, InfHistory> _score_cache{ 20, NoScore };
	LRUCache<int32_t, int64_t> _last_snapshot_timestamp{ 20, -1 };

	uint32_t _priority_for_once{ 300 };
	uint32_t _priority_for_twice{ 600 };
	uint32_t _priority_for_steady{ 1000 };
};

#endif /* HUMAN_DISCRIMINATOR_INTERNAL_H_ */
