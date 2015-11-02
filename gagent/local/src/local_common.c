#include "gagent.h"
#include "utils.h"
#include "lan.h"
#include "local.h"
#include "hal_receive.h"
#include "cloud.h"

/* 从文件传输请求包中，解析出文件大小，文件名等数据 */
int parsefileinfo(pfdesc pf, ppacket pp)
{
    /* header, len, cmd, sn, flag,
       size(4b), cklen(2b), ck
     */

    u8 *p = pp->ppayload;
    int ti = 0;
    short ts = 0;
    if(pf->using)
    {
        return RET_FAILED;
    }
    ti = *(int*)p;
    ti = ntohl(ti);
    pf->totalsize = ti;
    return RET_SUCCESS;
}

/* 返回剩余包数量 */
int pushfiledata(pfdesc pf, ppacket pp)
{
    /* header, len, cmd, sn, flag,
       piececount(2b), totalcount(2b), content,
       ck(1b)
     */
    short cc = 0, ct = 0;
    u8 *p = pp->ppayload;
    pf->data = pp;
    /* 裁掉包头，piece描述段，checksum值 */
    pp->ppayload += 4;
    //pp->pend -= 1;
    if(pp->pend < pp->ppayload)
    {
        return -1;
    }

    cc = *(short*)p;
    cc = ntohs(cc);
    pf->currentpiece = cc;
    p += 2;
    ct = *(short*)p;
    ct = ntohs(ct);
    pf->piececount = ct;
    return ct - cc;
}

/* 将文件发送到对端 */
int syncfile(pgcontext pgc, pfdesc pf)
{
    int ret;
    int i = 1;
    AS;
    /* debugpacket(pf->data); */
    /* return 0; */
    /*  */
    GAgent_Printf(GAGENT_DEBUG, "  Send piece %d of %d", pf->currentpiece, pf->piececount);
    if(pf->currentpiece != 1)
    {
        switch(pf->currentpiece - pf->lastpiece)
        {
        case 0:
            /* 重复包 */
            GAgent_Printf(GAGENT_DEBUG, "SYNCFILE:repeated piece, ignore");
            ret = 1;
            break;
        case 1:
            ret = Mqtt_SendPiece(pgc, pf->data);
            if(ret == RET_FAILED)
            {
                /* 云端发送故障，终止发送 */
                return RET_FAILED;
            }
            pf->recvsize += ret;
            break;
        default:
            /* 传输错误 */
            GAgent_Printf(GAGENT_DEBUG, "SYNCFILE:wrong sequece,break transmission");
            return RET_FAILED;
            break;
        }
    }
    else
    {
        pf->data->type |= CLOUD_DATA_OUT;
        i = pf->data->pend - pf->data->ppayload;
        ret = GAgent_Cloud_SendData(pgc, pf->data, pf->data->pend - pf->data->ppayload);
        if(ret == RET_SUCCESS)
        {
            pf->recvsize += i;
        }
        else
        {
            ret = RET_FAILED;
        }
#if 0
        for(i = 2; i < pf->piececount; i++)
        {
            GAgent_Printf(GAGENT_DEBUG, "  Send piece %d of %d", i, pf->piececount);
            memset(pf->data->ppayload, i, pf->data->pend - pf->data->ppayload);
            Mqtt_SendPiece(pgc, pf->data);
        }
#endif
    }
    GAgent_Printf(GAGENT_DEBUG, "SYNCFILE:sent %d of %d bytes.", pf->recvsize, pf->totalsize);
    return ret;
}

