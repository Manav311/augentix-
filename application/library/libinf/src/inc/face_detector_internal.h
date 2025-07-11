#ifndef FACE_DETECTOR_INTERNAL_H_
#define FACE_DETECTOR_INTERNAL_H_

#include "face_detector.h"

#include <functional>

#include "react/Domain.h"
#include "react/Event.h"
#include "react/Observer.h"

#include "json.h"

#include "inf_image.h"
#include "inf_model.h"
#include "od_event_internal.h"

using namespace react;

USING_REACTIVE_DOMAIN(EAI)

class FaceDetector {
    public:
	FaceDetector(MPI_WIN win, int scene_width, int scene_height, int snapshot_width, int snapshot_height,
	             InfModelCtx &&detector);
	~FaceDetector();

	void publish(uint32_t sensitivity, bool with_snapshot, std::function<bool()> unsubscribe);
	EventsT<OdEvent> &event()
	{
		return _emitter;
	}
	void detachModel()
	{
		_model_ctx.model = nullptr;
	}

    private:
	bool is_model_ready() const
	{
		return _model_ctx.model != nullptr;
	}
	void rescaleBox(MPI_RECT_POINT_S &rect);

	const int _snapshot_width;
	const int _snapshot_height;
	const float _width_ratio;
	const float _height_ratio;
	const MPI_WIN _win;
	InfModelCtx _model_ctx;

	EventSourceT<OdEvent> _emitter{ MakeEventSource<EAI, OdEvent>() };
};

#endif /* FACE_DETECTOR_INTERNAL_H_ */
