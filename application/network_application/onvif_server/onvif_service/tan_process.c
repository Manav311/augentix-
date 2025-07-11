#include "stdio.h"
#include "stdlib.h"
#include "soapStub.h"
#include "stdsoap2.h"
#include "augentix.h"
#include "md_grid.h"

#define TAN_INIT_ENTRY(__soapp, __entryp, __size)        \
	do {                                             \
		__entryp = soap_malloc(__soapp, __size); \
		memset(__entryp, 0, __size);             \
	} while (0)

/** Web service operation '__tan__GetSupportedRules' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tan__GetSupportedRules(struct soap *soap, struct _tan__GetSupportedRules *tan__GetSupportedRules,
                         struct _tan__GetSupportedRulesResponse *tan__GetSupportedRulesResponse)
{
	int item_num = 0;
	struct tt__ConfigDescription *RuleDescription_ = NULL;

	ONVIF_TRACE("__tan__GetSupportedRules\n");

	tan__GetSupportedRulesResponse->SupportedRules = soap_malloc(soap, sizeof(struct tt__SupportedRules));
	memset(tan__GetSupportedRulesResponse->SupportedRules, 0, sizeof(struct tt__SupportedRules));
	tan__GetSupportedRulesResponse->SupportedRules->__sizeRuleDescription = 1;
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription_ =
	        soap_malloc(soap, sizeof(struct tt__ConfigDescription));
	RuleDescription_ = tan__GetSupportedRulesResponse->SupportedRules->RuleDescription_;
	memset(RuleDescription_, 0, sizeof(struct tt__ConfigDescription));
	RuleDescription_->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Name, "tt:CellMotionDetector");
	RuleDescription_->Parameters = soap_malloc(soap, sizeof(struct tt__ItemListDescription));
	memset(RuleDescription_->Parameters, 0, sizeof(struct tt__ItemListDescription));
	item_num = 4;
	RuleDescription_->Parameters->SimpleItemDescription_ =
	        soap_malloc(soap, sizeof(struct _tt__ItemListDescription_SimpleItemDescription) * item_num);
	memset(RuleDescription_->Parameters->SimpleItemDescription_, 0,
	       sizeof(struct _tt__ItemListDescription_SimpleItemDescription) * item_num);
	RuleDescription_->Parameters->SimpleItemDescription_[0].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Parameters->SimpleItemDescription_[0].Name, "MinCount");
	RuleDescription_->Parameters->SimpleItemDescription_[0].Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Parameters->SimpleItemDescription_[0].Type, "xs:integer");
	RuleDescription_->Parameters->SimpleItemDescription_[1].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Parameters->SimpleItemDescription_[1].Name, "AlarmOnDelay");
	RuleDescription_->Parameters->SimpleItemDescription_[1].Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Parameters->SimpleItemDescription_[1].Type, "xs:integer");
	RuleDescription_->Parameters->SimpleItemDescription_[2].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Parameters->SimpleItemDescription_[2].Name, "AlarmOffDelay");
	RuleDescription_->Parameters->SimpleItemDescription_[2].Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Parameters->SimpleItemDescription_[2].Type, "xs:integer");
	RuleDescription_->Parameters->SimpleItemDescription_[3].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Parameters->SimpleItemDescription_[3].Name, "ActiveCells");
	RuleDescription_->Parameters->SimpleItemDescription_[3].Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Parameters->SimpleItemDescription_[3].Type, "xs:base64Binary");

	RuleDescription_->Messages_ = soap_malloc(soap, sizeof(struct _tt__ConfigDescription_Messages));
	memset(RuleDescription_->Messages_, 0, sizeof(struct _tt__ConfigDescription_Messages));
	RuleDescription_->Messages_->IsProperty = soap_malloc(soap, sizeof(enum xsd__boolean));
	*RuleDescription_->Messages_->IsProperty = xsd__boolean__true_;
	RuleDescription_->Messages_->Source = soap_malloc(soap, sizeof(struct tt__ItemListDescription));
	memset(RuleDescription_->Messages_->Source, 0, sizeof(struct tt__ItemListDescription));
	item_num = 3;
	RuleDescription_->Messages_->Source->__sizeSimpleItemDescription = item_num;
	RuleDescription_->Messages_->Source->SimpleItemDescription_ =
	        soap_malloc(soap, sizeof(struct _tt__ItemListDescription_SimpleItemDescription) * 3);
	RuleDescription_->Messages_->Source->SimpleItemDescription_[0].Name =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Messages_->Source->SimpleItemDescription_[0].Name, "VideoSourceConfigurationToken");
	RuleDescription_->Messages_->Source->SimpleItemDescription_[0].Type =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Messages_->Source->SimpleItemDescription_[0].Type, "tt:ReferenceToken");
	RuleDescription_->Messages_->Source->SimpleItemDescription_[1].Name =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Messages_->Source->SimpleItemDescription_[1].Name, "VideoAnalyticsConfigurationToken");
	RuleDescription_->Messages_->Source->SimpleItemDescription_[1].Type =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Messages_->Source->SimpleItemDescription_[1].Type, "tt:ReferenceToken");
	RuleDescription_->Messages_->Source->SimpleItemDescription_[2].Name =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Messages_->Source->SimpleItemDescription_[2].Name, "Rule");
	RuleDescription_->Messages_->Source->SimpleItemDescription_[2].Type =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Messages_->Source->SimpleItemDescription_[2].Type, "xs:string");

	RuleDescription_->Messages_->Data = soap_malloc(soap, sizeof(struct tt__ItemListDescription));
	memset(RuleDescription_->Messages_->Data, 0, sizeof(struct tt__ItemListDescription));
	RuleDescription_->Messages_->Data->__sizeSimpleItemDescription = 1;
	RuleDescription_->Messages_->Data->SimpleItemDescription_ =
	        soap_malloc(soap, sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
	RuleDescription_->Messages_->Data->SimpleItemDescription_->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Messages_->Source->SimpleItemDescription_->Name, "IsMotion");
	RuleDescription_->Messages_->Data->SimpleItemDescription_->Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Messages_->Source->SimpleItemDescription_->Name, "xs:boolean");
	RuleDescription_->Messages_->ParentTopic = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(RuleDescription_->Messages_->ParentTopic, "tns1:RuleEngine/CellMotionDetector/Motion");

	RuleDescription_->Parameters->ElementItemDescription_ =
	        soap_malloc(soap, sizeof(struct _tt__ItemListDescription_ElementItemDescription));

	return SOAP_OK;
}
/** Web service operation '__tan__CreateRules' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateRules(struct soap *soap, struct _tan__CreateRules *tan__CreateRules,
                                             struct _tan__CreateRulesResponse *tan__CreateRulesResponse)
{
	ONVIF_TRACE("__tan__CreateRules TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tan__DeleteRules' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteRules(struct soap *soap, struct _tan__DeleteRules *tan__DeleteRules,
                                             struct _tan__DeleteRulesResponse *tan__DeleteRulesResponse)
{
	ONVIF_TRACE("__tan__DeleteRules TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tan__GetRules' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tan__GetRules(struct soap *soap, struct _tan__GetRules *tan__GetRules,
                                          struct _tan__GetRulesResponse *tan__GetRulesResponse)
{
	int i = 0;
	int item_num = 0;
	int va_num = 2;
	int va_index = 0;
	int polygon_num = 4;
	struct tt__Config *rule = NULL;
	struct tt__PolygonConfiguration *polygon = NULL;
	AGTX_IVA_MD_CONF_S md_conf;

	ONVIF_TRACE("__tan__GetRules\n");

	/*Get MD config*/
	if (aux_get_cc_config(AGTX_CMD_MD_CONF, &md_conf) < 0) {
		return SOAP_NO_DATA;
	}

	tan__GetRulesResponse->__sizeRule = va_num;
	tan__GetRulesResponse->Rule_ = soap_malloc(soap, sizeof(struct tt__Config) * va_num);
	memset(tan__GetRulesResponse->Rule_, 0x00, sizeof(struct tt__Config) * va_num);

	/*motion rule*/
	rule = &tan__GetRulesResponse->Rule_[va_index];
	rule->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Name, "MyMotionDetectorRule");
	rule->Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Type, "tt:CellMotionDetector");
	rule->Parameters = soap_malloc(soap, sizeof(struct tt__ItemList));
	memset(rule->Parameters, 0x00, sizeof(struct tt__ItemList));
	item_num = 4;
	rule->Parameters->__sizeSimpleItem = item_num;
	rule->Parameters->SimpleItem_ = soap_malloc(soap, sizeof(struct tt__ItemList) * item_num);
	memset(rule->Parameters->SimpleItem_, 0x00, (sizeof(struct tt__ItemList) * item_num));
	rule->Parameters->SimpleItem_[0].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[0].Name, "MinCount");
	rule->Parameters->SimpleItem_[0].Value = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[0].Value, "5");
	rule->Parameters->SimpleItem_[1].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[1].Name, "AlarmOnDelay");
	rule->Parameters->SimpleItem_[1].Value = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[1].Value, "1000");
	rule->Parameters->SimpleItem_[2].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[2].Name, "AlarmOffDelay");
	rule->Parameters->SimpleItem_[2].Value = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[2].Value, "1000");
	rule->Parameters->SimpleItem_[3].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[3].Name, "ActiveCells");
	rule->Parameters->SimpleItem_[3].Value = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[3].Value, (char *)md_conf.active_cell);
	va_index++;

	/*tamper rule*/
	rule = &tan__GetRulesResponse->Rule_[va_index];
	rule->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Name, "MyTamperDetectorRule");
	rule->Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Type, "hikxsd:TamperDetector");
	rule->Parameters = soap_malloc(soap, sizeof(struct tt__ItemList));
	memset(rule->Parameters, 0x00, sizeof(struct tt__ItemList));
	rule->Parameters->__sizeElementItem = 1;
	rule->Parameters->ElementItem_ = soap_malloc(soap, sizeof(struct _tt__ItemList_ElementItem));
	memset(rule->Parameters->ElementItem_, 0x00, sizeof(struct _tt__ItemList_ElementItem));
	rule->Parameters->ElementItem_->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->ElementItem_->Name, "Field");
	rule->Parameters->ElementItem_->PolygonConfiguration =
	        soap_malloc(soap, sizeof(struct tt__PolygonConfiguration));
	polygon = (struct tt__PolygonConfiguration *)rule->Parameters->ElementItem_->PolygonConfiguration;
	memset(polygon, 0, sizeof(struct tt__PolygonConfiguration));
	polygon->Polygon = soap_malloc(soap, sizeof(struct tt__Polygon));
	memset(polygon->Polygon, 0x00, sizeof(struct tt__Polygon));
	polygon->Polygon->__sizePoint = polygon_num;
	polygon->Polygon->Point_ = soap_malloc(soap, sizeof(struct tt__Vector) * polygon_num);
	memset(polygon->Polygon->Point_, 0x00, sizeof(struct tt__Vector) * polygon_num);
	for (i = 0; i < polygon_num; i++) {
		polygon->Polygon->Point_[i].x = soap_malloc(soap, sizeof(float));
		polygon->Polygon->Point_[i].y = soap_malloc(soap, sizeof(float));
		*polygon->Polygon->Point_[i].x = 0;
		*polygon->Polygon->Point_[i].y = 0;
	}

	return SOAP_OK;
}
/** Web service operation '__tan__GetRuleOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tan__GetRuleOptions(struct soap *soap, struct _tan__GetRuleOptions *tan__GetRuleOptions,
                                                struct _tan__GetRuleOptionsResponse *tan__GetRuleOptionsResponse)
{
	ONVIF_TRACE("__tan__GetRuleOptions TODO\n");
	return SOAP_OK;
}

char *CaculateMdCelltoRoi(struct tt__ItemList *parameters, int *enable, MD_REG_LIST *list)
{
	int i = 0;
	int item_num = 0;
	struct _tt__ItemList_SimpleItem *simple = NULL;

	item_num = parameters->__sizeSimpleItem;

	for (i = 0; i < item_num; i++) {
		simple = &parameters->SimpleItem_[i];
		if (simple->Name && simple->Value) {
			if (!strcmp(simple->Name, "ActiveCells")) {
				ONVIF_TRACE("ActiveCells %s\n", simple->Value);
				MDGRID_decodeActiveCells(simple->Value, list);
				*enable = list->region_n ? 1 : 0;
				return simple->Value;
			}
		}
	}

	return NULL;
}

/** Web service operation '__tan__ModifyRules' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyRules(struct soap *soap, struct _tan__ModifyRules *tan__ModifyRules,
                                             struct _tan__ModifyRulesResponse *tan__ModifyRulesResponse)
{
	int i = 0;
	int enable_motion = 0;
	char *active_cell_tmp = NULL;
	struct tt__Config *rule = NULL;
	MD_REG_LIST list = { 0 };
	AGTX_IVA_MD_CONF_S md_conf;

	ONVIF_TRACE("__tan__ModifyRules\n");

	for (i = 0; i < tan__ModifyRules->__sizeRule; i++) {
		if (tan__ModifyRules && tan__ModifyRules->Rule_ && tan__ModifyRules->Rule_[i].Parameters &&
		    tan__ModifyRules->Rule_[i].Parameters->SimpleItem_) {
			rule = &tan__ModifyRules->Rule_[i];
			if (!strcmp(rule->Name, "MyMotionDetectorRule") &&
			    !strcmp(rule->Type, "tt:CellMotionDetector")) {
				active_cell_tmp = CaculateMdCelltoRoi(rule->Parameters, &enable_motion, &list);
				continue;
			}

			if (!strcmp(rule->Name, "MyTamperDetectorRule") && !strcmp(rule->Type, "\"\":TamperDetector")) {
				//TODO if we have ROI for TD
				continue;
			}
		}
	}

	if (active_cell_tmp) {
		if (aux_get_cc_config(AGTX_CMD_MD_CONF, &md_conf) < 0) {
			return SOAP_NO_DATA;
		}

		if (md_conf.sens == 0) {
			enable_motion = 0;
		}

		ONVIF_TRACE("enable_motion %d\n", enable_motion);

		md_conf.enabled = enable_motion;
		md_conf.rgn_cnt = list.region_n;
		strcpy((char*)md_conf.active_cell, active_cell_tmp);
		for (i = 0; i < list.region_n; i++) {
			md_conf.rgn_list[i].sens = md_conf.sens;
			md_conf.rgn_list[i].max_spd = md_conf.max_spd;
			md_conf.rgn_list[i].min_spd = md_conf.min_spd;
			md_conf.rgn_list[i].mode = md_conf.mode;
			md_conf.rgn_list[i].sx = list.reg[i].sx;
			md_conf.rgn_list[i].sy = list.reg[i].sy;
			md_conf.rgn_list[i].ex = list.reg[i].ex;
			md_conf.rgn_list[i].ey = list.reg[i].ey;
		}

		if (aux_set_cc_config(AGTX_CMD_MD_CONF, &md_conf) < 0) {
			return SOAP_NO_DATA;
		}
	}

	return SOAP_OK;
}
/** Web service operation '__tan__GetServiceCapabilities' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tan__GetServiceCapabilities(struct soap *soap, struct _tan__GetServiceCapabilities *tan__GetServiceCapabilities,
                              struct _tan__GetServiceCapabilitiesResponse *tan__GetServiceCapabilitiesResponse)
{
	ONVIF_TRACE("__tan__GetServiceCapabilities TODO\n");

	return SOAP_OK;
}

/** Web service operation '__tan__GetSupportedAnalyticsModules' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedAnalyticsModules(
        struct soap *soap, struct _tan__GetSupportedAnalyticsModules *tan__GetSupportedAnalyticsModules,
        struct _tan__GetSupportedAnalyticsModulesResponse *tan__GetSupportedAnalyticsModulesResponse)
{
	//	int item_num = 0;
	//	struct tt__ConfigDescription *AnalyticsModuleDescription_ = NULL;
	//	struct tt__ConfigDescription *MessageDescription_         = NULL;

	ONVIF_TRACE("__tan__GetSupportedAnalyticsModules\n");

	//	TAN_INIT_ENTRY(soap,
	//	    tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules,
	//	    sizeof(struct tt__SupportedAnalyticsModules));
	//
	//	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->__sizeAnalyticsModuleContentSchemaLocation = 1;
	//	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation_ = soap_malloc(soap,sizeof(char*));
	//	*tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation_ = soap_malloc(soap,sizeof(char) * MAX_STR_LEN);
	//	strcpy(*tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation_, "http://www.w3.org/2001/XMLSchema");
	//
	//	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->__sizeAnalyticsModuleDescription = 1;
	//	/* AnalyticsModuleDescription */
	//	TAN_INIT_ENTRY(soap,
	//		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription_,
	//		sizeof(struct tt__ConfigDescription));
	//	AnalyticsModuleDescription_ = tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription_;
	//
	//	TAN_INIT_ENTRY(soap,
	//		AnalyticsModuleDescription_->Name,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Name, "tt:CellMotionEngine");
	//
	//	TAN_INIT_ENTRY(soap,
	//		AnalyticsModuleDescription_->Parameters,
	//		sizeof(struct tt__ItemListDescription));
	//
	//	AnalyticsModuleDescription_->Parameters->__sizeSimpleItemDescription = 1;
	//	TAN_INIT_ENTRY(soap,
	//		AnalyticsModuleDescription_->Parameters->SimpleItemDescription_,
	//		sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
	//
	//	TAN_INIT_ENTRY(soap,
	//		AnalyticsModuleDescription_->Parameters->SimpleItemDescription_->Name,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Parameters->SimpleItemDescription_->Name,"Sensitivity");
	//
	//	TAN_INIT_ENTRY(soap,
	//		AnalyticsModuleDescription_->Parameters->SimpleItemDescription_->Type,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Parameters->SimpleItemDescription_->Type,"xs:integer");
	//
	//	AnalyticsModuleDescription_->Parameters->__sizeElementItemDescription = 1;
	//	TAN_INIT_ENTRY(soap,
	//		AnalyticsModuleDescription_->Parameters->ElementItemDescription_,
	//		sizeof(struct _tt__ItemListDescription_ElementItemDescription));
	//
	//	TAN_INIT_ENTRY(soap,
	//		AnalyticsModuleDescription_->Parameters->ElementItemDescription_->Name,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Parameters->ElementItemDescription_->Name,"Layout");
	//
	//	TAN_INIT_ENTRY(soap,
	//		AnalyticsModuleDescription_->Parameters->ElementItemDescription_->Type,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Parameters->ElementItemDescription_->Type,"tt:CellLayout");
	//
	//	AnalyticsModuleDescription_->__sizeMessages = 1;
	//	TAN_INIT_ENTRY(soap,
	//		AnalyticsModuleDescription_->Messages_,
	//		sizeof(struct _tt__ConfigDescription_Messages));
	//
	//	TAN_INIT_ENTRY(soap,
	//	               AnalyticsModuleDescription_->Messages_->IsProperty,
	//	    sizeof(enum xsd__boolean));
	//	*AnalyticsModuleDescription_->Messages_->IsProperty = xsd__boolean__true_;
	//
	//	TAN_INIT_ENTRY(soap,
	//		AnalyticsModuleDescription_->Messages_->Source,
	//		sizeof(struct tt__ItemListDescription));
	//
	//	item_num = 3;
	//	AnalyticsModuleDescription_->Messages_->Source->__sizeSimpleItemDescription = item_num;
	//	TAN_INIT_ENTRY(soap,
	//	    AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_,
	//		sizeof(struct _tt__ItemListDescription_SimpleItemDescription) * item_num);
	//
	//	TAN_INIT_ENTRY(soap,
	//	    AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_[0].Name,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_[0].Name,"VideoSourceConfigurationToken");
	//
	//	TAN_INIT_ENTRY(soap,
	//	    AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_[0].Type,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_[0].Type,"tt:ReferenceToken");
	//
	//	TAN_INIT_ENTRY(soap,
	//	    AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_[1].Name,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_[1].Name,"VideoAnalyticsConfigurationToken");
	//
	//	TAN_INIT_ENTRY(soap,
	//	    AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_[1].Type,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_[1].Type,"tt:ReferenceToken");
	//
	//	TAN_INIT_ENTRY(soap,
	//	    AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_[2].Name,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_[2].Name,"Rule");
	//
	//	TAN_INIT_ENTRY(soap,
	//	    AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_[2].Type,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Messages_->Source->SimpleItemDescription_[2].Type,"xs:string");
	//
	//	TAN_INIT_ENTRY(soap,
	//		AnalyticsModuleDescription_->Messages_->Data,
	//		sizeof(struct tt__ItemListDescription));
	//
	//	AnalyticsModuleDescription_->Messages_->Data->__sizeSimpleItemDescription = 1;
	//	TAN_INIT_ENTRY(soap,
	//	    AnalyticsModuleDescription_->Messages_->Data->SimpleItemDescription_,
	//		sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
	//
	//	TAN_INIT_ENTRY(soap,
	//	    AnalyticsModuleDescription_->Messages_->Data->SimpleItemDescription_->Name,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Messages_->Data->SimpleItemDescription_->Name,"IsMotion");
	//
	//	TAN_INIT_ENTRY(soap,
	//	               AnalyticsModuleDescription_->Messages_->Data->SimpleItemDescription_->Type,
	//		sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Messages_->Data->SimpleItemDescription_->Type,"xs:boolean");
	//
	//	TAN_INIT_ENTRY(soap,
	//	    AnalyticsModuleDescription_->Messages_->ParentTopic,
	//	    sizeof(char) * MAX_STR_LEN);
	//	strcpy(AnalyticsModuleDescription_->Messages_->ParentTopic,"tns1:RuleEngine/CellMotionDetector/Motion");

	return SOAP_OK;
}

