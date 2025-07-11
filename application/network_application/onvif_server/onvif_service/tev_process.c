#include "stdio.h"
#include "stdlib.h"
#include "augentix.h"

#define MAX_SUBSCRIPTION 3
#define ONVIF_EVENT_PATH "/tmp/event_onvif"

typedef struct event_subscription_ {
	int id;
	int in_use;
	int sync;
	int notify_en;
	time_t timeout;
	char endpoint[MAX_STR_LEN];
} event_subscription;

typedef struct event_control_ {
	int md_alarm;
	int td_alarm;
	event_subscription event_sub[MAX_SUBSCRIPTION];
} event_control;

event_control g_event_ctrl = { 0 };
pthread_mutex_t g_event_mutex = PTHREAD_MUTEX_INITIALIZER;

static event_subscription *tev_get_available_subcription()
{
	int i = 0;
	event_subscription *event_sub = NULL;
	event_control *ev_ctrl = &g_event_ctrl;

	for (i = 0; i < MAX_SUBSCRIPTION; i++) {
		event_sub = &ev_ctrl->event_sub[i];
		if (!event_sub->in_use) {
			break;
		}
	}

	if (i >= MAX_SUBSCRIPTION) {
		ONVIF_TRACE("No more resource for subcription\n");
		return NULL;
	}

	event_sub->id = i;

	return event_sub;
}

static event_subscription *tev_check_subcription(char *name)
{
	int i = 0;
	char *ptr = NULL;
	event_subscription *event_sub = NULL;
	event_control *ev_ctrl = &g_event_ctrl;

	ptr = strstr(name, "/onvif/Events/");
	if (!ptr) {
		return NULL;
	}

	i = atoi(ptr + 14) - 1;

	if (i >= MAX_SUBSCRIPTION) {
		ONVIF_TRACE("Can't find subcription %s\n", name);
		return NULL;
	}

	event_sub = &ev_ctrl->event_sub[i];

	if (!event_sub->in_use) {
		ONVIF_TRACE("Resource not available %d\n", i);
		return NULL;
	}

	return event_sub;
}

