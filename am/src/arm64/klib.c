#include <am.h>
#include <amdev.h>
#include <klib.h>

/* Subset of libs/klib/src/io.c backed by the mini-virt I/O extension. */

uint32_t uptime(void)
{
    _DEV_TIMER_UPTIME_t now;

    _io_read(_DEV_TIMER, _DEVREG_TIMER_UPTIME, &now, sizeof(now));
    return now.lo;
}

void get_timeofday(void *rtc)
{
    _io_read(_DEV_TIMER, _DEVREG_TIMER_DATE, rtc, sizeof(_DEV_TIMER_DATE_t));
}

int read_key(void)
{
    _DEV_INPUT_KBD_t key;

    _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &key, sizeof(key));
    if (key.keydown) {
        return key.keycode | 0x8000;
    }
    return key.keycode;
}

void draw_rect(uint32_t *pixels, int x, int y, int w, int h)
{
    _DEV_VIDEO_FBCTRL_t control = {
        .pixels = pixels,
        .x = x,
        .y = y,
        .w = w,
        .h = h,
        .sync = 0,
    };

    _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTRL, &control, sizeof(control));
}

void draw_sync(void)
{
    _DEV_VIDEO_FBCTRL_t control = {
        .pixels = NULL,
        .sync = 1,
    };

    _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTRL, &control, sizeof(control));
}

int screen_width(void)
{
    _DEV_VIDEO_INFO_t info;

    _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
    return info.width;
}

int screen_height(void)
{
    _DEV_VIDEO_INFO_t info;

    _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
    return info.height;
}

__attribute__((noreturn)) void panic(const char *message)
{
    _putc('\n');
    while (*message != '\0') {
        _putc(*message++);
    }
    _putc('\n');
    _halt(1);
}
