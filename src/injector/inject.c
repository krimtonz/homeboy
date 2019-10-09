#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>

#define REVERSEENDIAN32(X)  (((X) >> 24) & 0xff) | (((X)<<8) & 0xFF0000) | (((X) >> 8) & 0xff00) | (((X)<<24) & 0xff000000)
#define REVERSEENDIAN16(X) (((X)>>8) & 0xff) | (((X)<<8) & 0xFF00)

struct dol_hdr{
    uint32_t    text_offset[7];     /* 0x000 */
    uint32_t    data_offset[11];    /* 0x01C */
    uint32_t    text_loading[7];    /* 0x048 */
    uint32_t    data_loading[11];   /* 0x064 */
    uint32_t    text_size[7];       /* 0x090 */
    uint32_t    data_size[11];      /* 0x0AC */
    uint32_t    bss_loading;        /* 0x0D8 */
    uint32_t    bss_size;           /* 0x0DC */
    uint32_t    entry;              /* 0x0E0 */
    char        padding[0x1C];      /* 0x0E4 */
};                                  /* 0x100 */

uint32_t addpadding(uint32_t inp, uint32_t padding) {
	int ret = inp;
	if (inp % padding != 0) {
		ret = inp + (padding - (inp % padding));
	}
	return ret;
}

int main(int argc, char **argv){
    FILE *input = fopen(argv[1],"rb");
    struct stat sbuffer;
    stat(argv[1],&sbuffer);
    char *data = malloc(sbuffer.st_size);
    fread(data,1,sbuffer.st_size,input);
    fclose(input);
    struct dol_hdr dol = *(struct dol_hdr*)data;
    char **text_section = calloc(1,7 * sizeof(*text_section));
    char **data_section = calloc(1,11 * sizeof(*data_section));
    int inject_pos = 0;
    for(int i=0;i<7;i++){
        if(dol.text_size[i] == 0){
            inject_pos = i;
            break;
        }
        dol.text_offset[i] = REVERSEENDIAN32(dol.text_offset[i]);
        dol.text_loading[i] = REVERSEENDIAN32(dol.text_loading[i]);
        dol.text_size[i] = REVERSEENDIAN32(dol.text_size[i]);
        text_section[i] = malloc(dol.text_size[i]);
        memcpy(text_section[i],data + dol.text_offset[i],dol.text_size[i]);
    }
    int inject_point = REVERSEENDIAN32(dol.data_offset[0]);
    for(int i=0;i<11;i++){
        if(dol.data_size[i] == 0) continue;
        dol.data_offset[i] = REVERSEENDIAN32(dol.data_offset[i]);
        dol.data_loading[i] = REVERSEENDIAN32(dol.data_loading[i]);
        dol.data_size[i] = REVERSEENDIAN32(dol.data_size[i]);
        data_section[i] = malloc(dol.data_size[i]);
        memcpy(data_section[i],data + dol.data_offset[i],dol.data_size[i]);
    }

    FILE *inject = fopen(argv[2],"rb");
    stat(argv[2],&sbuffer);
    uint32_t inject_len = addpadding(sbuffer.st_size,0x10000);
    char *injectdata = calloc(1,inject_len);
    fread(injectdata,1,sbuffer.st_size,inject);
    fclose(inject);

    text_section[inject_pos] = injectdata;

    for(int i=0;i<11;i++){
        if(dol.data_size[i]==0) continue;
        dol.data_offset[i] += inject_len;
    }

    dol.text_offset[inject_pos] = inject_point;
    dol.text_loading[inject_pos] = 0x90000800;
    dol.text_size[inject_pos] = inject_len;

    uint32_t *emu = (uint32_t*)text_section[1];
    char *heap_set = (char *)(emu + 0x20C51); // lis r4, 0x9000
    heap_set[3] = 0x10; // lis r4, 0x9010

    heap_set[8] = 0x60;
    heap_set[9] = 0x00;
    heap_set[10] = 0x00;
    heap_set[11] = 0x00;

    heap_set[20] = 0x60;
    heap_set[21] = 0x00;
    heap_set[22] = 0x00;
    heap_set[23] = 0x00;

    uint32_t *hook = (uint32_t*)(text_section[1] + 0x8C8);
    hook[0] = (0x0090803c);
    hook[1] = (0x00088438);
    hook[2] = (0xa603897c);
    hook[3] = (0x18006380);
    hook[4] = (0x2104804e);

    // assemble new dol 
    FILE *newdol = fopen(argv[3],"wb");
    for(int i=0;i<7;i++){
        dol.text_offset[i] = REVERSEENDIAN32(dol.text_offset[i]);
        dol.text_loading[i] = REVERSEENDIAN32(dol.text_loading[i]);
        dol.text_size[i] = REVERSEENDIAN32(dol.text_size[i]);
    }

    for(int i=0;i<11;i++){
        if(dol.data_size[i] == 0) continue;
        dol.data_offset[i] = REVERSEENDIAN32(dol.data_offset[i]);
        dol.data_loading[i] = REVERSEENDIAN32(dol.data_loading[i]);
        dol.data_size[i] = REVERSEENDIAN32(dol.data_size[i]);
    }

    fwrite(&dol,sizeof(dol),1,newdol);
    for(int i=0;i<7;i++){
        if(dol.text_size[i]==0) continue;
        fwrite(text_section[i],1,REVERSEENDIAN32(dol.text_size[i]),newdol);
    }
    for(int i=0;i<11;i++){
        if(dol.data_size[i]==0) continue;
        fwrite(data_section[i],1,REVERSEENDIAN32(dol.data_size[i]),newdol);
    }
    fclose(newdol);
    
}