/** Web service operation '__tev__PullMessages' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__PullMessages(struct soap *soap, struct _tev__PullMessages *tev__PullMessages,
                                              struct _tev__PullMessagesResponse *tev__PullMessagesResponse)
{
	int md_alarm = AGTX_SW_EVENT_TRIG_TYPE_IVA_MD_NEGATIVE;
	int duration = 60;
	int cnt = 0;
	event_control *ev_ctrl = &g_event_ctrl;
	struct wsnt__NotificationMessageHolderType *msg = NULL;
	struct _tt__Message *tmsg = NULL;
	event_subscription *event_sub = NULL;

	//	ONVIF_TRACE("__tev__PullMessages\n");
	/*Collect alarm in 1 sec and Prevent onvif device manager request too quickly*/
	while (cnt <= 10) {
		if (ev_ctrl->md_alarm == AGTX_SW_EVENT_TRIG_TYPE_IVA_MD_POSITIVE) {
			md_alarm = 1;
			break;
		}
		usleep(100000);
		cnt++;
	}

	if (!soap->header) {
		soap_header(soap);
	}

	if (soap->header->wsa5__Action) {
		soap_dealloc(soap, (void *)soap->header->wsa5__Action);
	}

	soap->header->wsa5__Action = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(soap->header->wsa5__Action,
	       "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesResponse");

	pthread_mutex_lock(&g_event_mutex);
	event_sub = tev_check_subcription(soap->header->wsa5__To);

	if (!event_sub) {
		pthread_mutex_unlock(&g_event_mutex);
		return soap_sender_fault(soap, "Unknown Resource", NULL);
	}

	tev__PullMessagesResponse->CurrentTime = time(0);
	tev__PullMessagesResponse->TerminationTime = tev__PullMessagesResponse->CurrentTime + duration;
	pthread_mutex_unlock(&g_event_mutex);

	tev__PullMessagesResponse->__sizeNotificationMessage = 1;
	tev__PullMessagesResponse->wsnt__NotificationMessage =
	        aux_onvif_malloc(soap, sizeof(struct wsnt__NotificationMessageHolderType));
	msg = tev__PullMessagesResponse->wsnt__NotificationMessage;
	msg->Topic = aux_onvif_malloc(soap, sizeof(struct wsnt__TopicExpressionType));
	msg->Topic->Dialect = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(msg->Topic->Dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
	msg->Topic->__any = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(msg->Topic->__any, "tns1:RuleEngine/CellMotionDetector/Motion");
	msg->Message.tt__Message = aux_onvif_malloc(soap, sizeof(struct _tt__Message));
	tmsg = msg->Message.tt__Message;
	tmsg->UtcTime = time(0);
	tmsg->PropertyOperation = aux_onvif_malloc(soap, sizeof(enum tt__PropertyOperation));
	*tmsg->PropertyOperation =
	        event_sub->sync ? tt__PropertyOperation__Initialized : tt__PropertyOperation__Changed;
	tmsg->Source = aux_onvif_malloc(soap, sizeof(struct tt__ItemList));
	tmsg->Source->__sizeSimpleItem = 2;
	tmsg->Source->SimpleItem_ =
	        aux_onvif_malloc(soap, sizeof(struct _tt__ItemList_SimpleItem) * tmsg->Source->__sizeSimpleItem);
	tmsg->Source->SimpleItem_[0].Name = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Source->SimpleItem_[0].Name, "VideoSourceConfigurationToken");
	tmsg->Source->SimpleItem_[0].Value = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Source->SimpleItem_[0].Value, "000");
	tmsg->Source->SimpleItem_[1].Name = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Source->SimpleItem_[1].Name, "Rule");
	tmsg->Source->SimpleItem_[1].Value = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Source->SimpleItem_[1].Value, "MyMotionDetectorRule");

	tmsg->Data = aux_onvif_malloc(soap, sizeof(struct tt__ItemList));
	tmsg->Data->__sizeSimpleItem = 1;
	tmsg->Data->SimpleItem_ =
	        aux_onvif_malloc(soap, sizeof(struct _tt__ItemList_SimpleItem) * tmsg->Data->__sizeSimpleItem);
	tmsg->Data->SimpleItem_[0].Name = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Data->SimpleItem_[0].Name, "IsMotion");
	tmsg->Data->SimpleItem_[0].Value = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Data->SimpleItem_[0].Value, md_alarm ? "true" : "false");

	event_sub->sync = 0;

	pthread_mutex_unlock(&g_event_mutex);

	return SOAP_OK;
};
/** Web service operation '__tev__Seek' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__Seek(struct soap *soap, struct _tev__Seek *tev__Seek,
                                      struct _tev__SeekResponse *tev__SeekResponse)
{
	ONVIF_TRACE("__tev__Seek\n");

	/*Unsupported this feature,we don't support past event.*/

	return SOAP_FAULT;
};
/** Web service operation '__tev__SetSynchronizationPoint' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tev__SetSynchronizationPoint(struct soap *soap, struct _tev__SetSynchronizationPoint *tev__SetSynchronizationPoint,
                               struct _tev__SetSynchronizationPointResponse *tev__SetSynchronizationPointResponse)
{
	event_subscription *event_sub = NULL;

	ONVIF_TRACE("__tev__SetSynchronizationPoint\n");

	if (!soap->header) {
		soap_header(soap);
	}

	if (soap->header->wsa5__Action) {
		soap_dealloc(soap, (void *)soap->header->wsa5__Action);
	}

	soap->header->wsa5__Action = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(soap->header->wsa5__Action,
	       "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/SetSynchronizationPointResponse");

	event_sub = tev_check_subcription(soap->header->wsa5__To);

	if (!event_sub) {
		return soap_sender_fault(soap, "Unknown Resource", NULL);
	}

	event_sub->sync++;

	return SOAP_OK;
};
/** Web service operation '__tev__GetServiceCapabilities' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tev__GetServiceCapabilities(struct soap *soap, struct _tev__GetServiceCapabilities *tev__GetServiceCapabilities,
                              struct _tev__GetServiceCapabilitiesResponse *tev__GetServiceCapabilitiesResponse)
{
	struct tev__Capabilities *cap = NULL;

	ONVIF_TRACE("__tev__GetServiceCapabilities\n");

	tev__GetServiceCapabilitiesResponse->Capabilities = aux_onvif_malloc(soap, sizeof(struct tev__Capabilities));
	cap = tev__GetServiceCapabilitiesResponse->Capabilities;
	cap->MaxPullPoints = aux_onvif_malloc(soap, sizeof(int));
	*cap->MaxPullPoints = MAX_SUBSCRIPTION;
	cap->PersistentNotificationStorage = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
	*cap->PersistentNotificationStorage = xsd__boolean__false_;
	cap->WSPullPointSupport = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
	*cap->WSPullPointSupport = xsd__boolean__true_;
	cap->WSSubscriptionPolicySupport = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
	*cap->WSSubscriptionPolicySupport = xsd__boolean__true_;

	return SOAP_OK;
};
/** Web service operation '__tev__CreatePullPointSubscription' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__CreatePullPointSubscription(
        struct soap *soap, struct _tev__CreatePullPointSubscription *tev__CreatePullPointSubscription,
        struct _tev__CreatePullPointSubscriptionResponse *tev__CreatePullPointSubscriptionResponse)
{
	int duration = 0;
	char ip_buf[MAX_STR_LEN] = { 0 };
	event_subscription *event_sub = NULL;
	struct wsa5__EndpointReferenceType *end_point = NULL;

	ONVIF_TRACE("__tev__CreatePullPointSubscription\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (!soap->header) {
		soap_header(soap);
	}

	if (soap->header->wsa5__Action) {
		soap_dealloc(soap, (void *)soap->header->wsa5__Action);
	}

	soap->header->wsa5__Action = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(soap->header->wsa5__Action,
	       "http://www.onvif.org/ver10/events/wsdl/EventPortType/CreatePullPointSubscriptionResponse");

	pthread_mutex_lock(&g_event_mutex);
	event_sub = tev_get_available_subcription();

	if (!event_sub) {
		pthread_mutex_unlock(&g_event_mutex);
		return soap_sender_fault(soap, "No resource", NULL);
	}

	if (tev__CreatePullPointSubscription->InitialTerminationTime) {
		duration = aux_parse_iso8061_duration(tev__CreatePullPointSubscription->InitialTerminationTime);
	} else {
		ONVIF_TRACE("No duration found\n");
		duration = 10;
	}

	tev__CreatePullPointSubscriptionResponse->wsnt__CurrentTime = time(0);
	tev__CreatePullPointSubscriptionResponse->wsnt__TerminationTime =
	        tev__CreatePullPointSubscriptionResponse->wsnt__CurrentTime + duration;

	end_point = &tev__CreatePullPointSubscriptionResponse->SubscriptionReference;
	end_point->Address = aux_onvif_malloc(soap, MAX_STR_LEN);

	sprintf(end_point->Address, "http://%s:8899/onvif/Events/%d", SYS_Getipaddr("eth0", ip_buf), event_sub->id + 1);
	event_sub->timeout = tev__CreatePullPointSubscriptionResponse->wsnt__TerminationTime;
	event_sub->in_use = 1;
	event_sub->notify_en = 0;
	event_sub->sync = 0;
	pthread_mutex_unlock(&g_event_mutex);

	return SOAP_OK;
};
/** Web service operation '__tev__GetEventProperties' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tev__GetEventProperties(struct soap *soap, struct _tev__GetEventProperties *tev__GetEventProperties,
                          struct _tev__GetEventPropertiesResponse *tev__GetEventPropertiesResponse)
{
	ONVIF_TRACE("__tev__GetEventProperties\n");

	if (!soap->header) {
		soap_header(soap);
	}

	if (soap->header->wsa5__Action) {
		soap_dealloc(soap, (void *)soap->header->wsa5__Action);
	}

	soap->header->wsa5__Action = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(soap->header->wsa5__Action,
	       "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesResponse");

	tev__GetEventPropertiesResponse->__sizeTopicNamespaceLocation = 1;
	tev__GetEventPropertiesResponse->TopicNamespaceLocation_ = aux_onvif_malloc(soap, sizeof(char *));
	*tev__GetEventPropertiesResponse->TopicNamespaceLocation_ = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(*tev__GetEventPropertiesResponse->TopicNamespaceLocation_,
	       "http://www.onvif.org/onvif/ver10/topics/topicns.xml");

	tev__GetEventPropertiesResponse->__sizeTopicExpressionDialect = 2;
	tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect = aux_onvif_malloc(soap, sizeof(char *) * 2);
	tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect[0] =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect[0],
	       "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
	tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect[1] =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect[1],
	       "http://docs.oasis-open.org/wsn/t-1/TopicExpression/Concrete");

	tev__GetEventPropertiesResponse->__sizeMessageContentFilterDialect = 1;
	tev__GetEventPropertiesResponse->MessageContentFilterDialect_ = aux_onvif_malloc(soap, sizeof(char *));
	*tev__GetEventPropertiesResponse->MessageContentFilterDialect_ = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(*tev__GetEventPropertiesResponse->MessageContentFilterDialect_,
	       "http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter");

	tev__GetEventPropertiesResponse->__sizeMessageContentSchemaLocation = 1;
	tev__GetEventPropertiesResponse->MessageContentSchemaLocation_ = aux_onvif_malloc(soap, sizeof(char *));
	*tev__GetEventPropertiesResponse->MessageContentSchemaLocation_ = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(*tev__GetEventPropertiesResponse->MessageContentSchemaLocation_,
	       "http://www.onvif.org/ver10/schema/onvif.xsd");

	tev__GetEventPropertiesResponse->wsnt__FixedTopicSet = xsd__boolean__true_;
	tev__GetEventPropertiesResponse->wstop__TopicSet = aux_onvif_malloc(soap, sizeof(struct wstop__TopicSetType));
	tev__GetEventPropertiesResponse->wstop__TopicSet->__mixed = aux_onvif_malloc(soap, MAX_STR_LEN * 4);
	strcpy(tev__GetEventPropertiesResponse->wstop__TopicSet->__mixed,
	       "<tns1:RuleEngine>"
	       "<CellMotionDetector><Motion wstop:topic=\"true\">"
	       "<tt:MessageDescription IsProperty=\"true\">"
	       "<tt:Source>"
	       "<tt:SimpleItemDescription Name=\"VideoSourceConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
	       "<tt:SimpleItemDescription Name=\"VideoAnalyticsConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
	       "<tt:SimpleItemDescription Name=\"Rule\" Type=\"xs:string\"/>"
	       "</tt:Source>"
	       "<tt:Data>"
	       "<tt:SimpleItemDescription Name=\"IsMotion\" Type=\"xs:boolean\"/>"
	       "</tt:Data>"
	       "</tt:MessageDescription>"
	       "</Motion>"
	       "</CellMotionDetector>"
	       "</tns1:RuleEngine>");

	return SOAP_OK;
};
/** Web service operation '__tev__Renew' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__Renew(struct soap *soap, struct _wsnt__Renew *wsnt__Renew,
                                       struct _wsnt__RenewResponse *wsnt__RenewResponse)
{
	int duration = 0;
	event_subscription *event_sub = NULL;

	//	ONVIF_TRACE("__tev__Renew\n");

	if (!soap->header) {
		soap_header(soap);
	}

	if (soap->header->wsa5__Action) {
		soap_dealloc(soap, (void *)soap->header->wsa5__Action);
	}

	soap->header->wsa5__Action = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(soap->header->wsa5__Action, "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/RenewResponse");

	pthread_mutex_lock(&g_event_mutex);
	event_sub = tev_check_subcription(soap->header->wsa5__To);

	if (!event_sub) {
		pthread_mutex_unlock(&g_event_mutex);
		return soap_sender_fault(soap, "Unknown Resource", NULL);
	}

	if (wsnt__Renew->TerminationTime) {
		duration = aux_parse_iso8061_duration(wsnt__Renew->TerminationTime);
	} else {
		ONVIF_TRACE("No duration found\n");
		duration = 10;
	}

	wsnt__RenewResponse->CurrentTime = aux_onvif_malloc(soap, sizeof(time_t));
	*wsnt__RenewResponse->CurrentTime = time(0);
	wsnt__RenewResponse->TerminationTime = *wsnt__RenewResponse->CurrentTime + duration;
	event_sub->timeout = wsnt__RenewResponse->TerminationTime;
	pthread_mutex_unlock(&g_event_mutex);

	return SOAP_OK;
};
/** Web service operation '__tev__Unsubscribe' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe(struct soap *soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe,
                                             struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse)
{
	event_subscription *event_sub = NULL;

	ONVIF_TRACE("__tev__Unsubscribe %s %s\n", soap->header->wsa5__To, soap->header->wsa5__Action);

	if (!soap->header) {
		soap_header(soap);
	}

	if (soap->header->wsa5__Action) {
		soap_dealloc(soap, (void *)soap->header->wsa5__Action);
	}

	soap->header->wsa5__Action = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(soap->header->wsa5__Action,
	       "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/UnsubscribeResponse");

	pthread_mutex_lock(&g_event_mutex);
	event_sub = tev_check_subcription(soap->header->wsa5__To);

	if (!event_sub) {
		pthread_mutex_unlock(&g_event_mutex);
		return soap_sender_fault(soap, "No resource", NULL);
	}

	event_sub->in_use = 0;

	pthread_mutex_unlock(&g_event_mutex);

	return SOAP_OK;
};
/** Web service operation '__tev__Subscribe' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__Subscribe(struct soap *soap, struct _wsnt__Subscribe *wsnt__Subscribe,
                                           struct _wsnt__SubscribeResponse *wsnt__SubscribeResponse)
{
	int duration = 0;
	char ip_buf[MAX_STR_LEN] = { 0 };
	event_subscription *event_sub = NULL;
	struct wsa5__EndpointReferenceType *end_point = NULL;

	ONVIF_TRACE("__tev__Subscribe\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	pthread_mutex_lock(&g_event_mutex);
	event_sub = tev_get_available_subcription();

	if (!event_sub) {
		pthread_mutex_unlock(&g_event_mutex);
		return soap_sender_fault(soap, "No resource", NULL);
	}

	if (wsnt__Subscribe->InitialTerminationTime) {
		duration = aux_parse_iso8061_duration(wsnt__Subscribe->InitialTerminationTime);
	} else {
		ONVIF_TRACE("No duration found\n");
		duration = 10;
	}

	/*TODO filter*/

	wsnt__SubscribeResponse->CurrentTime = aux_onvif_malloc(soap, sizeof(time_t));
	*wsnt__SubscribeResponse->CurrentTime = time(0);
	wsnt__SubscribeResponse->TerminationTime = aux_onvif_malloc(soap, sizeof(time_t));
	*wsnt__SubscribeResponse->TerminationTime = *wsnt__SubscribeResponse->CurrentTime + duration;
	end_point = &wsnt__SubscribeResponse->SubscriptionReference;
	end_point->Address = aux_onvif_malloc(soap, MAX_STR_LEN);
	sprintf(end_point->Address, "http://%s:8899/onvif/Events/%d", SYS_Getipaddr("eth0", ip_buf), event_sub->id + 1);

	event_sub->timeout = *wsnt__SubscribeResponse->TerminationTime;
	strcpy(event_sub->endpoint, wsnt__Subscribe->ConsumerReference.Address);
	event_sub->in_use = 1;
	event_sub->notify_en = 1;
	event_sub->sync = 0;
	pthread_mutex_unlock(&g_event_mutex);

	return SOAP_OK;
};
/** Web service operation '__tev__GetCurrentMessage' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tev__GetCurrentMessage(struct soap *soap, struct _wsnt__GetCurrentMessage *wsnt__GetCurrentMessage,
                         struct _wsnt__GetCurrentMessageResponse *wsnt__GetCurrentMessageResponse)
{
	ONVIF_TRACE("__tev__GetCurrentMessage TODO\n");
	return SOAP_OK;
};
/** Web service operation '__tev__Notify' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify(struct soap *soap, struct _wsnt__Notify *wsnt__Notify)
{
	int msg_num = 1;
	int i = 0;
	int id = 0;
	event_control *ev_ctrl = &g_event_ctrl;
	event_subscription *event_sub = NULL;
	struct _tt__Message *tmsg = NULL;
	struct wsnt__TopicExpressionType *topic = NULL;
	struct _wsnt__NotificationMessageHolderType_Message *msg = NULL;

	//	ONVIF_TRACE("__tev__Notify\n");

	id = wsnt__Notify->__sizeNotificationMessage;
	event_sub = &ev_ctrl->event_sub[id];

	wsnt__Notify->__sizeNotificationMessage = msg_num;
	wsnt__Notify->NotificationMessage_ =
	        aux_onvif_malloc(soap, sizeof(struct wsnt__NotificationMessageHolderType) * msg_num);

	wsnt__Notify->NotificationMessage_[i].Topic = aux_onvif_malloc(soap, sizeof(struct wsnt__TopicExpressionType));
	topic = wsnt__Notify->NotificationMessage_[i].Topic;
	topic->Dialect = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(topic->Dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
	topic->__any = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(topic->__any, "tns1:RuleEngine/CellMotionDetector/Motion");

	msg = &wsnt__Notify->NotificationMessage_[i].Message;

	msg->tt__Message = aux_onvif_malloc(soap, sizeof(struct _tt__Message));
	tmsg = msg->tt__Message;
	tmsg->UtcTime = time(0);
	tmsg->PropertyOperation = aux_onvif_malloc(soap, sizeof(enum tt__PropertyOperation));
	*tmsg->PropertyOperation =
	        event_sub->sync ? tt__PropertyOperation__Initialized : tt__PropertyOperation__Changed;
	tmsg->Source = aux_onvif_malloc(soap, sizeof(struct tt__ItemList));
	tmsg->Source->__sizeSimpleItem = 3;
	tmsg->Source->SimpleItem_ =
	        aux_onvif_malloc(soap, sizeof(struct _tt__ItemList_SimpleItem) * tmsg->Source->__sizeSimpleItem);
	tmsg->Source->SimpleItem_[0].Name = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Source->SimpleItem_[0].Name, "VideoSourceConfigurationToken");
	tmsg->Source->SimpleItem_[0].Value = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Source->SimpleItem_[0].Value, "000");
	tmsg->Source->SimpleItem_[1].Name = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Source->SimpleItem_[1].Name, "VideoAnalyticsConfigurationToken");
	tmsg->Source->SimpleItem_[1].Value = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Source->SimpleItem_[1].Value, "VideoAnalyticsToken");
	tmsg->Source->SimpleItem_[2].Name = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Source->SimpleItem_[2].Name, "Rule");
	tmsg->Source->SimpleItem_[2].Value = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Source->SimpleItem_[2].Value, "MyMotionDetectorRule");

	tmsg->Data = aux_onvif_malloc(soap, sizeof(struct tt__ItemList));
	tmsg->Data->__sizeSimpleItem = 1;
	tmsg->Data->SimpleItem_ =
	        aux_onvif_malloc(soap, sizeof(struct _tt__ItemList_SimpleItem) * tmsg->Data->__sizeSimpleItem);
	tmsg->Data->SimpleItem_[0].Name = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Data->SimpleItem_[0].Name, "IsMotion");
	tmsg->Data->SimpleItem_[0].Value = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(tmsg->Data->SimpleItem_[0].Value,
	       ev_ctrl->md_alarm == AGTX_SW_EVENT_TRIG_TYPE_IVA_MD_POSITIVE ? "true" : "false");
	/*clear sync point*/
	event_sub->sync = 0;

	return SOAP_OK;
}

