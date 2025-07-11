#ifndef __call_h__
#define __call_h__

#define CALLX(x) x
#if defined(VERBOSE)
#define USE_CALL int _iRet_
#define USE_CALLE int _iRet_
#define CALL(x)                                                                                                   \
	(_iRet_ = x, (_iRet_ ? fprintf(stderr, #x "[%s::%d] Error: 0x%X\n", __FUNCTION__, __LINE__, _iRet_) : 0), \
	 _iRet_)
#define CALL2(x, s, p...) \
	(_iRet_ = x,      \
	 _iRet_ ? fprintf(stderr, #x "[%s::%d] Error: 0x%X. " #s "\n", __FUNCTION__, __LINE__, _iRet_, p) : 0, _iRet_)
#define CALLE(x)     \
	if (CALL(x)) \
		return _iRet_;
#define CALLE2(x, s, p...)                                                                                         \
	if ((_iRet_ = x,                                                                                           \
	     _iRet_ ? fprintf(stderr, #x "[%s::%d] Error: 0x%X. " #s "\n", __FUNCTION__, __LINE__, _iRet_, p) : 0, \
	     _iRet_))                                                                                              \
		return _iRet_;
#else
#define USE_CALL
#define USE_CALLE int _iRet_
#define CALL(x) (x)
#define CALL2(x, s, p...) (x)
#define CALLE(x)            \
	if ((_iRet_ = (x))) \
		return _iRet_;
#define CALLE2(x, s, p...)  \
	if ((_iRet_ = (x))) \
		return _iRet_;
#endif

#endif
