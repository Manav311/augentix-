#ifdef LIBEAI
#include <iostream>

#include "od_event_internal.h"
#include "inf_log.h"
#include "mpi_dev.h"

USING_REACTIVE_DOMAIN(EAI)

class IvaOdGuard
{
public:
	IvaOdGuard(MPI_WIN win) : _win(win)
	{
		_err = MPI_IVA_enableObjDet(_win);
		if (_err != MPI_SUCCESS) {
			inf_log_err("Failed to enable OD. err: %d", _err);
		}
	}

	~IvaOdGuard()
	{
		if (enabled()) {
			if (MPI_SUCCESS != MPI_IVA_disableObjDet(_win)) {
				inf_log_err("Disable OD on win %u failed.", _win.win);
			}
		}
	}

	bool enabled() { return _err == MPI_SUCCESS; }
private:
	MPI_WIN _win;
	int _err;
};

void OdEventSource::publish(MPI_IVA_OD_PARAM_S *param, std::function<bool()> unsubscribe)
{
	int ret = MPI_IVA_setObjParam(_win, param);
	if (ret != MPI_SUCCESS) {
		inf_log_err("Failed to set OD param. err: %d", ret);
		return;
	}

	IvaOdGuard od_guard(_win);
	if (!od_guard.enabled()) {
		return;
	}

	const int timeout_ms = 1000;
	uint32_t timestamp;
	MPI_IVA_OBJ_LIST_S obj_list;
	memset(&obj_list, 0, sizeof(obj_list));
	inf_log_info("OD Event Source start publish.");
	json_object *source_name = json_object_new_string("EAI_OD");
	while (!unsubscribe()) {
		ret = MPI_DEV_waitWin(_win, &timestamp, timeout_ms);
		if (ret != MPI_SUCCESS) {
			inf_log_err("FAILED to waitWin for win: (%d, %d, %d) in %d ms. err: %d",
			            _win.dev, _win.chn, _win.win, timeout_ms, ret);
			continue;
		}

		ret = MPI_IVA_getBitStreamObjList(_win, timestamp, &obj_list);
		if (ret != MPI_SUCCESS) {
			inf_log_err("Failed to get object list. err: %d", ret);
			continue;
		}

		for (int i = 0; i < obj_list.obj_num; ++i) {
			MPI_IVA_OBJ_ATTR_S& od = obj_list.obj[i];
			OdEvent event {
				.timestamp = obj_list.timestamp,
				.id = od.id,
				.life = od.life,
				.rect = od.rect,
				.mv = od.mv,
				.snapshot = nullptr,
				.attrs = json_object_new_object()
			};
			json_object_object_add(event.attrs, "event-source", json_object_get(source_name));

			_emitter << event;
			json_object_put(event.attrs);
		}
	}
	json_object_put(source_name);
	inf_log_info("OD Event Source end publish.");
}

EaiOdContext *EAI_OD_create(MPI_WIN win)
{
	return reinterpret_cast<EaiOdContext *>(new OdEventSource(win));
}

void EAI_OD_dispose(EaiOdContext *context)
{
	if (context != nullptr) {
		delete reinterpret_cast<OdEventSource *>(context);
	}
}

void EAI_OD_subscribe(EaiOdEventPublisher *publisher, on_od_event handler, void *handler_context)
{
	auto *event_stream = reinterpret_cast<EventsT<OdEvent> *>(publisher);
	Observe(*event_stream, [=] (const OdEvent& event) {
		// std::cout << "OD subscriber thread: " << std::this_thread::get_id() << std::endl;
		handler(handler_context, &event);
	});
}

EaiOdEventPublisher *EAI_OD_getPublisher(EaiOdContext *context)
{
	auto *od_instance = reinterpret_cast<OdEventSource *>(context);
	return reinterpret_cast<EaiOdEventPublisher *>(&od_instance->event());
}

void EAI_OD_publish(EaiOdContext *context, MPI_IVA_OD_PARAM_S *param, publish_unsubscribe unsubscribe)
{
	auto *od_instance = reinterpret_cast<OdEventSource *>(context);
	od_instance->publish(param, [unsubscribe] { return unsubscribe(); });
}
#endif /* LIBEAI */