int throwgabage(pgcontext pgc, pfdesc pf)
{
    /*  */
    int remain = pf->totalsize - pf->recvsize, ret = 0, i = 0;
    if(remain <= 0)
    {
        return 0;
    }
    if(pf->data == NULL)
    {
        return -1;
    }
    resetPacket(pf->data);

    pf->data->pend = pf->data->ppayload + 1024;
    while(remain > 0)
    {
#if 0
        memset(pf->data->ppayload, 0xa5 + i, 1024);
        GAgent_Printf(GAGENT_DEBUG, "SYNCFILE:throwgabage  %d ytes == %x", remain, 0xa5 + i);
        i++;
#endif
        if(remain < 1024)
        {
            pf->data->pend = pf->data->ppayload + remain;
        }
        ret = Mqtt_SendPiece(pgc, pf->data);
        if(ret > -1)
        {
            remain -= ret;
        }
        else
        {
            GAgent_Printf(GAGENT_DEBUG, "SYNCFILE:throwgabage send error");
            return RET_FAILED;
        }
        /* msleep(300); */
    }
    return RET_SUCCESS;
}

/* 重置文件状态 */
int resetfile(pfdesc pf)
{
    memset(pf, 0x0, sizeof(fdesc));
    return 0;
}

uint32 trans_parsemcuotainfo(pgcontext pgc, ppacket pp)
{
    int8 *p = NULL;
    memset(&pgc->tmcu, 0x00, sizeof(trans_mcuotainfo));
    p = pp->ppayload + 1;
    memcpy(pgc->tmcu.pk, p, 32);
    p += 32;
    memcpy(pgc->tmcu.did, p, 32);
    p += 32;
    memcpy(pgc->tmcu.hv, p, 8);
    p += 8;
    memcpy(pgc->tmcu.sv, p, 8);
    p += 8;
    pgc->tmcu.check = *p;

    return 0;
}

int8 *trans_parseotaurl(ppacket pp, u8 *url)
{
    int8 *p = NULL;
    int16 len = 0;

    p = pp->ppayload + 1;
    memcpy(&len, p, sizeof(len));
    len = htons(len);
    p += sizeof(len);
    memcpy(url, p, len);
    GAgent_Printf(GAGENT_DEBUG, "trans==Receive OTA url:%s", url);
    return url;
}
/* 立即执行 */
/* 失败是否重试？ */
int32 trans_dealmcutransction(pgcontext pgc, ppacket pp)
{
    u8 url[256] = {0};
    if(pgc->rtinfo.waninfo.mqttstatus != MQTT_STATUS_RUNNING)
    {
        return -1;
    }

    u8 *buf = pp->ppayload;
    switch(buf[0])
    {
    case TRANSCTION_OTA_CHECK:
        trans_parsemcuotainfo(pgc, pp);
        if(cloud_querymcuota(pgc))
        {
            /* 发送失败 */
            pgc->mcu.isBusy == 0;
            /* 回复MCU检查失败 */
            trans_sendotadownloadresult(pgc, TRANSCTION_OTA_FILE_DOWNLOAD_FAILED);
        }
        break;
    case TRANSCTION_OTA_FILE_DOWNLOAD:
        if(trans_parseotaurl(pp, url) != NULL)
        {
            trans_startmcuota(pgc, url);
        }
        else
        {

        }

        break;
    default:
        break;
    }
}

/* 发送OTA结果，由result传入：需要/不需要升级 */
int32 trans_sendotaresult(pgcontext pgc, u8 result)
{
    u8 *p = NULL;
    resetPacket(pgc->rtinfo.Txbuf);
    p = pgc->rtinfo.Txbuf->ppayload;
    *p++ = TRANSCTION_OTA_CHECK_ACK;
    if(result)
    {
        *p = TRANSCTION_OTA_CHECK_NEED_OTA;
    }
    else
    {
        *p = TRANSCTION_OTA_CHECK_DONT_NEED_OTA;
    }

    p++;
    pgc->rtinfo.Txbuf->pend = p;

    GAgent_LocalDataWriteP0(pgc, pgc->rtinfo.local.uart_fd, pgc->rtinfo.Txbuf, MCU_TRANSCTION_RESULT);
    return 0;
}

