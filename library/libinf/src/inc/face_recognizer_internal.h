#ifndef FACE_RECOGNIZER_INTERNAL_H_
#define FACE_RECOGNIZER_INTERNAL_H_

#include "face_recognizer.h"

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <string>
#include <list>

#include "react/Domain.h"
#include "react/Event.h"
#include "react/Observer.h"
#include "json.h"

#include "inf_image.h"
#include "face_detector_internal.h"
#include "lru_cache.h"

using namespace react;

USING_REACTIVE_DOMAIN(EAI)

class FaceRecognizer {
    public:
	FaceRecognizer(MPI_WIN win, int scene_width, int scene_height, int snapshot_width, int snapshot_height,
	               InfModelCtx &&fr_model);
	~FaceRecognizer();

	void startServiceWith(const EventsT<OdEvent> &source, uint32_t sensitivity);
	void stopService();
	EventsT<OdEvent> &event()
	{
		return _event;
	}
	void setPriorityFactor(uint32_t once, uint32_t twice, uint32_t steady);
	std::tuple<uint32_t, uint32_t, uint32_t> getPriorityFactor();

    private:
	friend void EAI_FR_setConfThreshold(EaiFrContext *context, float threshold);
	using IdentityHistory = std::tuple<std::string, std::string, std::string>;
	using ConfidenceHistory = std::tuple<float, float, float>;
	static const MPI_RECT_POINT_S INVALID_REGION;
	static const IdentityHistory EMPTY_HISTORY;
	static const ConfidenceHistory EMPYT_CONF_HISTORY;
	static bool isValidRegion(const MPI_RECT_POINT_S &region);
	static float intersectionRatio(const MPI_RECT_POINT_S &test, const MPI_RECT_POINT_S &target);
	static int historySize(const IdentityHistory &hist);
	static IdentityHistory updateHistory(const IdentityHistory &hist, std::string identity);
	static ConfidenceHistory updateConfidences(const ConfidenceHistory &hist, float conf);
	static json_object *build_fr_attrs(const IdentityHistory &hist, const ConfidenceHistory &confs);

	template <typename F> void doInRegionSession(F &&action_in_session)
	{
		std::unique_lock<std::mutex> lock{ _region_mutex };
		action_in_session();
	}

	template <typename F> void doInIdentitySession(F &&action_in_session)
	{
		std::unique_lock<std::mutex> lock{ _identity_mutex };
		action_in_session();
	}

	template <typename F> void doInQueueSession(F &&action_in_session)
	{
		std::unique_lock<std::mutex> lock{ _queue_mutex };
		action_in_session();
		_queue_changed.notify_one();
	}

	int32_t faceBelongsTo(const MPI_RECT_POINT_S &face_rect);
	void runIdentifyService();

	const int _snapshot_width;
	const int _snapshot_height;
	const int _scene_width;
	const int _scene_height;
	const float _width_ratio;
	const float _height_ratio;
	const MPI_WIN _win;
	InfModelCtx _model_ctx;

	EventSourceT<OdEvent> _event{ MakeEventSource<EAI, OdEvent>() };
	Continuation<EAI> _upstream_event_cont{};
	EventsT<std::pair<int32_t, OdEvent> > _interested_faces{};

	std::mutex _region_mutex{};
	LRUCache<int32_t, MPI_RECT_POINT_S> _obj_region_cache{ 20, INVALID_REGION };

	std::mutex _identity_mutex{};
	LRUCache<int32_t, IdentityHistory> _identity_cache{ 20, EMPTY_HISTORY };
	LRUCache<int32_t, ConfidenceHistory> _identity_conf{ 20, EMPYT_CONF_HISTORY };
	LRUCache<int32_t, int64_t> _last_identify_time{ 20, -1 };

	std::mutex _queue_mutex{};
	std::condition_variable _queue_changed{};
	std::list<std::pair<int32_t, InfImage> > _face_queue{};

	std::thread _fd_task{};
	std::thread _fr_task{};
	std::atomic_bool _service_running{ false };
	std::atomic<uint32_t> _sensed_timestamp{ 0 };

	float _face_overlap_ratio{ 0.8 };

	uint32_t _priority_for_once{ 500 };
	uint32_t _priority_for_twice{ 1000 };
	uint32_t _priority_for_steady{ 2000 };
};

#endif /* FACE_RECOGNIZER_INTERNAL_H_ */