/** Web service operation '__tev__GetMessages' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__GetMessages(struct soap *soap, struct _wsnt__GetMessages *wsnt__GetMessages,
                                             struct _wsnt__GetMessagesResponse *wsnt__GetMessagesResponse)
{
	ONVIF_TRACE("__tev__GetMessages TODO\n");
	return SOAP_OK;
};
/** Web service operation '__tev__DestroyPullPoint' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__DestroyPullPoint(struct soap *soap,
                                                  struct _wsnt__DestroyPullPoint *wsnt__DestroyPullPoint,
                                                  struct _wsnt__DestroyPullPointResponse *wsnt__DestroyPullPointResponse)
{
	ONVIF_TRACE("__tev__DestroyPullPoint TODO\n");
	return SOAP_OK;
};
/** Web service operation '__tev__Notify_' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify_(struct soap *soap, struct _wsnt__Notify *wsnt__Notify)
{
	ONVIF_TRACE("__tev__Notify_\n");

	return __tev__Notify(soap, wsnt__Notify);
};
/** Web service operation '__tev__CreatePullPoint' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__CreatePullPoint(struct soap *soap,
                                                 struct _wsnt__CreatePullPoint *wsnt__CreatePullPoint,
                                                 struct _wsnt__CreatePullPointResponse *wsnt__CreatePullPointResponse)
{
	ONVIF_TRACE("__tev__CreatePullPoint TODO\n");
	return SOAP_OK;
};
/** Web service operation '__tev__Renew_' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__Renew_(struct soap *soap, struct _wsnt__Renew *wsnt__Renew,
                                        struct _wsnt__RenewResponse *wsnt__RenewResponse)
{
	ONVIF_TRACE("__tev__Renew_\n");
	return __tev__Renew(soap, wsnt__Renew, wsnt__RenewResponse);
};
/** Web service operation '__tev__Unsubscribe_' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe_(struct soap *soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe,
                                              struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse)
{
	ONVIF_TRACE("__tev__Unsubscribe_\n");
	return __tev__Unsubscribe(soap, wsnt__Unsubscribe, wsnt__UnsubscribeResponse);
};
/** Web service operation '__tev__PauseSubscription' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tev__PauseSubscription(struct soap *soap, struct _wsnt__PauseSubscription *wsnt__PauseSubscription,
                         struct _wsnt__PauseSubscriptionResponse *wsnt__PauseSubscriptionResponse)
{
	ONVIF_TRACE("__tev__PauseSubscription\n");
	/*Unsupported this feature,we don't support.*/
	return SOAP_FAULT;
};
/** Web service operation '__tev__ResumeSubscription' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tev__ResumeSubscription(struct soap *soap, struct _wsnt__ResumeSubscription *wsnt__ResumeSubscription,
                          struct _wsnt__ResumeSubscriptionResponse *wsnt__ResumeSubscriptionResponse)
{
	ONVIF_TRACE("__tev__ResumeSubscription\n");
	/*Unsupported this feature,we don't support.*/
	return SOAP_FAULT;
};