/* 发送OTA固件下载结果 */
int32 trans_sendotadownloadresult(pgcontext pgc, u8 result)
{
    u8 *p = NULL;
    resetPacket(pgc->rtinfo.Txbuf);
    p = pgc->rtinfo.Txbuf->ppayload;
    *p++ = TRANSCTION_OTA_FILE_DOWNLOAD_RESULT;
    if(result)
    {
        *p = TRANSCTION_OTA_CHECK_NEED_OTA;
    }
    else
    {
        *p = TRANSCTION_OTA_CHECK_DONT_NEED_OTA;
    }

    p++;
    pgc->rtinfo.Txbuf->pend = p;

    GAgent_LocalDataWriteP0(pgc, pgc->rtinfo.local.uart_fd, pgc->rtinfo.Txbuf, MCU_TRANSCTION_RESULT);
    return 0;
}

int32 trans_startmcuota(pgcontext pgc, u8 *url)
{
    int i = 0;
    if(RET_SUCCESS == GAgent_MCUOTAByUrl(pgc, url))
    {
        /* 下载固件成功 */
        trans_sendotadownloadresult(pgc, TRANSCTION_OTA_FILE_DOWNLOAD_SUCCESS);
        GAgent_Printf( GAGENT_CRITICAL,"GAgent Download MCU Firmware success!\n");
        //inform MCU
        resetPacket(pgc->rtinfo.Rxbuf);
        *(uint32 *)(pgc->rtinfo.Rxbuf->ppayload) = htonl(pgc->rtinfo.filelen);
        *(uint16 *)(pgc->rtinfo.Rxbuf->ppayload+4) = htons(32);
        for(i=0; i<32; i++)
        {
            pgc->rtinfo.Rxbuf->ppayload[4+2+i] = pgc->mcu.MD5[i];
        }

        pgc->rtinfo.Rxbuf->pend = (pgc->rtinfo.Rxbuf->ppayload) + 4 + 2 + 32;
        copyPacket(pgc->rtinfo.Rxbuf, pgc->rtinfo.Txbuf);
        GAgent_LocalDataWriteP0( pgc,pgc->rtinfo.local.uart_fd, pgc->rtinfo.Txbuf, MCU_NEED_UPGRADE );
    }
    else
    {
        /* 下载固件失败 */
        trans_sendotadownloadresult(pgc, TRANSCTION_OTA_FILE_DOWNLOAD_FAILED);
    }
    return 0;
}

int32 trans_sendfirmwareinfo(pgcontext pgc, u8 *sv, u8 *url)
{
    ppacket pp = pgc->rtinfo.Txbuf;
    u8 *p = pp->ppayload;
    int16 len = 0;
    *p++ = TRANSCTION_OTA_CHECK_ACK;
    memcpy(p, sv, 8);
    p += 8;
    len = strlen(url);
    len = htons(len);
    memcpy(p, &len, sizeof(len));
    len = ntohs(len);
    p += sizeof(len);
    memcpy(p, url, len);
    p += len;
    pp->pend = p;
    GAgent_LocalDataWriteP0(pgc, pgc->rtinfo.local.uart_fd, pgc->rtinfo.Txbuf, MCU_TRANSCTION_RESULT);
    return 0;
}

int32 trans_checkmcuota(pgcontext pgc, u8 *sv, u8 *url)
{
    int ret = 0;
    /* flag决定是否判断 */
    if(pgc->tmcu.check)
    {
        /* 需要判断，检测软件版本号是否一致 */
        ret = memcmp(sv, pgc->tmcu.sv, 8);
        if(ret != 0)
        {
            /* 通知MCU需要升级 */
            trans_sendotaresult(pgc, TRANSCTION_OTA_CHECK_NEED_OTA);
            /* 启动MCU固件升级过程 */
            trans_startmcuota(pgc, url);
        }
        else
        {
            /* 通知MCU不需要升级 */
            trans_sendotaresult(pgc, TRANSCTION_OTA_CHECK_DONT_NEED_OTA);
            memset(&pgc->tmcu, 0x00, sizeof(pgc->tmcu));
        }
    }
    else
    {
        /* 如果不需要检测，仅通知MCU */
        trans_sendfirmwareinfo(pgc, sv, url);
    }
}