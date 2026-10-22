/* Linker script dùng chung: bố trí chương trình STM32F103C8 trong FLASH/RAM.
 * Tên _sdata, _edata, _sbss... phải khớp startup_stm32f103xb.s của CMSIS.
 */
ENTRY(Reset_Handler)
/* FLASH chứa mã và dữ liệu khởi tạo; RAM chứa biến, heap và stack. */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}
/* Stack chính/ISR bắt đầu từ đỉnh RAM và mở rộng về phía địa chỉ thấp. */
_estack = ORIGIN(RAM) + LENGTH(RAM);
/* Không dành riêng heap C. Heap FreeRTOS 10 KiB là mảng ucHeap trong .bss. */
_Min_Heap_Size = 0;
/* Dành 2 KiB cho stack chính/ISR; stack từng task được cấp từ heap FreeRTOS. */
_Min_Stack_Size = 0x800;
SECTIONS
{
    /* Bảng vector phải ở đầu FLASH; KEEP ngăn linker loại bỏ vì không được gọi trực tiếp. */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
    } > FLASH
    /* Mã máy và hằng số chỉ đọc. ALIGN giữ địa chỉ phù hợp với truy cập ARM. */
    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > FLASH
    /* Bảng thông tin hỗ trợ xử lý ngoại lệ/unwind nếu thư viện sinh ra chúng. */
    .ARM.extab : { *(.ARM.extab*) } > FLASH
    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH
    /* Các mảng hàm khởi tạo/kết thúc của runtime C được startup tham chiếu. */
    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } > FLASH
    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
    } > FLASH
    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } > FLASH
    /* Startup chép giá trị ban đầu từ địa chỉ FLASH này sang vùng .data trong RAM. */
    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH
    /* Biến chưa khởi tạo hoặc khởi tạo 0; startup xóa về 0, không lưu ảnh dữ liệu ở FLASH. */
    .bss (NOLOAD) :
    {
        . = ALIGN(8);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        _ebss = .;
    } > RAM
    /* Đặt chỗ trong RAM để linker báo lỗi nếu dữ liệu + vùng dự phòng vượt 20 KiB. */
    ._user_heap_stack (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        . = . + _Min_Heap_Size + _Min_Stack_Size;
        . = ALIGN(8);
    } > RAM
}
