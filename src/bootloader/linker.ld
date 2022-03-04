SECTIONS
{
    
    . = 0x80000;

    __prog_start = .;
    
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

    __prog_end = .;

    .bss (NOLOAD) : 
    {

        . = ALIGN(16);

        __bss_start = .;

        *(.bss .bss.*)

        __bss_end = .;

    }


}
__bss_size = (__bss_end - __bss_start) >> 3;
__prog_size = (__prog_end - __prog_start) >> 3;