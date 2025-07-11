#ifndef AV_LIB_H_
#define AV_LIB_H_

#include <stdbool.h>

#include "mpi_index.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*BooleanEvent)(void *context);
typedef bool (*EndTrigger)(void *context, unsigned int frame_number, float timestamp);

typedef struct av_audio_tape AvAudioTape;
typedef struct av_av_recorder AvAvRecorder;

AvAudioTape *AV_createAudioTape(unsigned int sample_rate, int gain, float audio_delay_seconds);
void AV_AudioTape_runRecording(AvAudioTape *subject, float tape_length_in_seconds, BooleanEvent stop_signal,
                               void *context);
bool AV_AudioTape_isReady(AvAudioTape *subject);
bool AV_AudioTape_isRecording(AvAudioTape *subject);
unsigned int AV_AudioTape_getSampleRate(AvAudioTape *subject);
void AV_AudioTape_dispose(AvAudioTape *subject);

AvAvRecorder *AV_createAvRecorder(MPI_ECHN encoder);
bool AV_AvRecorder_isActive(AvAvRecorder *subject);
bool AV_AvRecorder_activate(AvAvRecorder *subject);
bool AV_AvRecorder_activateWithAudio(AvAvRecorder *subject, AvAudioTape *tape);
bool AV_AvRecorder_createAudioTapeAndActivate(AvAvRecorder *subject, unsigned int sample_rate, int gain,
                                              float buffer_audio_seconds, float audio_delay_seconds);
void AV_AvRecorder_deactivate(AvAvRecorder *subject);
void AV_AvRecorder_exportToFile(AvAvRecorder *subject, const char *file_path, float start_point, bool variable_rate,
                                EndTrigger end_trigger, void *context);
void AV_AvRecorder_dispose(AvAvRecorder *subject);

#ifdef __cplusplus
}
#endif

#endif //AV_LIB_H_
