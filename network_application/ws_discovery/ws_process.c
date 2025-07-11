#include "stdio.h"
#include "stdlib.h"
#include "soapH.h"
#include "soapStub.h"
#include "stdsoap2.h"

#include "augentix.h"
#include "agtx_types.h"

SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__ProbeMatches(struct soap *soap, const char *soap_endpoint,
                                                         const char *soap_action,
                                                         struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{
	struct __wsdd__ProbeMatches soap_tmp___wsdd__ProbeMatches;
	if (soap_action == NULL)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ProbeMatches";
	soap_tmp___wsdd__ProbeMatches.wsdd__ProbeMatches = wsdd__ProbeMatches;
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_serializeheader(soap);
	soap_serialize___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH) {
		if (soap_envelope_begin_out(soap) || soap_putheader(soap) || soap_body_begin_out(soap) ||
		    soap_put___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches, "-wsdd:ProbeMatches", "") ||
		    soap_body_end_out(soap) || soap_envelope_end_out(soap))
			return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action) || soap_envelope_begin_out(soap) || soap_putheader(soap) ||
	    soap_body_begin_out(soap) ||
	    soap_put___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches, "-wsdd:ProbeMatches", "") ||
	    soap_body_end_out(soap) || soap_envelope_end_out(soap) || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