int tev_send_event(char *soap_endpoint, int id)
{
	char buf[MAX_STR_LEN] = { 0 };
	struct soap soap_data = { 0 };
	struct soap *soap = &soap_data;
	struct _wsnt__Notify wsnt__Notify = { 0 };
	struct __tev__Notify soap_tmp___tev__Notify = { 0 };

	/*if no string gsoap will stuck*/
	if (!strlen(soap_endpoint)) {
		ONVIF_TRACE("no soap_endpoint\n");
		return -1;
	}

	//	ONVIF_TRACE("onvif_send_event %s\n",soap_endpoint);
	soap_tmp___tev__Notify.wsnt__Notify = &wsnt__Notify;
	/*passing id to __tev__Notify*/
	soap_tmp___tev__Notify.wsnt__Notify->__sizeNotificationMessage = id;

	soap_init(soap);
	soap->connect_timeout = 1;
	soap->send_timeout = 1;
	soap->recv_timeout = 1;

	soap_begin(soap);
	soap_serializeheader(soap);

	/*soap header*/
	soap->header = aux_onvif_malloc(soap, sizeof(struct SOAP_ENV__Header));
	soap->header->wsa5__To = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(soap->header->wsa5__To, soap_endpoint);
	soap->header->wsa5__Action = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(soap->header->wsa5__Action, "http://docs.oasis-open.org/wsn/bw-2/NotificationConsumer/Notify");

	/*soap post header or body*/
	if (soap_POST(soap, soap_endpoint, NULL, "application/soap+xml; charset=utf-8") ||
	    soap_envelope_begin_out(soap) || soap_putheader(soap) || soap_body_begin_out(soap) ||
	    __tev__Notify(soap, &wsnt__Notify) ||
	    soap_put___tev__Notify(soap, &soap_tmp___tev__Notify, "-tev:Notify", "") || soap_body_end_out(soap) ||
	    soap_envelope_end_out(soap) || soap_end_send(soap) || soap_recv_empty_response(soap)) {
		soap_sprint_fault(soap, buf, MAX_STR_LEN);
		ONVIF_TRACE("endpoint %s / %s\n", soap_endpoint, buf);
	}

	/*close socket*/
	soap_closesock(soap);

	/*free resource*/
	soap_destroy((struct soap *)soap); // dealloc C++ data
	soap_end((struct soap *)soap); // dealloc data and clean up
	soap_done((struct soap *)soap); // detach soap struct

	return 0;
}

