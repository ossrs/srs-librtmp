/**
# Example to use srs-librtmp
# see: https://github.com/ossrs/srs/wiki/v2_CN_SrsLibrtmp
    gcc example.c srs_librtmp.cpp -g -O0 -lstdc++ -o example
*/
#include <stdio.h>
#include "srs_librtmp.h"

int main(int argc, char** argv) 
{
    srs_rtmp_t rtmp;
    
    printf("Example for srs-librtmp\n");
    printf("SRS(ossrs) client librtmp library.\n");
    printf("version: %d.%d.%d\n", srs_version_major(), srs_version_minor(), srs_version_revision());
    
    rtmp = srs_rtmp_create("rtmp://ossrs.net/live/livestream");
    srs_human_trace("create rtmp success");
    srs_rtmp_destroy(rtmp);
    
    return 0;
}

