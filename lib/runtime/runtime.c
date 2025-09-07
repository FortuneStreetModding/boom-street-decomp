#include <types.h>

const u64 __constants[] = {
    0x0000000000000000, // 0
    0x41F0000000000000, // ULONG_MAX + 1
    0x41E0000000000000  // INT_MAX + 1
};

asm u32 __cvt_fp2unsigned(register f32 x) {
    // clang-format off
    nofralloc
    stwu r1, -0x10(r1)

    lis r4, __constants@ha
    addi r4, r4, __constants@l

    li r3, 0

    lfd f0, 0(r4)
    lfd f3, 8(r4)
    lfd f4, 16(r4)

    // Clamp value to [0, ULONG_MAX]
    fcmpu cr0, x, f0
    fcmpu cr6, x, f3
    blt cr0, epilogue
    subi r3, r3, 1
    bge cr6, epilogue

    // Convert float to unsigned (subract INT_MAX)
    fcmpu cr7, x, f4
    fmr f2, x
    blt cr7, convert_fp
    fsub f2, x, f4

convert_fp:
    // Convert to unsigned (using stack to change register class)
    fctiwz f2, f2
    stfd f2, 8(r1)
    lwz r3, 0xc(r1)
    blt cr7, epilogue

    // Sign extend if necessary
    addis r3, r3, 0x8000

epilogue:
    addi r1, r1, 0x10
    blr
    // clang-format on
}
