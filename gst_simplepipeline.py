import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst

Gst.init(None)

pipeline = Gst.Pipeline()

src = Gst.ElementFactory.make("v4l2src", "src")
src.set_property("device", "/dev/video0")

sink = Gst.ElementFactory.make("xvimagesink", "sink")

pipeline.add(src)
pipeline.add(sink)
src.link(sink)

pipeline.set_state(Gst.State.PLAYING)

#  V4l2src element as the source, which captures video from a V4L2 device, in this case from the device at /dev/video0. The pipeline then uses the xvimagesink element as the sink, which displays the video in a window. The pipeline is then set to the PLAYING state to start the video capture and display.
# Note that this is a very basic pipeline and you may need to add additional elements to handle video format conversion, scaling, etc.