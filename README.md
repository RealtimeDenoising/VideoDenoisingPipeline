# Video Denoising Pipeline

This video denoising pipeline uses state-of-the-art image and video denoising networks to obtain high-quality denoised frames with low real-time latency.

# Overview

This source code provides the implementation of [HI-GAN](https://github.com/ZeroZero19/HI-GAN) and [FastDVDNet](https://github.com/m-tassano/fastdvdnet) in a pipeline with a custom [Denoising Analyzer](https://github.com/RealtimeDenoising/DenoisingAnalyzer) and [Noise Detector](https://github.com/RealtimeDenoising/NoiseDetector) algorithms.

The GStreamer pipeline is summarized in the diagram below.
![GStreamer diagram](https://user-images.githubusercontent.com/35348652/222982713-651d7f21-81e6-465c-b7c3-d226553aab8f.png)

# Datasets

// TODO:

# Runing times

// TODO:

# Results

// TODO:

# Architecture

![decoder post processing pipeline - correct2](https://user-images.githubusercontent.com/35348652/212770047-e6b4f641-30e7-4b08-b3ed-0bc4e86ae655.png)

# Code User Guide

Install Gstreamer 

    https://gstreamer.freedesktop.org/documentation/installing/index.html?gi-language=c

To run theh pipeline

    gcc gst1.c -o gst1 `pkg-config --cflags --libs gstreamer-1.0`
