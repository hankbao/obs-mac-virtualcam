#include <obs-module.h>
#include <obs.hpp>
#include <pthread.h>
#include "CMIO_DPA_Sample_Server_VCamDevice.h"
#include "CMIO_DPA_Sample_Server_VCamInputStream.h"
#include "CAHostTimeBase.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("mac-virtualcam", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "macOS virtual webcam output";
}

obs_output_t *output;

static const char *virtualcam_output_get_name(void *type_data)
{
    (void)type_data;
    return obs_module_text("macOS Virtual Webcam");
}

static void *virtualcam_output_create(obs_data_t *settings, obs_output_t *output)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_create");
}

static void virtualcam_output_destroy(void *data)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_destroy");
}

// TODO(johnboiles): Janky to extern this
extern void *virtualCamMain(void *ptr);

static bool virtualcam_output_start(void *data)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_start");
    pthread_t thread1;
    pthread_create(&thread1, NULL, virtualCamMain, (void *)"Thread 1");

    // TODO(johnboiles): Right now we're hardcoded for 720x480 but that should probably change
    struct video_scale_info conversion = {};
    conversion.format = VIDEO_FORMAT_UYVY;
    conversion.width = 720;
    conversion.height = 480;
    obs_output_set_video_conversion(output, &conversion);
    if (!obs_output_begin_data_capture(output, 0)) {
        return false;
    }

    return true;
}

static void virtualcam_output_stop(void *data, uint64_t ts)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_stop");
    obs_output_end_data_capture(output);
}

// TODO(johnboiles): Janky to extern this. Make classes and stuff.
extern CMIO::DPA::Sample::Server::VCamDevice *virtualCamDevice;

static void virtualcam_output_raw_video(void *data, struct video_data *frame)
{
    uint8_t *outData = frame->data[0];
    virtualCamDevice->mInputStream->FrameArrived(virtualCamDevice->mFrameSize, outData, frame->timestamp);
}

struct obs_output_info virtualcam_output_info = {
    .id = "virtualcam_output",
    .flags = OBS_OUTPUT_VIDEO,
    .get_name = virtualcam_output_get_name,
    .create = virtualcam_output_create,
    .destroy = virtualcam_output_destroy,
    .start = virtualcam_output_start,
    .stop = virtualcam_output_stop,
    .raw_video = virtualcam_output_raw_video,
};

void start()
{
    OBSData settings;
    output = obs_output_create("virtualcam_output", "virtualcam_output", settings, NULL);
    bool started = obs_output_start(output);
    obs_data_release(settings);
}

bool obs_module_load(void)
{
    blog(LOG_DEBUG, "LOADING VIRTUALCAM");
    obs_register_output(&virtualcam_output_info);

    start();
    
    return true;
}
