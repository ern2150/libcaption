/**********************************************************************************************/
/* Copyright 2016-2016 Twitch Interactive, Inc. or its affiliates. All Rights Reserved.       */
/*                                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"). You may not use this file  */
/* except in compliance with the License. A copy of the License is located at                 */
/*                                                                                            */
/*     http://aws.amazon.com/apache2.0/                                                       */
/*                                                                                            */
/* or in the "license" file accompanying this file. This file is distributed on an "AS IS"    */
/* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the    */
/* License for the specific language governing permissions and limitations under the License. */
/**********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "srt.h"
#include "scc.h"

#define MAX_SCC_SIZE (10*1024*1024)
#define MAX_READ_SIZE 4096
#define MAX_CC 128

size_t read_file (FILE* file, utf8_char_t* data, size_t size)
{
    size_t read, totl = 0;

    while (0 < (read = fread (data,1,MAX_READ_SIZE<size?MAX_READ_SIZE:size,file))) {
        totl += read; data += read; size -= read;
    }

    return totl;
}

srt_t* scc2srt (const char* data)
{
    double pts;
    size_t line_size = 0;
    int cc_idx, count, i;
    srt_t* srt = 0, *head = 0;
    caption_frame_t frame;
    uint16_t cc_data[MAX_CC];

    while (0 < (line_size = utf8_line_length (data))) {
        caption_frame_init (&frame);
        int cc_count = scc_to_608 (data, &pts, (uint16_t*) &cc_data, MAX_CC);
        data += line_size;
        data += utf8_line_length (data); // skip empty line

        // fprintf (stderr,"%f, %d| %.*s\n", pts, cc_count, (int) line_size,data);

        for (cc_idx = 0 ; cc_idx < cc_count ; ++cc_idx) {
            // eia608_dump (cc_data[cc_idx]);
            caption_frame_decode (&frame,cc_data[cc_idx],pts);
        }

        // utf8_char_t buff[CAPTION_FRAME_DUMP_BUF_SIZE];
        // size_t size = caption_frame_dump (&frame, buff);
        // fprintf (stderr,"%s\n", buff);
        srt = srt_from_caption_frame (&frame,srt);

        if (!head) {head = srt;}
    }

    return head;
}

int main (int argc, char** argv)
{
    char frame_buf[CAPTION_FRAME_DUMP_BUF_SIZE];

    if (argc < 2) {
        return 0;
    }

    FILE* file = fopen (argv[1],"r");

    if (! file) {
        return 0;
    }

    utf8_char_t* data = malloc (MAX_SCC_SIZE);
    read_file (file,data,MAX_SCC_SIZE);
    srt_t* srt = scc2srt (data);
    srt_dump (srt);
    srt_free (srt);
    free (data);
}
