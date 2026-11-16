ENTRY(_start)

MEMORY
{
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 0x1ffe00000
}

SECTIONS
{
    . = ORIGIN(RAM);
    .text : {
        KEEP(*(.text.boot))
        *(.text .text.*)
    } > RAM

    .rodata : {
        *(.rodata .rodata.*)
    } > RAM

    .eh_frame : {
        KEEP(*(.eh_frame))
        KEEP(*(.eh_frame.*))
    } > RAM

    .gcc_except_table : {
        *(.gcc_except_table .gcc_except_table.*)
    } > RAM

    .preinit_array : {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } > RAM

    .init_array : {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN(__init_array_end = .);
    } > RAM

    .fini_array : {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } > RAM

    .data : {
        *(.data .data.*)
    } > RAM

    .page_tables (NOLOAD) : ALIGN(4096) {
        KEEP(*(.bss.page_tables))
    } > RAM

    .bss (NOLOAD) : ALIGN(16) {
        __bss_start = .;
        *(.bss .bss.* COMMON)
        . = ALIGN(16);
        __bss_end = .;
    } > RAM

    . = ALIGN(16);
    __image_end = .;
    __heap_start = .;
    /* Validation-only file export window, read by QEMU/GDB after main. */
    __spec_export_start = 0x23b000000;
    __spec_export_end = 0x23f000000;
    __stack_top = ORIGIN(RAM) + LENGTH(RAM);
    __stack_bottom = __stack_top - 0x100000;
    __heap_end = __spec_export_start;

    /DISCARD/ : { *(.comment .note .note.*) }
}

ASSERT(__image_end <= __heap_end, "AArch64 bare-metal image overlaps stack")
ASSERT(__spec_export_end <= __stack_bottom,
       "SPEC export window overlaps stack")
