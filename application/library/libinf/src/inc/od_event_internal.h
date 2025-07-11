#ifndef OD_EVENT_INTERNAL_H_
#define OD_EVENT_INTERNAL_H_

#include "od_event.h"

#include <functional>

#include "react/Domain.h"
#include "react/Event.h"
#include "react/Observer.h"

using namespace react;

REACTIVE_DOMAIN(EAI, sequential_concurrent)

class OdEventSource {
	USING_REACTIVE_DOMAIN(EAI)
    public:
	OdEventSource(MPI_WIN win)
	        : _win(win)
	        , _emitter(MakeEventSource<EAI, OdEvent>())
	{
	}

	void publish(MPI_IVA_OD_PARAM_S *param, std::function<bool()> unsubscribe);
	EventsT<OdEvent> &event()
	{
		return _emitter;
	}

    private:
	MPI_WIN _win;
	EventSourceT<OdEvent> _emitter;
};

#endif /* OD_EVENT_INTERNAL_H_ */