/** Web service operation '__tan__CreateAnalyticsModules' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tan__CreateAnalyticsModules(struct soap *soap, struct _tan__CreateAnalyticsModules *tan__CreateAnalyticsModules,
                              struct _tan__CreateAnalyticsModulesResponse *tan__CreateAnalyticsModulesResponse)
{
	ONVIF_TRACE("__tan__CreateAnalyticsModules TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tan__DeleteAnalyticsModules' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tan__DeleteAnalyticsModules(struct soap *soap, struct _tan__DeleteAnalyticsModules *tan__DeleteAnalyticsModules,
                              struct _tan__DeleteAnalyticsModulesResponse *tan__DeleteAnalyticsModulesResponse)
{
	ONVIF_TRACE("__tan__DeleteAnalyticsModules TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tan__GetAnalyticsModules' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tan__GetAnalyticsModules(struct soap *soap, struct _tan__GetAnalyticsModules *tan__GetAnalyticsModules,
                           struct _tan__GetAnalyticsModulesResponse *tan__GetAnalyticsModulesResponse)
{
	int i = 0;
	int va_num = 2;
	int va_index = 0;
	int polygon_num = 4;
	struct tt__CellLayout *layout = NULL;
	struct tt__Config *va_module = NULL;
	struct tt__PolygonConfiguration *polygon = NULL;
	AGTX_IVA_MD_CONF_S md_conf;
	AGTX_IVA_TD_CONF_S td_conf = { 0 };

	ONVIF_TRACE("__tan__GetAnalyticsModules\n");

	/*Get MD config*/
	if(aux_get_cc_config(AGTX_CMD_MD_CONF, &md_conf) < 0) {
		return SOAP_NO_DATA;
	}

	/*Get TD config*/
	if (aux_get_cc_config(AGTX_CMD_TD_CONF, &td_conf) < 0) {
				return SOAP_NO_DATA;
	}

	tan__GetAnalyticsModulesResponse->__sizeAnalyticsModule = va_num;
	tan__GetAnalyticsModulesResponse->AnalyticsModule_ = soap_malloc(soap, sizeof(struct tt__Config) * va_num);
	memset(tan__GetAnalyticsModulesResponse->AnalyticsModule_, 0x00, sizeof(struct tt__Config) * va_num);

	/*motion module*/
	va_module = &tan__GetAnalyticsModulesResponse->AnalyticsModule_[va_index];
	memset(va_module, 0x00, sizeof(struct tt__Config));
	va_module->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Name, "MyCellMotionModule");
	va_module->Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Type, "tt:CellMotionEngine");
	va_module->Parameters = soap_malloc(soap, sizeof(struct tt__ItemList));
	memset(va_module->Parameters, 0x00, sizeof(struct tt__ItemList));
	va_module->Parameters->__sizeElementItem = 1;
	va_module->Parameters->__sizeSimpleItem = 1;
	va_module->Parameters->SimpleItem_ = soap_malloc(soap, sizeof(struct _tt__ItemList_SimpleItem));
	memset(va_module->Parameters->SimpleItem_, 0x00, sizeof(struct _tt__ItemList_SimpleItem));
	va_module->Parameters->SimpleItem_->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Parameters->SimpleItem_->Name, "Sensitivity");
	va_module->Parameters->SimpleItem_->Value = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	sprintf(va_module->Parameters->SimpleItem_->Value, "%d", md_conf.enabled ? md_conf.sens : 0);
	va_module->Parameters->ElementItem_ = soap_malloc(soap, sizeof(struct _tt__ItemList_ElementItem));
	memset(va_module->Parameters->ElementItem_, 0x00, sizeof(struct _tt__ItemList_ElementItem));
	va_module->Parameters->ElementItem_->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Parameters->ElementItem_->Name, "Layout");
	va_module->Parameters->ElementItem_->CellLayout = soap_malloc(soap, sizeof(struct tt__CellLayout));
	layout = (struct tt__CellLayout *)va_module->Parameters->ElementItem_->CellLayout;
	memset(layout, 0, sizeof(struct tt__CellLayout));
	layout->Columns = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(layout->Columns, "16"); //22
	layout->Rows = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(layout->Rows, "12"); //15
	layout->Transformation = soap_malloc(soap, sizeof(struct tt__Transformation));
	memset(layout->Transformation, 0, sizeof(struct tt__Transformation));
	layout->Transformation->Translate = soap_malloc(soap, sizeof(struct tt__Vector));
	layout->Transformation->Translate->x = soap_malloc(soap, sizeof(float));
	*layout->Transformation->Translate->x = -1.000000;
	layout->Transformation->Translate->y = soap_malloc(soap, sizeof(float));
	*layout->Transformation->Translate->y = -1.000000;
	layout->Transformation->Scale = soap_malloc(soap, sizeof(struct tt__Vector));
	layout->Transformation->Scale->x = soap_malloc(soap, sizeof(float));
	*layout->Transformation->Scale->x = 0.006250; //0.090909;
	layout->Transformation->Scale->y = soap_malloc(soap, sizeof(float));
	*layout->Transformation->Scale->y = 0.008340; //0.133333;
	va_index++;

	/*tamper module*/
	va_module = &tan__GetAnalyticsModulesResponse->AnalyticsModule_[va_index];
	memset(va_module, 0x00, sizeof(struct tt__Config));
	va_module->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Name, "MyTamperDetecModule");
	va_module->Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Type, "hikxsd:TamperEngine");
	va_module->Parameters = soap_malloc(soap, sizeof(struct tt__ItemList));
	memset(va_module->Parameters, 0x00, sizeof(struct tt__ItemList));
	va_module->Parameters->__sizeSimpleItem = 1;
	va_module->Parameters->SimpleItem_ = soap_malloc(soap, sizeof(struct _tt__ItemList_SimpleItem));
	memset(va_module->Parameters->SimpleItem_, 0x00, sizeof(struct _tt__ItemList_SimpleItem));
	va_module->Parameters->SimpleItem_->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Parameters->SimpleItem_->Name, "Sensitivity");
	va_module->Parameters->SimpleItem_->Value = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	/*Dislabe TD by td_sensitivity equal zero*/
	sprintf(va_module->Parameters->SimpleItem_->Value, "%d", td_conf.enabled ? td_conf.sensitivity : 0);
	va_module->Parameters->__sizeElementItem = 2;
	va_module->Parameters->ElementItem_ =
	        soap_malloc(soap, sizeof(struct _tt__ItemList_ElementItem) * va_module->Parameters->__sizeElementItem);
	memset(va_module->Parameters->ElementItem_, 0x00,
	       sizeof(struct _tt__ItemList_ElementItem) * va_module->Parameters->__sizeElementItem);
	va_module->Parameters->ElementItem_[0].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Parameters->ElementItem_[0].Name, "Transformation");
	va_module->Parameters->ElementItem_[0].Transformation = soap_malloc(soap, sizeof(struct tt__Transformation));
	memset(va_module->Parameters->ElementItem_[0].Transformation, 0, sizeof(struct tt__Transformation));
	va_module->Parameters->ElementItem_[0].Transformation->Translate = soap_malloc(soap, sizeof(struct tt__Vector));
	va_module->Parameters->ElementItem_[0].Transformation->Translate->x = soap_malloc(soap, sizeof(float));
	*va_module->Parameters->ElementItem_[0].Transformation->Translate->x = -1.000000;
	va_module->Parameters->ElementItem_[0].Transformation->Translate->y = soap_malloc(soap, sizeof(float));
	*va_module->Parameters->ElementItem_[0].Transformation->Translate->y = -1.000000;
	va_module->Parameters->ElementItem_[0].Transformation->Scale = soap_malloc(soap, sizeof(struct tt__Vector));
	va_module->Parameters->ElementItem_[0].Transformation->Scale->x = soap_malloc(soap, sizeof(float));
	*va_module->Parameters->ElementItem_[0].Transformation->Scale->x = 0.002841;
	va_module->Parameters->ElementItem_[0].Transformation->Scale->y = soap_malloc(soap, sizeof(float));
	*va_module->Parameters->ElementItem_[0].Transformation->Scale->y = 0.004167;

	va_module->Parameters->ElementItem_[1].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Parameters->ElementItem_[1].Name, "Field");
	va_module->Parameters->ElementItem_[1].PolygonConfiguration =
	        soap_malloc(soap, sizeof(struct tt__PolygonConfiguration));
	polygon = (struct tt__PolygonConfiguration *)va_module->Parameters->ElementItem_[1].PolygonConfiguration;
	memset(polygon, 0, sizeof(struct tt__PolygonConfiguration));
	polygon->Polygon = soap_malloc(soap, sizeof(struct tt__Polygon));
	memset(polygon->Polygon, 0x00, sizeof(struct tt__Polygon));
	polygon->Polygon->__sizePoint = polygon_num;
	polygon->Polygon->Point_ = soap_malloc(soap, sizeof(struct tt__Vector) * polygon_num);
	memset(polygon->Polygon->Point_, 0x00, sizeof(struct tt__Vector) * polygon_num);
	for (i = 0; i < polygon_num; i++) {
		polygon->Polygon->Point_[i].x = soap_malloc(soap, sizeof(float));
		polygon->Polygon->Point_[i].y = soap_malloc(soap, sizeof(float));
		*polygon->Polygon->Point_[i].x = ((i == 2) || (i == 3)) ? 704 : 0;
		*polygon->Polygon->Point_[i].y = ((i == 1) || (i == 2)) ? 480 : 0;
	}

	return SOAP_OK;
}
/** Web service operation '__tan__GetAnalyticsModuleOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tan__GetAnalyticsModuleOptions(
        struct soap *soap, struct _tan__GetAnalyticsModuleOptions *tan__GetAnalyticsModuleOptions,
        struct _tan__GetAnalyticsModuleOptionsResponse *tan__GetAnalyticsModuleOptionsResponse)
{
	ONVIF_TRACE("__tan__GetAnalyticsModuleOptions TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tan__ModifyAnalyticsModules' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tan__ModifyAnalyticsModules(struct soap *soap, struct _tan__ModifyAnalyticsModules *tan__ModifyAnalyticsModules,
                              struct _tan__ModifyAnalyticsModulesResponse *tan__ModifyAnalyticsModulesResponse)
{
	int i = 0;
	int md_sensitivity = -1;
	int td_sensitivity = -1;
	struct tt__Config *va_module = NULL;
	struct _tt__ItemList_SimpleItem *simple = NULL;
	AGTX_IVA_MD_CONF_S md_conf;
	AGTX_IVA_TD_CONF_S td_conf = { 0 };

	ONVIF_TRACE("__tan__ModifyAnalyticsModules\n");

	for (i = 0; i < tan__ModifyAnalyticsModules->__sizeAnalyticsModule; i++) {
		if (tan__ModifyAnalyticsModules && tan__ModifyAnalyticsModules->AnalyticsModule_ &&
		    tan__ModifyAnalyticsModules->AnalyticsModule_[i].Parameters &&
		    tan__ModifyAnalyticsModules->AnalyticsModule_[i].Parameters->SimpleItem_) {
			va_module = &tan__ModifyAnalyticsModules->AnalyticsModule_[i];
			simple = va_module->Parameters->SimpleItem_;
			if (!strcmp(va_module->Name, "MyCellMotionModule") &&
			    !strcmp(va_module->Type, "tt:CellMotionEngine") && simple->Name &&
			    !strcmp(simple->Name, "Sensitivity") && simple->Value) {
				md_sensitivity = atoi(simple->Value);
				continue;
			}

			if (!strcmp(va_module->Name, "MyTamperDetecModule") &&
			    !strcmp(va_module->Type, "\"\":TamperEngine") && simple->Name &&
			    !strcmp(simple->Name, "Sensitivity") && simple->Value) {
				td_sensitivity = atoi(simple->Value);
				continue;
			}
		}
	}

	ONVIF_TRACE("md_sensitivity %d td_sensitivity %d\n", md_sensitivity, td_sensitivity);

	if (md_sensitivity >= 0) {
		/*Get MD config*/
		if(aux_get_cc_config(AGTX_CMD_MD_CONF, &md_conf) < 0) {
			return SOAP_NO_DATA;
		}

		md_conf.enabled = (md_sensitivity == 0) ? 0 : 1;
		md_conf.sens = md_sensitivity;
		for (i = 0; i < md_conf.rgn_cnt; i++) {
			md_conf.rgn_list[i].sens = md_conf.sens;
		}

		if (aux_set_cc_config(AGTX_CMD_MD_CONF, &md_conf) < 0) {
			return SOAP_NO_DATA;
		}
	}

	if (td_sensitivity >= 0) {
		/*Get TD config*/
		if (aux_get_cc_config(AGTX_CMD_TD_CONF, &td_conf) < 0) {
			return SOAP_NO_DATA;
		}

		td_conf.enabled = (td_sensitivity > 0) ? 1 : 0;
		td_conf.sensitivity = td_sensitivity;

		/*Set TD config*/
		if (aux_set_cc_config(AGTX_CMD_TD_CONF, &td_conf) < 0) {
			return SOAP_NO_DATA;
		}
	}

	return SOAP_OK;
}
