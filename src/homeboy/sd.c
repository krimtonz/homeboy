#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "homeboy.h"
#include "sd.h"

static uint8_t *rw_buffer = NULL;
static const char *sdcard = "/dev/sdio/slot0";

struct _sdiorequest
{
	uint32_t cmd;
	uint32_t cmd_type;
	uint32_t rsp_type;
	uint32_t arg;
	uint32_t blk_cnt;
	uint32_t blk_size;
	void *dma_addr;
	uint32_t isdma;
	uint32_t pad0;
};
 
struct _sdioresponse
{
	uint32_t rsp_fields[3];
	uint32_t acmd12_response;
};

typedef struct _ioctlv
{
	void *data;
	uint32_t len;
} ioctlv;
 
static int32_t hId = -1;
 
static int32_t __sd0_fd = -1;
static uint16_t  __sd0_rca = 0;
static int32_t __sd0_initialized = 0;
static int32_t __sd0_sdhc = 0;
static uint8_t __sd0_cid[16];
 
static int32_t __sdio_initialized = 0;

static int32_t __sdio_sendcommand(uint32_t cmd,uint32_t cmd_type,uint32_t rsp_type,uint32_t arg,uint32_t blk_cnt,uint32_t blk_size,void *buffer,void *reply,uint32_t rlen)
{
	int32_t ret;

    ioctlv *iovec = ios_alloc(hId,sizeof(*iovec)*3,32);
    struct _sdiorequest *request = ios_alloc(hId,sizeof(*request),32);
    struct _sdioresponse *response = ios_alloc(hId,sizeof(*response),32);

	request->cmd = cmd;
	request->cmd_type = cmd_type;
	request->rsp_type = rsp_type;
	request->arg = arg;
	request->blk_cnt = blk_cnt;
	request->blk_size = blk_size;
	request->dma_addr = buffer;
	request->isdma = ((buffer!=NULL)?1:0);
	request->pad0 = 0;
 
	if(request->isdma || __sd0_sdhc == 1) {
		iovec[0].data = request;
		iovec[0].len = sizeof(struct _sdiorequest);
		iovec[1].data = buffer;
		iovec[1].len = (blk_size*blk_cnt);
		iovec[2].data = response;
		iovec[2].len = sizeof(struct _sdioresponse);
        ret = ios_ioctlv(__sd0_fd,IOCTL_SDIO_SENDCMD,2,1,iovec);
	} else
        ret = ios_ioctl(__sd0_fd,IOCTL_SDIO_SENDCMD,request,sizeof(*request),response,sizeof(*response));
 
	if(reply && !(rlen>16)) memcpy(reply,response,rlen);

    ios_free(hId,request);
    ios_free(hId,response);
    ios_free(hId,iovec);

	return ret;
}
 
static int32_t __sdio_setclock(uint32_t set)
{
	int32_t ret;

    uint32_t *clock = ios_alloc(hId,sizeof(*clock),32);

	*clock = set;
	ret = ios_ioctl(__sd0_fd,IOCTL_SDIO_SETCLK,clock,sizeof(*clock),NULL,0);
    ios_free(hId,clock);
	return ret;
}
static int32_t __sdio_getstatus(void)
{
	int32_t ret;
    uint32_t *status = ios_alloc(hId,sizeof(*status),32);
 
	ret = ios_ioctl(__sd0_fd,IOCTL_SDIO_GETSTATUS,NULL,0,status,sizeof(*status));
	if(ret<0){
        ios_free(hId,status);
        return ret;
    } 
    ret = *status;
    ios_free(hId,status);
	return ret;
}
 
static int32_t __sdio_resetcard(void)
{
	int32_t ret;
    uint32_t *status = ios_alloc(hId,sizeof(*status),32);
	__sd0_rca = 0;
	ret = ios_ioctl(__sd0_fd,IOCTL_SDIO_RESETCARD,NULL,0,status,sizeof(*status));
	if(ret<0){
        ios_free(hId,status);
        return ret;
    }
 
	__sd0_rca = (uint16_t )(*status>>16);
    ret = *status&0xFFFF;
    ios_free(hId,status);
	return (ret&0xffff);
}
 
static int32_t __sdio_gethcr(uint8_t reg, uint8_t size, uint32_t *val)
{
	int32_t ret;

	if(val==NULL) return -4;

    uint32_t *hcr_value = ios_alloc(hId,sizeof(*hcr_value),32);
    uint32_t *hcr_query = ios_alloc(hId,sizeof(*hcr_query) * 6,32);
 
 	*hcr_value = 0;
	*val = 0;
	hcr_query[0] = reg;
	hcr_query[1] = 0;
	hcr_query[2] = 0;
	hcr_query[3] = size;
	hcr_query[4] = 0;
	hcr_query[5] = 0;
	ret = ios_ioctl(__sd0_fd,IOCTL_SDIO_READHCREG,(void*)hcr_query,sizeof(*hcr_query) * 6,hcr_value,sizeof(*hcr_value));
	*val = *hcr_value;
    ios_free(hId,hcr_value);
    ios_free(hId,hcr_query);
 
	return ret;
}
 
