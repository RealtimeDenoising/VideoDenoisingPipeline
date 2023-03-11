#include <gst/gst.h>

static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}

// gst-launch-1.0 filesrc location="file.wav" ! tcpclientsink host=127.0.0.1 port=5000
int main(int argc, char *argv[])
{
    GstElement *pipeline;
    GError *err = NULL;
    GstBus *bus;
    GMainLoop *loop;

    gst_init(&argc, &argv);

    loop = g_main_loop_new(NULL, FALSE);
    pipeline = gst_parse_launch("tcpserversrc host=127.0.0.1 port=5000 ! decodebin ! audioconvert ! alsasink", &err);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch (bus, bus_call, loop);
    g_main_loop_run(loop);

    return 0;
}