#include <gst/gst.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData {
  GstElement *pipeline;
  GstElement *source;
  GstElement *audio_queue;
  GstElement *audio_convert;
  GstElement *audio_resample;
  GstElement *audio_sink;
  GstElement *video_queue;
  GstElement *video_convert;
  GstElement *video_filter;
  GstElement *video_sink;
} CustomData;

/* Handler for the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *pad, CustomData *data);

void read_messages(GstBus* bus, CustomData* data);

int main(int argc, char *argv[]) {
  CustomData data;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the elements */
  data.source = gst_element_factory_make("uridecodebin", "source");
  data.audio_queue = gst_element_factory_make("queue", "audio_queue");
  data.audio_convert = gst_element_factory_make ("audioconvert", "audio_convert");
  data.audio_resample = gst_element_factory_make ("audioresample", "audio_resample");
  data.audio_sink = gst_element_factory_make ("autoaudiosink", "audio_sink");
  data.video_queue = gst_element_factory_make("queue", "video_queue");
  data.video_convert = gst_element_factory_make("videoconvert", "video_convert");
  data.video_filter = gst_element_factory_make("videobalance", "video_filter");
  data.video_sink = gst_element_factory_make("autovideosink", "video_sink");

  /* Create the empty pipeline */
  data.pipeline = gst_pipeline_new ("test-pipeline");

  if (!data.pipeline || !data.source || !data.audio_queue || !data.audio_convert || !data.audio_resample
      || !data.audio_sink || !data.video_queue || !data.video_convert || !data.video_filter || !data.video_sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  /* Build the pipeline. Note that we are NOT linking the source at this
   * point. We will do it later. */
  
  // Todo : Add the Denoising components here 
  gst_bin_add_many (GST_BIN (data.pipeline), data.source, data.audio_queue,
      data.audio_convert, data.audio_resample, data.audio_sink,
      data.video_queue, data.video_convert, data.video_filter, data.video_sink, NULL);
  if (!gst_element_link_many (data.audio_queue, data.audio_convert, data.audio_resample, data.audio_sink, NULL)
      || !gst_element_link_many (data.video_queue, data.video_convert, data.video_filter, data.video_sink, NULL)) {
    g_printerr ("Audio and video elements could not be linked.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }

  /* Set the URI to play */
  g_object_set (data.source, "uri", "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);

  /* Connect to the pad-added signal */
  g_signal_connect (data.source, "pad-added", G_CALLBACK (pad_added_handler), &data);

  /* Start playing */
  ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }

  /* Listen to the bus */
  bus = gst_element_get_bus (data.pipeline);
  read_messages(bus, &data);

  /* Free resources */
  gst_object_unref (bus);
  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  gst_object_unref (data.pipeline);
  return 0;
}

/* This function will be called by the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data) {
  GstPad *queue_audio_pad = gst_element_get_static_pad (data->audio_queue, "sink");
  GstPad *queue_video_pad = gst_element_get_static_pad (data->video_queue, "sink");
  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;

  g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

  /* If our converter is already linked, we have nothing to do here */
  if (gst_pad_is_linked (queue_audio_pad) && gst_pad_is_linked (queue_video_pad)) {
    g_print ("We are already linked. Ignoring.\n");
    goto exit;
  }

  /* Check the new pad's type */
  new_pad_caps = gst_pad_get_current_caps (new_pad);
  new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
  new_pad_type = gst_structure_get_name (new_pad_struct);

  if (g_str_has_prefix(new_pad_type, "video/x-raw")) {
    /* Attemp the audio link */
      ret = gst_pad_link(new_pad, queue_video_pad);
      if (GST_PAD_LINK_FAILED(ret)) {
          g_print("Type is '%s' but link failed.\n", new_pad_type);
      }
      else {
          g_print("Link suceeded (type '%s').\n", new_pad_type);
      }
  }
  else if (g_str_has_prefix(new_pad_type, "audio/x-raw")) {
    /* Attempt the audio link */
    ret = gst_pad_link(new_pad, queue_audio_pad);
    if (GST_PAD_LINK_FAILED(ret)) {
        g_print("Type is '%s' but link failed.\n", new_pad_type);
    }
    else {
        g_print("Link succeeded (type '%s').\n", new_pad_type);
    }
  } else {
    g_print ("It has type '%s' which is not raw audio or video. Ignoring.\n",
        new_pad_type);
    goto exit;
  }

exit:
  /* Unreference the new pad's caps, if we got them */
  if (new_pad_caps != NULL)
    gst_caps_unref (new_pad_caps);

  /* Unreference the sink pad */
  gst_object_unref (queue_audio_pad);
  gst_object_unref (queue_video_pad);
}

void read_messages(GstBus* bus, CustomData* data) {
  GstMessage* msg;
  gboolean terminate = FALSE;

  do {
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
        GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    /* Parse message */
    if (msg != NULL) {
      GError* err;
      gchar* debug_info;

      switch (GST_MESSAGE_TYPE(msg)) {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error(msg, &err, &debug_info);
        g_printerr("Error received from element %s: %s\n",
            GST_OBJECT_NAME(msg->src), err->message);
        g_printerr("Debugging information: %s\n",
            debug_info ? debug_info : "none");
        g_clear_error(&err);
        g_free(debug_info);
        terminate = TRUE;
        break;
      case GST_MESSAGE_EOS:
        g_print("End-Of-Stream reached.\n");
        terminate = TRUE;
        break;
      case GST_MESSAGE_STATE_CHANGED:
        /* We are only interested in state-changed messages from the pipeline */
        if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data->pipeline)) {
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed(msg, &old_state, &new_state,
                &pending_state);
            g_print("Pipeline state changed from %s to %s:\n",
                gst_element_state_get_name(old_state),
                gst_element_state_get_name(new_state));
        }
        break;
      default:
        /* We should not reach here */
        g_printerr("Unexpected message received.\n");
        break;
      }
      gst_message_unref(msg);
    }
  } while (!terminate);
}

int main (int argc, char *argv[])
{
#if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
  return gst_macos_main (tutorial_main, argc, argv, NULL);
#else
  return tutorial_main (argc, argv);
#endif
}