static int32_t __sdio_sethcr(uint8_t reg, uint8_t size, uint32_t data)
{
	int32_t ret;
    uint32_t *hcr_query = ios_alloc(hId,sizeof(*hcr_query) * 6,32);
	hcr_query[0] = reg;
	hcr_query[1] = 0;
	hcr_query[2] = 0;
	hcr_query[3] = size;
	hcr_query[4] = data;
	hcr_query[5] = 0;
	ret = ios_ioctl(__sd0_fd,IOCTL_SDIO_WRITEHCREG,(void*)hcr_query,sizeof(*hcr_query)*6,NULL,0);
    ios_free(hId,hcr_query);
 
	return ret;
}

static int32_t __sdio_waithcr(uint8_t reg, uint8_t size, uint8_t unset, uint32_t mask)
{
	uint32_t val;
	int32_t ret;
	int32_t tries = 10;

	while(tries-- > 0)
	{
		ret = __sdio_gethcr(reg, size, &val);
		if(ret < 0) return ret;
		if((unset && !(val & mask)) || (!unset && (val & mask))) return 0;
		for(int i=0;i<0x2B73A840 * ((float)10000/(float)1000000);i++);
	}

	return -1;
}
 
static int32_t __sdio_setbuswidth(uint32_t bus_width)
{
	int32_t ret;
	uint32_t hc_reg = 0;
    
	ret = __sdio_gethcr(SDIOHCR_HOSTCONTROL, 1, &hc_reg);
	if(ret<0) return ret;
 
	hc_reg &= 0xff; 	
	hc_reg &= ~SDIOHCR_HOSTCONTROL_4BIT;
	if(bus_width==4) hc_reg |= SDIOHCR_HOSTCONTROL_4BIT;
 
	return __sdio_sethcr(SDIOHCR_HOSTCONTROL, 1, hc_reg);		
}

static int32_t __sd0_getrca(void)
{
	int32_t ret;
	uint32_t rca;
 
	ret = __sdio_sendcommand(SDIO_CMD_SENDRCA,0,SDIO_RESPONSE_R5,0,0,0,NULL,&rca,sizeof(rca));	
	if(ret<0) return ret;

	__sd0_rca = (uint16_t )(rca>>16);
	return (rca&0xffff);
}
 
static int32_t __sd0_select(void)
{
	int32_t ret;
 
	ret = __sdio_sendcommand(SDIO_CMD_SELECT,SDIOCMD_TYPE_AC,SDIO_RESPONSE_R1B,(__sd0_rca<<16),0,0,NULL,NULL,0);
 
	return ret;
}
 
static int32_t __sd0_deselect(void)
{
	int32_t ret;
 
	ret = __sdio_sendcommand(SDIO_CMD_DESELECT,SDIOCMD_TYPE_AC,SDIO_RESPONSE_R1B,0,0,0,NULL,NULL,0);
 
	return ret;
}
 
static int32_t __sd0_setblocklength(uint32_t blk_len)
{
	int32_t ret;
 
	ret = __sdio_sendcommand(SDIO_CMD_SETBLOCKLEN,SDIOCMD_TYPE_AC,SDIO_RESPONSE_R1,blk_len,0,0,NULL,NULL,0);
 
	return ret;
}
 
static int32_t __sd0_setbuswidth(uint32_t bus_width)
{
	uint16_t  val;
	int32_t ret;
 
	val = 0x0000;
	if(bus_width==4) val = 0x0002;
 
	ret = __sdio_sendcommand(SDIO_CMD_APPCMD,SDIOCMD_TYPE_AC,SDIO_RESPONSE_R1,(__sd0_rca<<16),0,0,NULL,NULL,0);
	if(ret<0) return ret;
 
	ret = __sdio_sendcommand(SDIO_ACMD_SETBUSWIDTH,SDIOCMD_TYPE_AC,SDIO_RESPONSE_R1,val,0,0,NULL,NULL,0);
 
	return ret;		
}

static int32_t __sd0_getcid(void)
{
	int32_t ret;
 
	ret = __sdio_sendcommand(SDIO_CMD_ALL_SENDCID,0,SDIO_RESPOSNE_R2,(__sd0_rca<<16),0,0,NULL,__sd0_cid,16);
 
	return ret;
}


