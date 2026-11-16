#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>

#include <baremetal/platform.h>

extern char __heap_start[];
extern char __heap_end[];

static char *current_break = __heap_start;

ssize_t _write(int fd, const void *buffer, size_t length)
{
    const char *bytes = buffer;

    if (fd != STDOUT_FILENO && fd != STDERR_FILENO) {
        errno = EBADF;
        return -1;
    }
    for (size_t index = 0; index < length; ++index) {
        baremetal_platform_putc(bytes[index]);
    }
    return (ssize_t)length;
}

void *_sbrk(ptrdiff_t increment)
{
    char *previous = current_break;
    uintptr_t next;

    if (increment < 0) {
        errno = ENOMEM;
        return (void *)-1;
    }
    next = ((uintptr_t)current_break + (uintptr_t)increment + 15U) &
           ~(uintptr_t)15U;
    if (next > (uintptr_t)__heap_end) {
        errno = ENOMEM;
        return (void *)-1;
    }
    current_break = (char *)next;
    return previous;
}

int _close(int fd)
{
    (void)fd;
    errno = EBADF;
    return -1;
}

int _fstat(int fd, struct stat *st)
{
    if (fd != STDOUT_FILENO && fd != STDERR_FILENO) {
        errno = EBADF;
        return -1;
    }
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int fd)
{
    return fd == STDOUT_FILENO || fd == STDERR_FILENO;
}

off_t _lseek(int fd, off_t offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    errno = ESPIPE;
    return (off_t)-1;
}

ssize_t _read(int fd, void *buffer, size_t length)
{
    (void)fd;
    (void)buffer;
    (void)length;
    errno = ENOSYS;
    return -1;
}

int _gettimeofday(struct timeval *time_value, void *timezone)
{
    (void)time_value;
    (void)timezone;
    errno = ENOSYS;
    return -1;
}

clock_t _times(struct tms *buffer)
{
    (void)buffer;
    errno = ENOSYS;
    return (clock_t)-1;
}

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int signal)
{
    (void)pid;
    (void)signal;
    errno = EINVAL;
    return -1;
}

int _link(const char *old_path, const char *new_path)
{
    (void)old_path;
    (void)new_path;
    errno = ENOSYS;
    return -1;
}

int _open(const char *path, int flags, ...)
{
    (void)path;
    (void)flags;
    errno = ENOSYS;
    return -1;
}

int _stat(const char *path, struct stat *st)
{
    (void)path;
    (void)st;
    errno = ENOSYS;
    return -1;
}

int _unlink(const char *path)
{
    (void)path;
    errno = ENOSYS;
    return -1;
}

int chdir(const char *path)
{
    (void)path;
    return 0;
}

int ftruncate(int fd, off_t length)
{
    (void)fd;
    if (length < 0) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

long sysconf(int name)
{
    if (name == _SC_PAGESIZE) {
        return 4096;
    }
    return 1;
}

char *getcwd(char *buffer, size_t size)
{
    if (buffer == NULL || size < 2) {
        errno = ERANGE;
        return NULL;
    }
    buffer[0] = '/';
    buffer[1] = '\0';
    return buffer;
}

int getpagesize(void)
{
    return 4096;
}

__attribute__((noreturn)) void _exit(int status)
{
    unsigned int value;
    char digits[10];
    size_t count = 0;

    _write(STDERR_FILENO, "\n[arm-am] exit=", sizeof("\n[arm-am] exit=") - 1);
    if (status < 0) {
        baremetal_platform_putc('-');
        value = 0U - (unsigned int)status;
    } else {
        value = (unsigned int)status;
    }
    do {
        digits[count++] = (char)('0' + value % 10U);
        value /= 10U;
    } while (value != 0U);
    while (count != 0U) {
        baremetal_platform_putc(digits[--count]);
    }
    baremetal_platform_putc('\n');
    baremetal_platform_halt(status);
}
