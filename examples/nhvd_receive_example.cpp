/*
 * NHVD Network Hardware Video Decoder example
 *
 * Copyright 2020 (C) Bartosz Meglicki <meglickib@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * Note that NHVD was written for integration with Unity
 * - it assumes that engine checks just before rendering if new frame has arrived
 * - this example simulates such behaviour be sleeping frame time
 * - for those reason NHVD may not fit your workflow
 */

#include "../nhvd.h"

#include <iostream>
#include <unistd.h> //usleep, note that this is not portable

using namespace std;

void main_loop(nhvd *network_decoder);
int process_user_input(int argc, char **argv, nhvd_hw_config *hw_config, nhvd_net_config *net_config);

//decoder configuration
const char *HARDWARE=NULL; //input through CLI, e.g. "vaapi"
const char *CODEC=NULL;  //input through CLI, e.g. "h264"
const char *DEVICE=NULL; //optionally input through CLI, e.g. "/dev/dri/renderD128"
const char *PIXEL_FORMAT="bgr0"; //NULL for default (NV12) or pixel format e.g. "rgb0"
//the pixel format that you want to receive data in
//this has to be supported by hardware
//here "bgr0" for easy integration with Unity

//network configuration
const char *IP=NULL; //listen on or NULL (listen on any)
const uint16_t PORT=9766; //to be input through CLI
const int TIMEOUT_MS=500; //timeout, accept new streaming sequence by receiver

//we simpulate application rendering at framerate
const int FRAMERATE = 30;
const int SLEEP_US = 1000000/30;

int main(int argc, char **argv)
{
	nhvd_hw_config hw_config= {HARDWARE, CODEC, DEVICE, PIXEL_FORMAT};
	nhvd_net_config net_config= {IP, PORT, TIMEOUT_MS};

	if(process_user_input(argc, argv, &hw_config, &net_config) != 0)
		return 1;

	nhvd *network_decoder = nhvd_init(&net_config, &hw_config);

	if(!network_decoder)
	{
		cerr << "failed to initalize nhvd" << endl;
		return 2;
	}

	main_loop(network_decoder);

	nhvd_close(network_decoder);
	return 0;
}

void main_loop(nhvd *network_decoder)
{
	//this is where we will get the decoded data
	nhvd_frame frame;

	bool keep_working=true;

	while(keep_working)
	{
		if( nhvd_get_frame_begin(network_decoder, &frame) == NHVD_OK )
		{
			//...
			//do something with the:
			// - frame.width
			// - frame.height
			// - frame.format
			// - frame.data
			// - frame.linesize
			// be quick - we are holding the mutex
			// Examples:
			// - fill the texture
			// - copy for later use if you can't be quick
			//...

			cout << endl << "decoded frame " << frame.width << "x" << frame.height << endl;
		}

		if( nhvd_get_frame_end(network_decoder) != NHVD_OK )
			break; //error occured

		//this should spin once per frame rendering
		//so wait until we are after rendering
		usleep(SLEEP_US);
	}
}

int process_user_input(int argc, char **argv, nhvd_hw_config *hw_config, nhvd_net_config *net_config)
{
	if(argc < 4)
	{
		fprintf(stderr, "Usage: %s <port> <hardware> <codec> [device]\n\n", argv[0]);
		fprintf(stderr, "examples: \n");
		fprintf(stderr, "%s 9766 vaapi h264 \n", argv[0]);
		fprintf(stderr, "%s 9766 vdpau h264 \n", argv[0]);
		fprintf(stderr, "%s 9766 vaapi h264 /dev/dri/renderD128\n", argv[0]);
		fprintf(stderr, "%s 9766 vaapi h264 /dev/dri/renderD129\n", argv[0]);
		fprintf(stderr, "%s 9766 dxva2 h264 \n", argv[0]);
		fprintf(stderr, "%s 9766 d3d11va h264 \n", argv[0]);
		fprintf(stderr, "%s 9766 videotoolbox h264 \n", argv[0]);
		return 1;
	}

	net_config->port = atoi(argv[1]);
	hw_config->hardware = argv[2];
	hw_config->codec = argv[3];
	hw_config->device = argv[4]; //NULL or device, both are ok

	return 0;
}