static bool __sd0_initio(void)
{
	int32_t ret;
	int32_t tries;
	uint32_t status;
	struct _sdioresponse resp;


	__sdio_resetcard();
	status = __sdio_getstatus();
	
	if(!(status & SDIO_STATUS_CARD_INSERTED))
		return false;

	if(!(status & SDIO_STATUS_CARD_INITIALIZED))
	{
		// IOS doesn't like this card, so we need to convice it to accept it.

		// reopen the handle which makes IOS clean stuff up
        ios_close(__sd0_fd);
        
		__sd0_fd = ios_open(sdcard,3);

		// reset the host controller
		if(__sdio_sethcr(SDIOHCR_SOFTWARERESET, 1, 7) < 0) goto fail;
		if(__sdio_waithcr(SDIOHCR_SOFTWARERESET, 1, 1, 7) < 0) goto fail;

		// initialize interrupts (sd_reset_card does this on success)
		__sdio_sethcr(0x34, 4, 0x13f00c3);
		__sdio_sethcr(0x38, 4, 0x13f00c3);

		// enable power
		__sd0_sdhc = 1;
		ret = __sdio_sethcr(SDIOHCR_POWERCONTROL, 1, 0xe);
		if(ret < 0) goto fail;
		ret = __sdio_sethcr(SDIOHCR_POWERCONTROL, 1, 0xf);
		if(ret < 0) goto fail;

		// enable internal clock, wait until it gets stable and enable sd clock
		ret = __sdio_sethcr(SDIOHCR_CLOCKCONTROL, 2, 0);
		if(ret < 0) goto fail;
		ret = __sdio_sethcr(SDIOHCR_CLOCKCONTROL, 2, 0x101);
		if(ret < 0) goto fail;
		ret = __sdio_waithcr(SDIOHCR_CLOCKCONTROL, 2, 0, 2);
		if(ret < 0) goto fail;
		ret = __sdio_sethcr(SDIOHCR_CLOCKCONTROL, 2, 0x107);
		if(ret < 0) goto fail;

		// setup timeout
		ret = __sdio_sethcr(SDIOHCR_TIMEOUTCONTROL, 1, SDIO_DEFAULT_TIMEOUT);
		if(ret < 0) goto fail;

		// standard SDHC initialization process
		ret = __sdio_sendcommand(SDIO_CMD_GOIDLE, 0, 0, 0, 0, 0, NULL, NULL, 0);
		if(ret < 0) goto fail;
		ret = __sdio_sendcommand(SDIO_CMD_SENDIFCOND, 0, SDIO_RESPONSE_R6, 0x1aa, 0, 0, NULL, &resp, sizeof(resp));
		if(ret < 0) goto fail;
		if((resp.rsp_fields[0] & 0xff) != 0xaa) goto fail;

		tries = 10;
		while(tries-- > 0)
		{
			ret = __sdio_sendcommand(SDIO_CMD_APPCMD, SDIOCMD_TYPE_AC,SDIO_RESPONSE_R1,0,0,0,NULL,NULL,0);
			if(ret < 0) goto fail;
			ret = __sdio_sendcommand(SDIO_ACMD_SENDOPCOND, 0, SDIO_RESPONSE_R3, 0x40300000, 0, 0, NULL, &resp, sizeof(resp));
			if(ret < 0) goto fail;
			if(resp.rsp_fields[0] & (1 << 31)) break;

			for(int i=0;i<0x2B73A840 * ((float)10000/(float)1000000);i++);
		}
		if(tries < 0) goto fail;

		// FIXME: SDv2 cards which are not high-capacity won't work :/
		if(resp.rsp_fields[0] & (1 << 30))
			__sd0_sdhc = 1;
		else
			__sd0_sdhc = 0;

		ret = __sd0_getcid();
		if(ret < 0) goto fail;
		ret = __sd0_getrca();
		if(ret < 0) goto fail;
	}
	else if(status&SDIO_STATUS_CARD_SDHC)
		__sd0_sdhc = 1;
	else
		__sd0_sdhc = 0;
 
	ret = __sdio_setbuswidth(4);
	if(ret<0) return false;
 
	ret = __sdio_setclock(1);
	if(ret<0) return false;
 
	ret = __sd0_select();
	if(ret<0) return false;
 
	ret = __sd0_setblocklength(PAGE_SIZE512);
	if(ret<0) {
		ret = __sd0_deselect();
		return false;
	}
 
	ret = __sd0_setbuswidth(4);
	if(ret<0) {
		ret = __sd0_deselect();
		return false;
	}
	__sd0_deselect();

	__sd0_initialized = 1;
	return true;

	fail:
	__sdio_sethcr(SDIOHCR_SOFTWARERESET, 1, 7);
	__sdio_waithcr(SDIOHCR_SOFTWARERESET, 1, 1, 7);
	ios_close(__sd0_fd);
	__sd0_fd = ios_open(sdcard,3);
	return false;
}

