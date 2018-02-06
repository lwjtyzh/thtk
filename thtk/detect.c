/*
 * Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the
 * following conditions are met:
 *
 * 1. Redistributions of source code must retain this list
 *    of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce this
 *    list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */
#include <config.h>
#include <string.h>
#include <thtk/thtk.h>
#include <stdio.h> /* for SEEK_SET */ 
#include "thcrypt.h"
#include "thlzss.h"
#include "dattypes.h"
#define DETECT_DEF(x) \
    /* thdat02 */ \
    x(0, 1,1, NULL) \
    x(1, 2,2, NULL) \
    x(2, 3,3, NULL) \
    x(3, 3,4, NULL) \
    x(4, 3,5, NULL) \
    /* thdat06 */ \
    x(5, 6,6, NULL) \
    x(6, 7,7, "th07.dat") \
    /* thdat08 */ \
    x(7, 8,8, "th08.dat") \
    x(8, 9,9, "th09.dat") \
    /* thdat95 */ \
    x(9, 95,95, "th095.dat") \
    x(10, 95,10, "th10.dat") \
    x(11, 95,11, "th11.dat") \
    x(12, 12,12, "th12.dat") \
    x(13, 12,125, "th125.dat") \
    x(14, 12,128, "th128.dat") \
    x(15, 13,13, "th13.dat") \
    x(16, 14,14, "th14.dat") \
    x(17, 14,143, "th143.dat") \
    x(18, 14,15, "th15.dat") \
    x(19, 14,16, "th16.dat") \
    /* thdat105 */ \
    x(20, 105,105, NULL) \
    x(21, 123,123, NULL) 
    
static const thdat_detect_entry_t detect_table[] = {
#define x(idx, var, alias, filename) {var,alias,filename},
        DETECT_DEF(x)
        {0,0},       
#undef x
};
#define DETECT_ENTRIES (sizeof(detect_table)/sizeof(detect_table[0]) - 1)

static int detect_ver_to_idx(int ver) {
    switch(ver) {
    default:return -1;
#define x(idx, var, alias, filename) case alias:return idx;
    DETECT_DEF(x)
#undef x
    }
}

#define SET_OUT(x) do { \
   int macrotemp = detect_ver_to_idx(x); \
   out[macrotemp/32] |= 1 << (macrotemp%32); \
} while(0)

int
thdat_detect_filename(
        const char* filename)
{
    const thdat_detect_entry_t* ent = detect_table;
    while(ent->variant) {
        if(ent->filename && !strcmp(filename,ent->filename)) {
            return ent->alias;
        }
        ent++;
    }
    /* SJIS filenames */
    struct {
        unsigned int alias;
        const unsigned char filename[8+3+2];
    } static const multi[] = {
        {1, {0x93,0x8c,0x95,0xfb,0xe8,0xcb,0x88,0xd9,'.',0x93,0x60,0}},
        {2, {0x93,0x8c,0x95,0xfb,0x95,0x95,0x96,0x82,'.',0x98,0x5e,0}},
        {3, {0x96,0xb2,0x83,0x9e,0x8b,0xf3,'1','.','D','A','T',0}},
        {3, {0x96,0xb2,0x83,0x9e,0x8b,0xf3,'2','.','D','A','T',0}},
        {4, {0x93,0x8c,0x95,0xfb,0x8c,0xb6,0x91,0x7a,'.',0x8b,0xbd,0}},
        {4, {0x8c,0xb6,0x91,0x7a,0x8b,0xbd,'E','D','.','D','A','T',0}},
        {5, {0x89,0xf6,0xe3,0x59,0x92,0x6b,'1','.','D','A','T',0}},
        {5, {0x89,0xf6,0xe3,0x59,0x92,0x6b,'2','.','D','A','T',0}},
        {0},
    }, *mp = multi;
    while(mp->alias) {
        if(!strcmp(filename,mp->filename)) {
            return mp->alias;
        }
        mp++;
    }
    /* Unicode filenames */
    /* TODO: make this work on windows */
    struct {
        unsigned int alias;
        const unsigned char *filename;
    } static const multi2[] = {
        /* SJIS translated to Unicode */
        {1, "東方靈異.伝"},
        {2, "東方封魔.録"},
        {3, "夢時空1.DAT"},
        {3, "夢時空2.DAT"},
        {4, "東方幻想.郷"},
        {4, "幻想郷ED.DAT"},
        {5, "怪綺談1.DAT"},
        {5, "怪綺談2.DAT"},
        /* SJIS interpreted as CP1251 translated to Unicode */
        {1, "“Œ•ûèËˆÙ.“`"},
        {2, "“Œ•û••–‚.˜^"},
        {3, "–²Žž‹ó1.DAT"},
        {3, "–²Žž‹ó2.DAT"},
        {4, "“Œ•ûŒ¶‘z.‹½"},
        {4, "Œ¶‘z‹½ED.DAT"},
        {5, "‰öãY’k1.DAT"},
        {5, "‰öãY’k1.DAT"},
        {0},
    }, *mp2 = multi2;
    while(mp2->alias) {
        if(!strcmp(filename,mp2->filename)) {
            return mp2->alias;
        }
        mp2++;
    }
    
    return -1;
}

