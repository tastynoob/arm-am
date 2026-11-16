#include <am.h>
#include <amdev.h>

/*
 * Minimal I/O extension for mini-virt.
 *
 * The timer is backed by CNTVCT_EL0 and the serial port by the bare-metal
 * PL011.  Everything else reports an empty device so that nexus-am code can
 * probe for it without faulting.
 */

int _ioe_init(void)
{
    return 0;
}

size_t _io_read(uint32_t dev, uintptr_t reg, void *buf, size_t size)
{
    switch (dev) {
    case _DEV_TIMER:
        if (reg == _DEVREG_TIMER_UPTIME &&
            size == sizeof(_DEV_TIMER_UPTIME_t)) {
            _DEV_TIMER_UPTIME_t *uptime = (_DEV_TIMER_UPTIME_t *)buf;
            uint64_t microseconds = am_read_time_us();

            uptime->lo = (uint32_t)microseconds;
            uptime->hi = (uint32_t)(microseconds >> 32);
            return size;
        }
        if (reg == _DEVREG_TIMER_DATE && size == sizeof(_DEV_TIMER_DATE_t)) {
            _DEV_TIMER_DATE_t *date = (_DEV_TIMER_DATE_t *)buf;

            date->year = 0;
            date->month = 1;
            date->day = 1;
            date->hour = 0;
            date->minute = 0;
            date->second = 0;
            return size;
        }
        break;
    case _DEV_SERIAL:
        if (reg == _DEVREG_SERIAL_STAT &&
            size == sizeof(_DEV_SERIAL_STAT_t)) {
            _DEV_SERIAL_STAT_t *status = (_DEV_SERIAL_STAT_t *)buf;

            status->data = 0; /* no console input */
            return size;
        }
        if (reg == _DEVREG_SERIAL_RECV &&
            size == sizeof(_DEV_SERIAL_RECV_t)) {
            _DEV_SERIAL_RECV_t *receive = (_DEV_SERIAL_RECV_t *)buf;

            receive->data = 0;
            return size;
        }
        break;
    case _DEV_INPUT:
        if (reg == _DEVREG_INPUT_KBD && size == sizeof(_DEV_INPUT_KBD_t)) {
            _DEV_INPUT_KBD_t *keyboard = (_DEV_INPUT_KBD_t *)buf;

            keyboard->keydown = 0;
            keyboard->keycode = _KEY_NONE;
            return size;
        }
        break;
    case _DEV_VIDEO:
        if (reg == _DEVREG_VIDEO_INFO && size == sizeof(_DEV_VIDEO_INFO_t)) {
            _DEV_VIDEO_INFO_t *info = (_DEV_VIDEO_INFO_t *)buf;

            info->width = 0;
            info->height = 0;
            return size;
        }
        break;
    default:
        break;
    }
    return 0;
}

size_t _io_write(uint32_t dev, uintptr_t reg, const void *buf, size_t size)
{
    if (dev == _DEV_SERIAL && reg == _DEVREG_SERIAL_SEND &&
        size == sizeof(_DEV_SERIAL_SEND_t)) {
        const _DEV_SERIAL_SEND_t *send = (const _DEV_SERIAL_SEND_t *)buf;

        am_putc((char)send->data);
        return size;
    }
    if (dev == _DEV_SERIAL && reg == _DEVREG_SERIAL_CTRL) {
        return size;
    }
    return 0;
}
