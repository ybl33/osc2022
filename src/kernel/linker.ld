__heap_size  = 0x00200000;

SECTIONS
{
    . = 0x80000;

    __start = .;
    
    .text : 
    { 
        KEEP(*(.text.boot))
         *(.text) 
    }
    .rodata : 
    {
        *(.rodata) 
    }

    .data : 
    { 
        *(.data) 
    }

    .bss (NOLOAD) : 
    {

        . = ALIGN(16);

        __bss_start = .;

        *(.bss .bss.*)

        __bss_end = .;

    }
    
    __heap_start = .;
    
    . = . + __heap_size;

    __heap_end = .;

    __end = .;
}
__bss_size = (__bss_end - __bss_start) >> 3;