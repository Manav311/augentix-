/** @file
 * @brief 多用户帧缓冲区导出接口
 *
 * SDK的云存储模块在内部维护了一个循环缓冲区，用于保存要上传的音视频数据。 \n
 * 当缓冲区満时，最老的数据帧会被丢弃。
 *
 * 应用层可以复用这个缓冲机制，用于SD录像、网络传输等。
 */
#ifndef __TgCloudApi_mufb_h__
#define __TgCloudApi_mufb_h__

#ifdef __cplusplus
extern "C" {
#endif

//#include <sys/uio.h>
#include "platforms.h"

/** 帧头 */
typedef
struct MUFHEADER {
	unsigned short type;	///< media type
	unsigned short flags;	///< 1 - sync point, key frame
	unsigned int ts;	///< timestamp, used by user
	unsigned int len;	///< length of data(exclude header)
} MUFHEADER;

struct mufbuffer;

/** mufb client */
typedef
struct MUFBCLT {
    struct mufbuffer *fb;
    int clt_id;
} MUFBCLT;

/** 初始化一个缓冲区用户
 * @return 0: ok; -1: 云存储还没有初始化
 */
int TcfbClientInit(MUFBCLT *clt);

/** 删除缓冲区用户
 *  \param clt  TcfbClientInit()初始化过的MUFBCLT指针
 * @return: 0 - ok; -1 - 参数无效
 */
int TcfbClientDestroy(MUFBCLT *clt);

/* 调用顺序
    TfcbFetchPreKeyFrame() -> TfcbReleaseFrame() -> { TfcbFetchFrame() -> TfcbReleaseFrame() }*
*/

/** \brief 取最近的关键帧(用于预录)
 *  使用完后调用 TcfbReleaseFrame() 释放
 *  \return NULL 或录像开始的关键帧
 */
MUFHEADER *TcfbFetchPreKeyFrame(MUFBCLT *clt);


/** \brief 获取下一帧
 *  使用完后调用mufb_release_frame()释放
 *  \param clt MUFBCLT 指针
 *  \param b_overwritten when returns, indicates whether some frames are discarded
 */
MUFHEADER *TcfbFetchFrame(MUFBCLT *clt, int *b_overwritten);

/** \brief 获取帧数据指针
 *  \param clt
 *  \param pfh pointer to frame header
 *  \param vec space to receive the pointer(s) of the frame
 *  \return number of iovec filled in vec
 */
int TcfbGetFrameDataPtr(const MUFBCLT *clt, const MUFHEADER *pfh, struct iovec vec[2]);

/** \brief 释放 Fetch 操作获取的数据帧, 并且内部指针前进到下一帧(下一次 TcfbFetchFrame() 返回新的帧)
 *  \param clt
 *  \param pfh fetch操作返回的帧指针
 *  \return always 0
 */
int TcfbReleaseFrame(MUFBCLT *clt, struct MUFHEADER *pfh);


#ifdef __cplusplus
}
#endif
#endif