static void tev_clean_old_Fd(const char *fileName)
{
	if (!access(fileName, F_OK)) {
		fprintf(stderr, "Deleting old unix File descriptor \n");

		if (!remove(fileName)) {
			char buf[64];

			memset(buf, 0, 64);

			sprintf(buf, "rm -f %s", fileName);

			system(buf); //force delete
		}
	}
}

void tev_event_thread(void *data)
{
	int i = 0;
	int max_fd = 0;
	int sockfd = -1;
	int clientfd = -1;
	int servlen = 0;
	int ret = 0;
	int err_cnt = 0;
	int sin_size = 0;
	int alarm = 0;
	int in_use = 0;
	int notify_en = 0;
	fd_set read_fds;
	struct timeval tv = { 0 };
	struct sockaddr_un serv_addr = { 0 };
	event_control *ev_ctrl = &g_event_ctrl;
	event_subscription *event_sub = NULL;

	/* clean up any Orphen socket before start server */
	tev_clean_old_Fd(ONVIF_EVENT_PATH);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		ONVIF_TRACE("Create onvif event socket error \n");
		return;
	}

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, ONVIF_EVENT_PATH);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	//binding
	if (bind(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
		ONVIF_TRACE("onvif event bind failed\n");
		return;
	}

	//Start listening
	if (listen(sockfd, 10) == -1) {
		ONVIF_TRACE("onvif event listen failed\n");
		return;
	}

	sin_size = sizeof(struct sockaddr_in);

	while (1) {
		tv.tv_sec = 0;
		tv.tv_usec = 250000;

		FD_ZERO(&read_fds);
		FD_SET(sockfd, &read_fds);
		max_fd = sockfd;

		if (clientfd > 0) {
			FD_SET(clientfd, &read_fds);
		}

		if (clientfd > max_fd) {
			max_fd = clientfd;
		}

		ret = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
		if (ret < 0) {
			printf("select error\n");
			continue;
		} else if (ret == 0) {
			//			printf( "select timeout\n" );
			pthread_mutex_lock(&g_event_mutex);
			for (i = 0; i < MAX_SUBSCRIPTION; i++) {
				event_sub = &ev_ctrl->event_sub[i];
				if (event_sub->in_use && (event_sub->timeout < time(0))) {
					ONVIF_TRACE("Remove subsription %d time %d / %d\n", i, (int)event_sub->timeout,
					            (int)time(0));
					event_sub->in_use = 0;
				}
			}
			pthread_mutex_unlock(&g_event_mutex);

			for (i = 0; i < MAX_SUBSCRIPTION; i++) {
				event_sub = &ev_ctrl->event_sub[i];
				pthread_mutex_lock(&g_event_mutex);
				in_use = event_sub->in_use;
				notify_en = event_sub->notify_en;
				pthread_mutex_unlock(&g_event_mutex);
				if (in_use && notify_en && event_sub->sync) {
					tev_send_event(event_sub->endpoint, i);
				}
			}

			continue;
		} else {
			if (FD_ISSET(sockfd, &read_fds)) {
				if ((clientfd = accept(sockfd, (struct sockaddr *)&serv_addr, (socklen_t *)&sin_size)) <
				    0) {
					ONVIF_TRACE("Accept Error ..\n");
					continue;
				}
			} else if (FD_ISSET(clientfd, &read_fds)) {
				if (err_cnt > 5) {
					ONVIF_TRACE("Too many error close socket and leave.\n");
					close(clientfd);
					clientfd = -1;
					err_cnt = 0;
					continue;
				}

				ret = read(clientfd, &alarm, sizeof(alarm));
				if (ret < 0) {
					ONVIF_TRACE("Read failed %d(%m),leave thread.\n", errno);
					close(clientfd);
					clientfd = -1;
					continue;
				} else if (ret != sizeof(alarm)) {
					ONVIF_TRACE("Read too short %d(%m)\n", errno);
					err_cnt++;
					continue;
				}

				if (alarm == AGTX_SW_EVENT_TRIG_TYPE_IVA_MD_POSITIVE ||
				    alarm == AGTX_SW_EVENT_TRIG_TYPE_IVA_MD_NEGATIVE) {
					ev_ctrl->md_alarm = alarm;
				} else if (alarm == AGTX_SW_EVENT_TRIG_TYPE_IVA_TD_POSITIVE ||
				           alarm == AGTX_SW_EVENT_TRIG_TYPE_IVA_TD_NEGATIVE) {
					ev_ctrl->td_alarm = alarm;
				}

				//				ONVIF_TRACE("sid %d\n", alarm);
				err_cnt = 0;
				for (i = 0; i < MAX_SUBSCRIPTION; i++) {
					event_sub = &ev_ctrl->event_sub[i];
					pthread_mutex_lock(&g_event_mutex);
					in_use = event_sub->in_use;
					notify_en = event_sub->notify_en;
					pthread_mutex_unlock(&g_event_mutex);
					if (event_sub->in_use && event_sub->notify_en) {
						tev_send_event(event_sub->endpoint, i);
					}
				}
				//				close(clientfd);
				//				clientfd = -1;
			}
		}
	}

	close(sockfd);
}

int tev_process_init()
{
	pthread_t pid;
    char name[16] = "event_thread";
    char tmpname[16] = {0};
    int rc = 0;

	if (pthread_create(&pid, NULL, (void *(*)(void *))tev_event_thread, NULL)) {
		ONVIF_TRACE("event_thread init fail\n");
		return -1;
	}
    rc = pthread_setname_np (pid, name);
    sleep(1);
    if ( rc == 0 ) {
        if ( pthread_getname_np(pid, tmpname,sizeof(tmpname)) == 0) {
            printf("%s thread create [Done]",tmpname);
        } else {
            printf("%s thread create [Fail]",name);
        }
    }
    rc = pthread_join(pid,NULL);
    if (rc != 0) {
        printf("Error Occured in %s \n",__func__);
    }
	return 0;
}