int
thdat_detect(
        const char* filename,
	thtk_io_t* input,
	uint32_t out[4],
        unsigned int *heur,
	thtk_error_t** error)
{
    out[0]=out[1]=out[2]=out[3]=0;
    *heur = -1;

    union {
        th02_entry_header_t head2[256];
        th03_entry_header_t head3[256];
    } uni;
    th02_entry_header_t *head2 = uni.head2, emptyhead2={0};
    th03_entry_header_t *head3 = uni.head3, emptyhead3={0};
    /* Note that failed header reads are fatal, and make us immediately return
     * The rationale for this is that if file can't fit a header (regardless of format)
     * then it's probably not an archive
     *
     * Entry table reads, on the other hand, are considered nonfatal */

    /* th02 */
    if(-1 == thtk_io_seek(input, 0, SEEK_SET, error)) {
        return -1;
    }
    if(-1 == thtk_io_read(input, head2, sizeof(head2[0]), error)) {
        return -1;
    }
    if(head2[0].magic != magic1 && head2[0].magic != magic2) {
        goto notth02;
    }
    if(head2[0].offset % sizeof(head2[0])) {
        goto notth02;
    }
    int entry_count = head2[0].offset/sizeof(head2[0]);
    if(entry_count > 256) { // maximum in vanilla archives is 180+1 (th03)
        goto notth02;
    }
    if(-1 == thtk_io_read(input, &head2[1], head2[0].offset-sizeof(head2[0]), error)) {
        goto notth02;
    }
    for(int i=1;i<entry_count-1;i++) {
        if(head2[i].magic != magic1 && head2[i].magic != magic2) {
            goto notth02;
        }    
    }
    if(memcmp(&head2[entry_count],&emptyhead2, sizeof(emptyhead2))) {
        goto notth02;
    }
    /* TODO: differentiate */
    SET_OUT(1);
    SET_OUT(2);
notth02:
    /* th03 */
    if(-1 == thtk_io_seek(input, 0, SEEK_SET, error)) {
        return -1;
    }
    th03_archive_header_t ahead;
    if(-1 == thtk_io_read(input, &ahead, sizeof(ahead), error)) {
        return -1;
    }
    if(++ahead.count > 256) {
        goto notth03;
    }
    if(-1 == thtk_io_read(input, &head3[0], sizeof(head3[0])*ahead.count, error)) {
        goto notth03;
    }
    unsigned char* data = (unsigned char*)head3;
    for(int i=0;i<ahead.count*sizeof(head3[0]);i++) {
        data[i] ^= ahead.key;
        ahead.key -= data[i];
    }
    for(int i=0;i<ahead.count-1;i++) {
        if(head3[i].magic != magic1 && head3[i].magic != magic2) {
            goto notth03;
        }
    }
    if(head3[ahead.count-1].magic != 0) {
        goto notth03;
    }
    SET_OUT(3);
    SET_OUT(4);
    SET_OUT(5);
notth03:

    if(-1 == thtk_io_seek(input, 0, SEEK_SET, error)) {
        return -1;
    }
    
    /* read magic for TSA 06+ */
    char magic[sizeof(th95_archive_header_t)];
    if(-1 == thtk_io_read(input, magic, sizeof(magic), error)) {
        return -1;
    }
    /* th06 */
    if(!memcmp(magic,"PBG3",4))
        SET_OUT(6);
    /* th07*/
    if(!memcmp(magic,"PBG4",4))
        SET_OUT(7);
    /* th08/th09 */
    if(!memcmp(magic,"PBGZ",4)) {
        /* TODO: differentiate */
        SET_OUT(8);
        SET_OUT(9);
    }
    /* th095+ */
    th_decrypt((unsigned char*)magic,sizeof(th95_archive_header_t),0x1b,0x37,sizeof(th95_archive_header_t),sizeof(th95_archive_header_t));
    if(!memcmp(magic,"THA1",4)) {
        SET_OUT(95);
        SET_OUT(10);
        SET_OUT(11);
        SET_OUT(12);
        SET_OUT(125);
        SET_OUT(128);
        SET_OUT(13);
        SET_OUT(14);
        SET_OUT(143);
        SET_OUT(15);
        SET_OUT(16);
    }
    
    /* heur */
    uint32_t out2[4];
    memcpy(out2,out,sizeof(out2));
    const thdat_detect_entry_t* ent;
    int fnheur = filename ? thdat_detect_filename(filename) : -1; /* detect filename */
    int variant=-1;
    int alias=-1;
    int count = 0;
    while((ent = thdat_detect_iter(out2))) {
        if(ent->alias == fnheur) {
            *heur = fnheur;
            return 0;
        }
        
        count++;
        alias = ent->alias; /* save the exact alias, if this is the only entry */
        if(variant == -1) { /* set the initial variant */
            variant = ent->variant;
        }
        else if(variant != ent->variant) { /* multiple variants -> unconclusive result */
            variant = -1;
            break;
        }
    }
    /* check the remaining entries for match with filename */
    while((ent = thdat_detect_iter(out2))) {
        if(ent->alias == fnheur) {
            *heur = fnheur;
            return 0;
        }
    }
    if(count == 1) {
        *heur = alias;
    }
    else {
        *heur = variant;
    }
    return 0;
}

const thdat_detect_entry_t*
thdat_detect_iter(
    uint32_t out[4])
{
    for(int i=0;i<4;i++) {
        if(!out[i]) continue;
        uint32_t v = out[i];
        int j=0;
        for(;!(v&1);v>>=1, j++);
        out[i] &= -1 << (j+1);
        int entry_num = i*32 + j;
        if(entry_num >= DETECT_ENTRIES) { /* non-existent entry */
            out[0]=out[1]=out[2]=out[3] = 0;
            return NULL;
        }
        return &detect_table[entry_num];
    }
    return NULL;
}