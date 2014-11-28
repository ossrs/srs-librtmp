/**
# Example to use srs-librtmp
# see: https://github.com/winlinvip/simple-rtmp-server/wiki/v2_CN_SrsLibrtmp
    gcc main.cpp srs_librtmp.cpp -g -O0 -lstdc++ -o output
*/
#include <windows.h>
#include <stdio.h>
#include "..\src\srs\srs_librtmp.h"

int main(int argc, char** argv)
{
    printf("suck rtmp stream like rtmpdump\n");
    printf("srs(simple-rtmp-server) client librtmp library.\n");
    printf("version: %d.%d.%d\n", srs_version_major(), srs_version_minor(), srs_version_revision());
    
    if (argc <= 1) {
        printf("Usage: %s <rtmp_url>\n"
            "   rtmp_url     RTMP stream url to play\n"
            "For example:\n"
            "   %s rtmp://127.0.0.1:1935/live/livestream\n"
            "   %s rtmp://ossrs.net:1935/live/livestream\n",
            argv[0], argv[0], argv[0]);
        exit(-1);
    }
	
    // startup socket for windows.
    WSADATA WSAData;
    if (WSAStartup(MAKEWORD(1, 1), &WSAData)) {
        printf("WSAStartup failed.\n");
        return -1;
    }
    
    srs_human_trace("rtmp url: %s", argv[1]);
    srs_rtmp_t rtmp = srs_rtmp_create(argv[1]);
    
    if (srs_rtmp_handshake(rtmp) != 0) {
        srs_human_trace("simple handshake failed.");
        goto rtmp_destroy;
    }
    srs_human_trace("simple handshake success");
    
    if (srs_rtmp_connect_app(rtmp) != 0) {
        srs_human_trace("connect vhost/app failed.");
        goto rtmp_destroy;
    }
    srs_human_trace("connect vhost/app success");
    
    if (srs_rtmp_play_stream(rtmp) != 0) {
        srs_human_trace("play stream failed.");
        goto rtmp_destroy;
    }
    srs_human_trace("play stream success");
    
    for (;;) {
        int size;
        char type;
        char* data;
        u_int32_t timestamp, pts;
        
        if (srs_rtmp_read_packet(rtmp, &type, &timestamp, &data, &size) != 0) {
            goto rtmp_destroy;
        }
        if (srs_utils_parse_timestamp(timestamp, type, data, size, &pts) != 0) {
            goto rtmp_destroy;
        }
        srs_human_trace("got packet: type=%s, dts=%d, pts=%d, size=%d", 
            srs_human_flv_tag_type2string(type), timestamp, pts, size);
        
        free(data);
    }
    
rtmp_destroy:
    srs_rtmp_destroy(rtmp);

    // cleanup socket for windows.
    WSACleanup();
    
    return 0;
}