static bool __sdio_deinitialize(void)
{
	if(__sd0_fd>=0)
		ios_close(__sd0_fd);
 
	__sd0_fd = -1;
	__sdio_initialized = 0;
	return true;
}

bool sdio_start(void)
{
	if(__sdio_initialized==1) return true;
 
	if(hId<0) {
		hId = ios_create_heap((void*)0x933e8000, SDIO_HEAPSIZE);
		if(hId<0) return false;
	}

	if(rw_buffer == NULL) rw_buffer = ios_alloc(hId,(4*1024),32);
	if(rw_buffer == NULL) return false;
 
	__sd0_fd = ios_open(sdcard,3);

	if(__sd0_fd<0) {
		__sdio_deinitialize();
		return false;
	}
 
	if(__sd0_initio()==false) {
		__sdio_deinitialize();
		return false;
	}
	__sdio_initialized = 1;
	return true;
}
 
bool sdio_stop(void)
{
	if(__sd0_initialized==0) return false;

	__sdio_deinitialize();
 
	__sd0_initialized = 0;
	return true;
}

bool sdio_read_sectors(uint32_t sector, uint32_t numSectors,void* buffer)
{
	int32_t ret;
	uint8_t *ptr;
	uint32_t blk_off;
 
	if(buffer==NULL) return false;
 
	ret = __sd0_select();
	if(ret<0) return false;

	if((uint32_t)buffer & 0x1F) {
		ptr = (uint8_t*)buffer;
		int secs_to_read;
		while(numSectors>0) {
			if(__sd0_sdhc == 0) blk_off = (sector*PAGE_SIZE512);
			else blk_off = sector;
			if(numSectors > 8)secs_to_read = 8;
			else secs_to_read = numSectors;
			ret = __sdio_sendcommand(SDIO_CMD_READMULTIBLOCK,SDIOCMD_TYPE_AC,SDIO_RESPONSE_R1,blk_off,secs_to_read,PAGE_SIZE512,rw_buffer,NULL,0);
			if(ret>=0) {
				memcpy(ptr,rw_buffer,PAGE_SIZE512*secs_to_read);
				ptr += PAGE_SIZE512*secs_to_read;
				sector+=secs_to_read;
				numSectors-=secs_to_read;
			} else
				break;
		}
	} else {
		if(__sd0_sdhc == 0) sector *= PAGE_SIZE512;
		ret = __sdio_sendcommand(SDIO_CMD_READMULTIBLOCK,SDIOCMD_TYPE_AC,SDIO_RESPONSE_R1,sector,numSectors,PAGE_SIZE512,buffer,NULL,0);
	}

	__sd0_deselect();
 
	return (ret>=0);
}

bool sdio_write_sectors(uint32_t sector, uint32_t numSectors,const void* buffer)
{
	int32_t ret;
	uint8_t *ptr;
	uint32_t blk_off;
 
	if(buffer==NULL) return false;
 
	ret = __sd0_select();
	if(ret<0) return false;

	if((uint32_t)buffer & 0x1F) {
		ptr = (uint8_t*)buffer;
		int secs_to_write;
		while(numSectors>0) {
			if(__sd0_sdhc == 0) blk_off = (sector*PAGE_SIZE512);
			else blk_off = sector;
			if(numSectors > 8)secs_to_write = 8;
			else secs_to_write = numSectors;
			memcpy(rw_buffer,ptr,PAGE_SIZE512*secs_to_write);
			ret = __sdio_sendcommand(SDIO_CMD_WRITEMULTIBLOCK,SDIOCMD_TYPE_AC,SDIO_RESPONSE_R1,blk_off,secs_to_write,PAGE_SIZE512,rw_buffer,NULL,0);
			if(ret>=0) {
				ptr += PAGE_SIZE512*secs_to_write;
				sector+=secs_to_write;
				numSectors-=secs_to_write;
			} else
				break;
		}
	} else {
		if(__sd0_sdhc == 0) sector *= PAGE_SIZE512;
		ret = __sdio_sendcommand(SDIO_CMD_WRITEMULTIBLOCK,SDIOCMD_TYPE_AC,SDIO_RESPONSE_R1,sector,numSectors,PAGE_SIZE512,(char *)buffer,NULL,0);
	}

	__sd0_deselect();
 
	return (ret>=0);
}

bool sdio_is_inserted(void)
{
	return ((__sdio_getstatus() & SDIO_STATUS_CARD_INSERTED) ==
			SDIO_STATUS_CARD_INSERTED);
}

bool sdio_is_initialized(void)
{
	return ((__sdio_getstatus() & SDIO_STATUS_CARD_INITIALIZED) ==
			SDIO_STATUS_CARD_INITIALIZED);
}

bool sdio_is_sdhc(void){
    return __sd0_sdhc;
}