/** Web service operation 'SOAP_ENV__Fault' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap *soap, char *faultcode, char *faultstring, char *faultactor,
                                          struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code,
                                          struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node,
                                          char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
	AGTX_UNUSED(soap);
	AGTX_UNUSED(faultcode);
	AGTX_UNUSED(faultstring);
	AGTX_UNUSED(faultactor);
	AGTX_UNUSED(detail);
	AGTX_UNUSED(SOAP_ENV__Code);
	AGTX_UNUSED(SOAP_ENV__Reason);
	AGTX_UNUSED(SOAP_ENV__Node);
	AGTX_UNUSED(SOAP_ENV__Role);
	AGTX_UNUSED(SOAP_ENV__Detail);

	return SOAP_OK;
}
/** Web service operation '__wsdd__Hello' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Hello(struct soap *soap, struct wsdd__HelloType *wsdd__Hello)
{
	AGTX_UNUSED(soap);
	AGTX_UNUSED(wsdd__Hello);

	ONVIF_TRACE("__wsdd__Hello\n");
	return SOAP_OK;
}
/** Web service operation '__wsdd__Bye' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Bye(struct soap *soap, struct wsdd__ByeType *wsdd__Bye)
{
	AGTX_UNUSED(soap);
	AGTX_UNUSED(wsdd__Bye);

	ONVIF_TRACE("__wsdd__Bye\n");
	return SOAP_OK;
}
/** Web service operation '__wsdd__Probe' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Probe(struct soap *soap, struct wsdd__ProbeType *wsdd__Probe)
{
#if 0
	char ip_buf[MAX_STR_LEN];
	struct wsdd__ProbeMatchesType probe_matches = {0};
	static struct wsa5__RelatesToType s_wsa_RelatesTo;
	static char s_MessageID[100] = "urn:uuid:315bacac-5566-7788-99aa-0012176a4047";

	ONVIF_TRACE("__wsdd__Probe\n");

	probe_matches.ProbeMatch = (struct wsdd__ProbeMatchType *)soap_malloc(soap, sizeof(struct wsdd__ProbeMatchType));
	memset(probe_matches.ProbeMatch,0x00,sizeof(struct wsdd__ProbeMatchType));
	probe_matches.ProbeMatch->XAddrs = (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	sprintf(probe_matches.ProbeMatch->XAddrs, "http://%s/onvif/device_service",SYS_Getipaddr("eth0",ip_buf));
	probe_matches.ProbeMatch->Types = (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(probe_matches.ProbeMatch->Types, "dn:NetworkVideoTransmitter");
	probe_matches.ProbeMatch->Scopes = (struct wsdd__ScopesType*)soap_malloc(soap,sizeof(struct wsdd__ScopesType));
	memset(probe_matches.ProbeMatch->Scopes,0x00,sizeof(struct wsdd__ScopesType));
	probe_matches.ProbeMatch->Scopes->__item =(char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(probe_matches.ProbeMatch->Scopes->__item,0,sizeof(char) * MAX_STR_LEN);
	strcat(probe_matches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/type/NetworkVideoTransmitter");
	probe_matches.ProbeMatch->wsa5__EndpointReference.Address = (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	sprintf(probe_matches.ProbeMatch->wsa5__EndpointReference.Address,"urn:uuid:315bacac-5566-7788-99aa-0012176a4047");
	probe_matches.ProbeMatch->MetadataVersion = 10;
	probe_matches.__sizeProbeMatch = 1;


	if(soap->header)
	{
		ONVIF_TRACE("soap->header\n");
		if(soap->header->wsa5__MessageID)
		{
			ONVIF_TRACE("wsa5__MessageID\n");
			soap->header->wsa5__RelatesTo=&s_wsa_RelatesTo;
			//soap_default__wsa5__RelatesTo(soap,soap->header->wsa5__RelatesTo);
			soap_default_wsa5__RelatesToType(soap,soap->header->wsa5__RelatesTo);
			soap->header->wsa5__RelatesTo->__item =soap->header->wsa5__MessageID;

			(soap->header->wsa5__MessageID) =s_MessageID;
			soap->header->wsa5__To ="http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous";
			soap->header->wsa5__Action ="http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches";
		}
	}


	soap_send___wsdd__ProbeMatches(soap, "http://", NULL, &probe_matches);
#else
	ONVIF_TRACE("__wsdd__Probe\n");
	char ip_buf[MAX_STR_LEN] = { 0 };
	char mac_addr[6] = { 0 };
	char _HwId[1024];

	wsdd__ProbeMatchesType ProbeMatches;
	ProbeMatches.ProbeMatch = (struct wsdd__ProbeMatchType *)soap_malloc(soap, sizeof(struct wsdd__ProbeMatchType));
	ProbeMatches.ProbeMatch->XAddrs = (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	ProbeMatches.ProbeMatch->Types = (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	ProbeMatches.ProbeMatch->Scopes = (struct wsdd__ScopesType *)soap_malloc(soap, sizeof(struct wsdd__ScopesType));
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceProperties =
	        (struct wsa__ReferencePropertiesType *)soap_malloc(soap, sizeof(struct wsa__ReferencePropertiesType));
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceParameters =
	        (struct wsa__ReferenceParametersType *)soap_malloc(soap, sizeof(struct wsa__ReferenceParametersType));
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ServiceName =
	        (struct wsa__ServiceNameType *)soap_malloc(soap, sizeof(struct wsa__ServiceNameType));
	ProbeMatches.ProbeMatch->wsa__EndpointReference.PortType =
	        (char **)soap_malloc(soap, sizeof(char *) * MAX_STR_LEN);
	ProbeMatches.ProbeMatch->wsa__EndpointReference.__any =
	        (char **)soap_malloc(soap, sizeof(char *) * MAX_STR_LEN);
	ProbeMatches.ProbeMatch->wsa__EndpointReference.__anyAttribute =
	        (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	ProbeMatches.ProbeMatch->wsa__EndpointReference.Address = (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);

	sprintf(_HwId, "urn:uuid:%s", soap_rand_uuid(soap, NULL));

	ProbeMatches.__sizeProbeMatch = 1;
	ProbeMatches.ProbeMatch->Scopes->__item = (char *)soap_malloc(soap, 1024);
	memset(ProbeMatches.ProbeMatch->Scopes->__item, 0, sizeof(char) * 1024);

	//Scopes MUST BE
	//strcat(ProbeMatches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/type/NetworkVideoTransmitter");
	strcpy(ProbeMatches.ProbeMatch->Scopes->__item,
	       "onvif://www.onvif.org/type/video_encoder onvif://www.onvif.org/type/audio_encoder onvif://www.onvif.org/hardware/IPC-model onvif://www.onvif.org/location/country/taipei onvif://www.onvif.org/name/NVT onvif://www.onvif.org/Profile/Streaming ");

	ProbeMatches.ProbeMatch->Scopes->MatchBy = NULL;
	sprintf(ProbeMatches.ProbeMatch->XAddrs, "http://%s:8899/onvif/device_service", SYS_Getipaddr("eth0", ip_buf));
	strcpy(ProbeMatches.ProbeMatch->Types, wsdd__Probe->Types);
	ONVIF_TRACE("wsdd__Probe->Types=%s\n", wsdd__Probe->Types);
	ProbeMatches.ProbeMatch->MetadataVersion = 1;
	//ws-discovery规定 为可选项
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceProperties->__size = 0;
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceProperties->__any = NULL;
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceParameters->__size = 0;
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceParameters->__any = NULL;

	ProbeMatches.ProbeMatch->wsa__EndpointReference.PortType[0] =
	        (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	//ws-discovery规定 为可选项
	strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.PortType[0], "ttl");
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ServiceName->__item = NULL;
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ServiceName->PortName = NULL;
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ServiceName->__anyAttribute = NULL;
	ProbeMatches.ProbeMatch->wsa__EndpointReference.__any[0] =
	        (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.__any[0], "Any");
	strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.__anyAttribute, "Attribute");
	ProbeMatches.ProbeMatch->wsa__EndpointReference.__size = 0;
	//strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.Address,"urn:uuid:315bacac-5566-7788-99aa-0012176a4047");
	SYS_Getmacaddr("eth0", mac_addr);
	sprintf(ProbeMatches.ProbeMatch->wsa__EndpointReference.Address,
	        "urn:uuid:315bacac-5566-7788-99aa-%02x%02x%02x%02x%02x%02x", mac_addr[0], mac_addr[1], mac_addr[2],
	        mac_addr[3], mac_addr[4], mac_addr[5]);
	/*注释的部分为可选，注释掉onvif test也能发现ws-d*/
	soap->header->wsa__To = "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous";
	soap->header->wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches";
	soap->header->wsa__RelatesTo = (struct wsa__Relationship *)soap_malloc(soap, sizeof(struct wsa__Relationship));
	//it's here
	soap->header->wsa__RelatesTo->__item = soap->header->wsa__MessageID;
	soap->header->wsa__RelatesTo->RelationshipType = NULL;
	soap->header->wsa__RelatesTo->__anyAttribute = NULL;

	soap->header->wsa__MessageID = (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	//strcpy(soap->header->wsa__MessageID,_HwId+4);
	strcpy(soap->header->wsa__MessageID, _HwId + 4);

	/* send over current socket as HTTP OK response: */
	/*测试过，第二参数必须http，action随意*/
	soap_send___wsdd__ProbeMatches(soap, "http://", NULL, &ProbeMatches);

#endif
	return SOAP_OK;
}
/** Web service operation '__wsdd__ProbeMatches' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ProbeMatches(struct soap *soap, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{
	AGTX_UNUSED(soap);
	AGTX_UNUSED(wsdd__ProbeMatches);

	ONVIF_TRACE("__wsdd__ProbeMatches\n");
	return SOAP_OK;
}
/** Web service operation '__wsdd__Resolve' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Resolve(struct soap *soap, struct wsdd__ResolveType *wsdd__Resolve)
{
	AGTX_UNUSED(soap);
	AGTX_UNUSED(wsdd__Resolve);

	ONVIF_TRACE("__wsdd__Resolve\n");
	return SOAP_OK;
}
/** Web service operation '__wsdd__ResolveMatches' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ResolveMatches(struct soap *soap,
                                                 struct wsdd__ResolveMatchesType *wsdd__ResolveMatches)
{
	AGTX_UNUSED(soap);
	AGTX_UNUSED(wsdd__ResolveMatches);

	ONVIF_TRACE("__wsdd__ResolveMatches\n");
	return SOAP_OK;
}
/** Web service operation '__tdn__Hello' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tdn__Hello(struct soap *soap, struct wsdd__HelloType tdn__Hello,
                                       struct wsdd__ResolveType *tdn__HelloResponse)
{
	AGTX_UNUSED(soap);
	AGTX_UNUSED(tdn__Hello);
	AGTX_UNUSED(tdn__HelloResponse);

	ONVIF_TRACE("__tdn__Hello\n");
	return SOAP_OK;
}
/** Web service operation '__tdn__Bye' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tdn__Bye(struct soap *soap, struct wsdd__ByeType tdn__Bye,
                                     struct wsdd__ResolveType *tdn__ByeResponse)
{
	AGTX_UNUSED(soap);
	AGTX_UNUSED(tdn__Bye);
	AGTX_UNUSED(tdn__ByeResponse);

	ONVIF_TRACE("__tdn__Bye\n");
	return SOAP_OK;
}
/** Web service operation '__tdn__Probe' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tdn__Probe(struct soap *soap, struct wsdd__ProbeType tdn__Probe,
                                       struct wsdd__ProbeMatchesType *tdn__ProbeResponse)
{
	AGTX_UNUSED(soap);
	AGTX_UNUSED(tdn__Probe);
	AGTX_UNUSED(tdn__ProbeResponse);

	ONVIF_TRACE("__tdn__Probe\n");
	return SOAP_OK;
}
