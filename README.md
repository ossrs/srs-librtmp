srs.librtmp
===========

The client library srs-librtmp of SRS(https://github.com/ossrs/srs)

This repository is exported by SRS3.0+, please read the wiki(
[CN](https://github.com/ossrs/srs/wiki/v3_CN_SrsLibrtmp#export-srs-librtmp),
[EN](https://github.com/ossrs/srs/wiki/v3_EN_SrsLibrtmp#export-srs-librtmp)
).

When you program on linux, the exported project or files can be direclty used.

For windows, I provides a visual studio project example.

We also patch the [librtmp](https://github.com/ossrs/librtmp) to work better with SRS,
to get the debug srs ip/pid/cid, which is very important to fast debug.

## RTMP

The API to publish or play RTMP stream.

1. Create RTMP object:

    ```
    /**
     * create/destroy a rtmp protocol stack.
     * @url rtmp url, for example:
     *         rtmp://localhost/live/livestream
     * @remark default timeout to 30s if not set by srs_rtmp_set_timeout.
     *
     * @return a rtmp handler, or NULL if error occured.
     */
    extern srs_rtmp_t srs_rtmp_create(const char* url);
    ```

1. Handshake with server:

    ```
    /**
    * connect and handshake with server
    * category: publish/play
    * previous: rtmp-create
    * next: connect-app
    *
    * @return 0, success; otherswise, failed.
    */
    extern int srs_rtmp_handshake(srs_rtmp_t rtmp);
    ```

1. Setup the RTMP object:
    ```
    /**
    * Set the schema of URL when connect to tcUrl by srs_rtmp_connect_app.
     * @param schema, The schema of URL, @see srs_url_schema.
     * @return 0, success; otherswise, failed.
     */
    extern int srs_rtmp_set_schema(srs_rtmp_t rtmp, enum srs_url_schema schema);
    ```

1. Connect to RTMP app:

    ```
    /**
    * connect to rtmp vhost/app
    * category: publish/play
    * previous: handshake
    * next: publish or play
    *
    * @return 0, success; otherswise, failed.
    */
    extern int srs_rtmp_connect_app(srs_rtmp_t rtmp);
    ```

1. Retrieve server informations:
    ```
    /**
     * Retrieve server ip from RTMP handler.
     * @Param ip A NULL terminated string specifies the server ip.
     * @Param pid An int specifies the PID of server. -1 is no PID information.
     * @Param cid An int specifies the CID of connection. -1 is no CID information.
     * @remark For SRS, ip/pid/cid is the UUID of a client. For other server, these values maybe unknown.
     * @remark When connected to server by srs_rtmp_connect_app, the information is ready to be retrieved.
     * @return 0, success; otherswise, failed.
     */
    extern int srs_rtmp_get_server_id(srs_rtmp_t rtmp, char** ip, int* pid, int* cid);

    /**
     * Retrieve server signature from RTMP handler.
     * @Param sig A NULL terminated string specifies the server signature.
     * @remark When connected to server by srs_rtmp_connect_app, the information is ready to be retrieved.
     * @return 0, success; otherswise, failed.
     */
    extern int srs_rtmp_get_server_sig(srs_rtmp_t rtmp, char** sig);

    /**
     * Retrieve server version from RTMP handler, which in major.minor.revision.build format.
     * @remark When connected to server by srs_rtmp_connect_app, the information is ready to be retrieved.
     * @return 0, success; otherswise, failed.
     */
    extern int srs_rtmp_get_server_version(srs_rtmp_t rtmp, int* major, int* minor, int* revision, int* build);
    ```

1. Start play or publish stream:
    ```
    /**
    * play a live/vod stream.
    * category: play
    * previous: connect-app
    * next: destroy
    * @return 0, success; otherwise, failed.
    */
    extern int srs_rtmp_play_stream(srs_rtmp_t rtmp);

    /**
    * publish a live stream.
    * category: publish
    * previous: connect-app
    * next: destroy
    * @return 0, success; otherwise, failed.
    */
    extern int srs_rtmp_publish_stream(srs_rtmp_t rtmp);
    ```

1. Send or receive RTMP packet:
    ```
    /**
    * E.4.1 FLV Tag, page 75
    */
    // 8 = audio
    #define SRS_RTMP_TYPE_AUDIO 8
    // 9 = video
    #define SRS_RTMP_TYPE_VIDEO 9
    // 18 = script data
    #define SRS_RTMP_TYPE_SCRIPT 18
    /**
    * read a audio/video/script-data packet from rtmp stream.
    * @param type, output the packet type, macros:
    *            SRS_RTMP_TYPE_AUDIO, FlvTagAudio
    *            SRS_RTMP_TYPE_VIDEO, FlvTagVideo
    *            SRS_RTMP_TYPE_SCRIPT, FlvTagScript
    *            otherswise, invalid type.
    * @param timestamp, in ms, overflow in 50days
    * @param data, the packet data, according to type:
    *             FlvTagAudio, @see "E.4.2.1 AUDIODATA"
    *            FlvTagVideo, @see "E.4.3.1 VIDEODATA"
    *            FlvTagScript, @see "E.4.4.1 SCRIPTDATA"
    *            User can free the packet by srs_rtmp_free_packet.
    * @param size, size of packet.
    * @return the error code. 0 for success; otherwise, error.
    *
    * @remark: for read, user must free the data.
    * @remark: for write, user should never free the data, even if error.
    * @example /trunk/research/librtmp/srs_play.c
    * @example /trunk/research/librtmp/srs_publish.c
    *
    * @return 0, success; otherswise, failed.
    */
    extern int srs_rtmp_read_packet(srs_rtmp_t rtmp,
        char* type, uint32_t* timestamp, char** data, int* size
    );
    extern int srs_rtmp_write_packet(srs_rtmp_t rtmp,
        char type, uint32_t timestamp, char* data, int size
    );
    ```

1. Destroy RTMP object:

    ```
    /**
    * close and destroy the rtmp stack.
    * @remark, user should never use the rtmp again.
    */
    extern void srs_rtmp_destroy(srs_rtmp_t rtmp);
    ```

## RAW AAC Data

The API to send RAW AAC frame over RTMP.

1. Determine whether frame is in ADTS format:

    ```
    /**
    * whether aac raw data is in adts format,
    * which bytes sequence matches '1111 1111 1111'B, that is 0xFFF.
    * @param aac_raw_data the input aac raw data, a encoded aac frame data.
    * @param ac_raw_size the size of aac raw data.
    *
    * @reamrk used to check whether current frame is in adts format.
    *       @see ISO_IEC_14496-3-AAC-2001.pdf, page 75, 1.A.2.2 ADTS
    * @example /trunk/research/librtmp/srs_aac_raw_publish.c
    *
    * @return 0 false; otherwise, true.
    */
    extern srs_bool srs_aac_is_adts(char* aac_raw_data, int ac_raw_size);
    ```

1. Parse the RAW AAC ADTS frame, get the size of AAC data:

    ```
    /**
    * parse the adts header to get the frame size,
    * which bytes sequence matches '1111 1111 1111'B, that is 0xFFF.
    * @param aac_raw_data the input aac raw data, a encoded aac frame data.
    * @param ac_raw_size the size of aac raw data.
    *
    * @return failed when <=0 failed; otherwise, ok.
    */
    extern int srs_aac_adts_frame_size(char* aac_raw_data, int ac_raw_size);
    ```

1. Write the RAW AAC frame over RTMP:

    ```
    /**
    * write an audio raw frame to srs.
    * not similar to h.264 video, the audio never aggregated, always
    * encoded one frame by one, so this api is used to write a frame.
    *
    * @param sound_format Format of SoundData. The following values are defined:
    *               0 = Linear PCM, platform endian
    *               1 = ADPCM
    *               2 = MP3
    *               3 = Linear PCM, little endian
    *               4 = Nellymoser 16 kHz mono
    *               5 = Nellymoser 8 kHz mono
    *               6 = Nellymoser
    *               7 = G.711 A-law logarithmic PCM
    *               8 = G.711 mu-law logarithmic PCM
    *               9 = reserved
    *               10 = AAC
    *               11 = Speex
    *               14 = MP3 8 kHz
    *               15 = Device-specific sound
    *               Formats 7, 8, 14, and 15 are reserved.
    *               AAC is supported in Flash Player 9,0,115,0 and higher.
    *               Speex is supported in Flash Player 10 and higher.
    * @param sound_rate Sampling rate. The following values are defined:
    *               0 = 5.5 kHz
    *               1 = 11 kHz
    *               2 = 22 kHz
    *               3 = 44 kHz
    * @param sound_size Size of each audio sample. This parameter only pertains to
    *               uncompressed formats. Compressed formats always decode
    *               to 16 bits internally.
    *               0 = 8-bit samples
    *               1 = 16-bit samples
    * @param sound_type Mono or stereo sound
    *               0 = Mono sound
    *               1 = Stereo sound
    * @param timestamp The timestamp of audio.
    *
    * @example /trunk/research/librtmp/srs_aac_raw_publish.c
    * @example /trunk/research/librtmp/srs_audio_raw_publish.c
    *
    * @remark for aac, the frame must be in ADTS format.
    *       @see ISO_IEC_14496-3-AAC-2001.pdf, page 75, 1.A.2.2 ADTS
    * @remark for aac, only support profile 1-4, AAC main/LC/SSR/LTP,
    *       @see ISO_IEC_14496-3-AAC-2001.pdf, page 23, 1.5.1.1 Audio object type
    *
    * @see https://github.com/ossrs/srs/issues/212
    * @see E.4.2.1 AUDIODATA of video_file_format_spec_v10_1.pdf
    *
    * @return 0, success; otherswise, failed.
    */
    extern int srs_audio_write_raw_frame(srs_rtmp_t rtmp,
        char sound_format, char sound_rate, char sound_size, char sound_type,
        char* frame, int frame_size, uint32_t timestamp
    );
    ```

# RAW H.264 Data

The API to send RAW H.264 frame over RTMP.

1. Check whether RAW H.264 is in annexb format:

    ```
    /**
    * whether h264 raw data starts with the annexb,
    * which bytes sequence matches N[00] 00 00 01, where N>=0.
    * @param h264_raw_data the input h264 raw data, a encoded h.264 I/P/B frame data.
    * @paam h264_raw_size the size of h264 raw data.
    * @param pnb_start_code output the size of start code, must >=3.
    *       NULL to ignore.
    *
    * @reamrk used to check whether current frame is in annexb format.
    * @example /trunk/research/librtmp/srs_h264_raw_publish.c
    *
    * @return 0 false; otherwise, true.
    */
    extern srs_bool srs_h264_startswith_annexb(
        char* h264_raw_data, int h264_raw_size,
        int* pnb_start_code
    );
    ```

1. Write H.264 RAW frame over RTMP:

    ```
    /**
    * write h.264 raw frame over RTMP to rtmp server.
    * @param frames the input h264 raw data, encoded h.264 I/P/B frames data.
    *       frames can be one or more than one frame,
    *       each frame prefixed h.264 annexb header, by N[00] 00 00 01, where N>=0,
    *       for instance, frame = header(00 00 00 01) + payload(67 42 80 29 95 A0 14 01 6E 40)
    *       about annexb, @see ISO_IEC_14496-10-AVC-2003.pdf, page 211.
    * @param frames_size the size of h264 raw data.
    *       assert frames_size > 0, at least has 1 bytes header.
    * @param dts the dts of h.264 raw data.
    * @param pts the pts of h.264 raw data.
    *
    * @remark, user should free the frames.
    * @remark, the tbn of dts/pts is 1/1000 for RTMP, that is, in ms.
    * @remark, cts = pts - dts
    * @remark, use srs_h264_startswith_annexb to check whether frame is annexb format.
    * @example /trunk/research/librtmp/srs_h264_raw_publish.c
    * @see https://github.com/ossrs/srs/issues/66
    *
    * @return 0, success; otherswise, failed.
    *       for dvbsp error, @see srs_h264_is_dvbsp_error().
    *       for duplictated sps error, @see srs_h264_is_duplicated_sps_error().
    *       for duplictated pps error, @see srs_h264_is_duplicated_pps_error().
    */
    /**
    For the example file:
        http://winlinvip.github.io/srs.release/3rdparty/720p.h264.raw
    The data sequence is:
        // SPS
        000000016742802995A014016E40
        // PPS
        0000000168CE3880
        // IFrame
        0000000165B8041014C038008B0D0D3A071.....
        // PFrame
        0000000141E02041F8CDDC562BBDEFAD2F.....
    User can send the SPS+PPS, then each frame:
        // SPS+PPS
        srs_h264_write_raw_frames('000000016742802995A014016E400000000168CE3880', size, dts, pts)
        // IFrame
        srs_h264_write_raw_frames('0000000165B8041014C038008B0D0D3A071......', size, dts, pts)
        // PFrame
        srs_h264_write_raw_frames('0000000141E02041F8CDDC562BBDEFAD2F......', size, dts, pts)
    User also can send one by one:
        // SPS
        srs_h264_write_raw_frames('000000016742802995A014016E4', size, dts, pts)
        // PPS
        srs_h264_write_raw_frames('00000000168CE3880', size, dts, pts)
        // IFrame
        srs_h264_write_raw_frames('0000000165B8041014C038008B0D0D3A071......', size, dts, pts)
        // PFrame
        srs_h264_write_raw_frames('0000000141E02041F8CDDC562BBDEFAD2F......', size, dts, pts)
    */
    extern int srs_h264_write_raw_frames(srs_rtmp_t rtmp,
        char* frames, int frames_size, uint32_t dts, uint32_t pts
    );
    ```

## MP4 File Format

The API to read MP4 format.

1. Open MP4 file:

    ```
    /* Open mp4 file for muxer(write) or demuxer(read). */
    extern srs_mp4_t srs_mp4_open_read(const char* file);
    ```

1. Initialize the MP4 demuxer:

    ```
    /**
     * Initialize mp4 demuxer in non-seek mode.
     * @remark Only support non-seek mode, that is fmp4 or moov before mdata.
     *      For the live streaming, we must feed stream frame by frame.
     */
    extern int srs_mp4_init_demuxer(srs_mp4_t mp4);
    ```

1. Read next sample(header and frame payload):
    ```
    /**
     * Read a sample form mp4.
     * @remark User can use srs_mp4_sample_to_flv_tag to convert mp4 sampel to flv tag.
     *      Use the srs_mp4_to_flv_tag_size to calc the flv tag data size to alloc.
     */
    extern int srs_mp4_read_sample(srs_mp4_t mp4, srs_mp4_sample_t* sample);
    ```
> Remark: Use `srs_mp4_is_eof` to check whether EOF.

1. Calculate the size of FLV tag, converted from MP4 sample:

    ```
    /**
     * Calc the size of flv tag, for the mp4 sample to convert to.
     */
    extern int32_t srs_mp4_sizeof(srs_mp4_t mp4, srs_mp4_sample_t* sample);
    ```

1. Convert MP4 sample to FLV tag:
    ```
    /**
     * Covert mp4 sample to flv tag.
     */
    extern int srs_mp4_to_flv_tag(srs_mp4_t mp4, srs_mp4_sample_t* sample, char* type, uint32_t* time, char* data, int32_t size);
    ```
> Remark: User must allocate the tag payload.
> Reamrk: User can send the FLV tag over RTMP.

1. Free the MP4 sample:

    ```
    /**
     * Free the allocated mp4 sample.
     */
    extern void srs_mp4_free_sample(srs_mp4_sample_t* sample);
    ```

1. Destroy MP4 file:

    ```
    extern void srs_mp4_close(srs_mp4_t mp4);
    ```


## FLV File Format

The API to read or write FLV file.

1. Open the FLV file, in read or write mode:

    ```
    /* open flv file for both read/write. */
    extern srs_flv_t srs_flv_open_read(const char* file);
    extern srs_flv_t srs_flv_open_write(const char* file);
    ```

1. Read the FLV file header from file:
    ```
    /**
    * read the flv header. 9bytes header.
    * @param header, @see E.2 The FLV header, flv_v10_1.pdf in SRS doc.
    *   3bytes, signature, "FLV",
    *   1bytes, version, 0x01,
    *   1bytes, flags, UB[5] 0, UB[1] audio present, UB[1] 0, UB[1] video present.
    *   4bytes, dataoffset, 0x09, The length of this header in bytes
    *
    * @return 0, success; otherswise, failed.
    * @remark, drop the 4bytes zero previous tag size.
    */
    extern int srs_flv_read_header(srs_flv_t flv, char header[9]);
    ```
> Reamark: Use `srs_flv_is_eof` to check whether EOF.

1. Read a FLV tag from file:
    ```
    /**
    * read the flv tag header, 1bytes tag, 3bytes data_size,
    * 4bytes time, 3bytes stream id.
    * @param ptype, output the type of tag, macros:
    *            SRS_RTMP_TYPE_AUDIO, FlvTagAudio
    *            SRS_RTMP_TYPE_VIDEO, FlvTagVideo
    *            SRS_RTMP_TYPE_SCRIPT, FlvTagScript
    * @param pdata_size, output the size of tag data.
    * @param ptime, output the time of tag, the dts in ms.
    *
    * @return 0, success; otherswise, failed.
    * @remark, user must ensure the next is a tag, srs never check it.
    */
    extern int srs_flv_read_tag_header(srs_flv_t flv,
        char* ptype, int32_t* pdata_size, uint32_t* ptime
    );
    ```
> Reamark: Use `srs_flv_is_eof` to check whether EOF.

1. Read a FLV tag body from file:
```
    /**
    * read the tag data. drop the 4bytes previous tag size
    * @param data, the data to read, user alloc and free it.
    * @param size, the size of data to read, get by srs_flv_read_tag_header().
    * @remark, srs will ignore and drop the 4bytes previous tag size.
    */
    extern int srs_flv_read_tag_data(srs_flv_t flv, char* data, int32_t size);
```
> Reamark: Use `srs_flv_is_eof` to check whether EOF.

1. Write the FLV file header to file:
    ```
    /**
    * write the flv header. 9bytes header.
    * @param header, @see E.2 The FLV header, flv_v10_1.pdf in SRS doc.
    *   3bytes, signature, "FLV",
    *   1bytes, version, 0x01,
    *   1bytes, flags, UB[5] 0, UB[1] audio present, UB[1] 0, UB[1] video present.
    *   4bytes, dataoffset, 0x09, The length of this header in bytes
    *
    * @return 0, success; otherswise, failed.
    * @remark, auto write the 4bytes zero previous tag size.
    */
    extern int srs_flv_write_header(srs_flv_t flv, char header[9]);
    ```

1. Write a FLV tag header and body to file:
    ```
    /**
    * write the flv tag to file.
    *
    * @return 0, success; otherswise, failed.
    * @remark, auto write the 4bytes zero previous tag size.
    */
    /* write flv tag to file, auto write the 4bytes previous tag size */
    extern int srs_flv_write_tag(srs_flv_t flv,
        char type, int32_t time, char* data, int size
    );
    ```

1. Close the FLV file:
    ```
    extern void srs_flv_close(srs_flv_t flv);
    ```

## AMF0 Format

The following APIs are AMF0 codec.

1. Number defined as double:
```
typedef double srs_amf0_number;
```

1. Create AMF0 instance:
```
extern srs_amf0_t srs_amf0_create_string(const char* value);
extern srs_amf0_t srs_amf0_create_number(srs_amf0_number value);
extern srs_amf0_t srs_amf0_create_ecma_array();
extern srs_amf0_t srs_amf0_create_strict_array();
extern srs_amf0_t srs_amf0_create_object();
```

1. Parse AMF0 instance from buffer:
```
extern srs_amf0_t srs_amf0_parse(char* data, int size, int* nparsed);
```

1. Encode AMF0 instance to buffer:
```
extern int srs_amf0_size(srs_amf0_t amf0);
extern int srs_amf0_serialize(srs_amf0_t amf0, char* data, int size);
```

1. Detect the type of AMF0 instance:
```
extern srs_bool srs_amf0_is_string(srs_amf0_t amf0);
extern srs_bool srs_amf0_is_boolean(srs_amf0_t amf0);
extern srs_bool srs_amf0_is_number(srs_amf0_t amf0);
extern srs_bool srs_amf0_is_null(srs_amf0_t amf0);
extern srs_bool srs_amf0_is_object(srs_amf0_t amf0);
extern srs_bool srs_amf0_is_ecma_array(srs_amf0_t amf0);
extern srs_bool srs_amf0_is_strict_array(srs_amf0_t amf0);
```

1. Convert the AMF0 instance to basic type:
```
extern const char* srs_amf0_to_string(srs_amf0_t amf0);
extern srs_bool srs_amf0_to_boolean(srs_amf0_t amf0);
extern srs_amf0_number srs_amf0_to_number(srs_amf0_t amf0);
```

1. Set the AMF0 Number:
```
extern void srs_amf0_set_number(srs_amf0_t amf0, srs_amf0_number value);
```

1. For AMF0 Object instance:
```
extern int srs_amf0_object_property_count(srs_amf0_t amf0);
extern const char* srs_amf0_object_property_name_at(srs_amf0_t amf0, int index);
extern srs_amf0_t srs_amf0_object_property_value_at(srs_amf0_t amf0, int index);
extern srs_amf0_t srs_amf0_object_property(srs_amf0_t amf0, const char* name);
extern void srs_amf0_object_property_set(srs_amf0_t amf0, const char* name, srs_amf0_t value);
extern void srs_amf0_object_clear(srs_amf0_t amf0);
```

1. For AMF0 EcmaArray instance:
```
extern int srs_amf0_ecma_array_property_count(srs_amf0_t amf0);
extern const char* srs_amf0_ecma_array_property_name_at(srs_amf0_t amf0, int index);
extern srs_amf0_t srs_amf0_ecma_array_property_value_at(srs_amf0_t amf0, int index);
extern srs_amf0_t srs_amf0_ecma_array_property(srs_amf0_t amf0, const char* name);
extern void srs_amf0_ecma_array_property_set(srs_amf0_t amf0, const char* name, srs_amf0_t value);
```

1. For AMF0 StrictArray instance:
```
extern int srs_amf0_strict_array_property_count(srs_amf0_t amf0);
extern srs_amf0_t srs_amf0_strict_array_property_at(srs_amf0_t amf0, int index);
extern void srs_amf0_strict_array_append(srs_amf0_t amf0, srs_amf0_t value);
```

1. Destroy AMF0 instance:
```
extern void srs_amf0_free(srs_amf0_t amf0);
```

Winlin 2014.11
