#include <string.h>

// seems to match the memcpy from the Xenoblade repo, but that one is asm
// an attempt to convert it to C can be found here:
// https://github.com/DarkRTA/rb3/blob/master/src/sdk/PowerPC_EABI_Support/Runtime/__mem.c

//Handwritten
DECL_SECTION(".init") asm void* memcpy(void* dest, const void* src, size_t size){
    //Return if size is 0
    cmplwi cr1, r5, 0
    beqlr cr1

    cmplw cr1, r4, r3
    blt cr1, reverse
    beqlr cr1

    li r6, 0x80
    cmplw cr5, r5, r6
    blt cr5, test_word_alignment

    clrlwi r9, r4, 0x1d
    clrlwi r10, r3, 0x1d
    subf r8, r10, r3

    //Request a data cache block fetch
    dcbt 0, r4

    xor. r11, r10, r9
    bne byte_setup
    andi. r10, r10, 7
    beq+ double_copy_setup
    li r6, 8
    subf r9, r9, r6
    addi r8, r3, 0
    mtctr r9
    subf r5, r9, r5

byte_loop_double_align:
    lbz r9, 0(r4)
    addi r4, r4, 1
    stb r9, 0(r8)
    addi r8, r8, 1
    bdnz byte_loop_double_align

double_copy_setup:
    srwi r6, r5, 5
    mtctr r6

double_loop:
    lfd f1, 0(r4)
    lfd f2, 8(r4)
    lfd f3, 0x10(r4)
    lfd f4, 0x18(r4)
    addi r4, r4, 0x20
    stfd f1, 0(r8)
    stfd f2, 8(r8)
    stfd f3, 0x10(r8)
    stfd f4, 0x18(r8)

    addi r8, r8, 0x20
    bdnz double_loop
    andi. r6, r5, 0x1f
    beqlr
    addi r4, r4, -1
    mtctr r6
    addi r8, r8, -1

byte_loop_1:
    lbzu r9, 1(r4)
    stbu r9, 1(r8)
    bdnz byte_loop_1
    blr

test_word_alignment:
    li r6, 0x14
    cmplw cr5, r5, r6
    ble cr5, byte_setup
    clrlwi r9, r4, 0x1e
    clrlwi r10, r3, 0x1e
    xor. r11, r10, r9
    bne byte_setup
    li r6, 4
    subf r9, r9, r6
    addi r8, r3, 0
    subf r5, r9, r5
    mtctr r9

byte_loop_word_align:
    lbz r9, 0(r4)
    addi r4, r4, 1
    stb r9, 0(r8)
    addi r8, r8, 1
    bdnz byte_loop_word_align

word_copy_setup:
    srwi r6, r5, 4
    mtctr r6

word_loop:
    lwz r9, 0(r4)
    lwz r10, 4(r4)
    lwz r11, 8(r4)
    lwz r12, 0xc(r4)
    addi r4, r4, 0x10
    stw r9, 0(r8)
    stw r10, 4(r8)
    stw r11, 8(r8)
    stw r12, 0xc(r8)
    addi r8, r8, 0x10
    bdnz word_loop

    andi. r6, r5, 0xf
    beqlr
    addi r4, r4, -1
    mtctr r6
    addi r8, r8, -1

byte_loop_2:
    lbzu r9, 1(r4)
    stbu r9, 1(r8)
    bdnz byte_loop_2
    blr

byte_setup:
    addi r7, r4, -1
    addi r8, r3, -1
    mtctr r5

byte_loop_3:
    lbzu r9, 1(r7)
    stbu r9, 1(r8)
    bdnz byte_loop_3
    blr

reverse:
    add r4, r4, r5
    add r12, r3, r5
    li r6, 0x80
    cmplw cr5, r5, r6
    blt cr5, reverse_test_word_alignment
    clrlwi r9, r4, 0x1d
    clrlwi r10, r12, 0x1d
    xor. r11, r10, r9
    bne reverse_byte_setup
    andi. r10, r10, 7
    beq+ reverse_double_copy_setup
    mtctr r10

reverse_byte_loop_double_align:
    lbzu r9, -1(r4)
    stbu r9, -1(r12)
    bdnz reverse_byte_loop_double_align

reverse_double_copy_setup:
    subf r5, r10, r5
    srwi r6, r5, 5
    mtctr r6

reverse_double_loop:
    lfd f1, -8(r4)
    lfd f2, -0x10(r4)
    lfd f3, -0x18(r4)
    lfd f4, -0x20(r4)
    addi r4, r4, -32
    stfd f1, -8(r12)
    stfd f2, -0x10(r12)
    stfd f3, -0x18(r12)
    stfdu f4, -0x20(r12)

    bdnz reverse_double_loop

    andi. r6, r5, 0x1f
    beqlr
    mtctr r6

reverse_byte_loop_1:
    lbzu r9, -1(r4)
    stbu r9, -1(r12)
    bdnz reverse_byte_loop_1
    blr

reverse_test_word_alignment:
    li r6, 0x14
    cmplw cr5, r5, r6
    ble cr5, reverse_byte_setup
    clrlwi r9, r4, 0x1e
    clrlwi r10, r12, 0x1e
    xor. r11, r10, r9
    bne reverse_byte_setup
    andi. r10, r10, 7
    beq+ reverse_word_loop_setup
    mtctr r10

reverse_byte_loop_word_align:
    lbzu r9, -1(r4)
    stbu r9, -1(r12)
    bdnz reverse_byte_loop_word_align

reverse_word_loop_setup:
    subf r5, r10, r5
    srwi r6, r5, 4
    mtctr r6

reverse_word_loop:
    lwz r9, -4(r4)
    lwz r10, -8(r4)
    lwz r11, -0xc(r4)
    lwz r8, -0x10(r4)
    addi r4, r4, -16
    stw r9, -4(r12)
    stw r10, -8(r12)
    stw r11, -0xc(r12)
    stwu r8, -0x10(r12)
    bdnz reverse_word_loop

    andi. r6, r5, 0xf
    beqlr
    mtctr r6

reverse_byte_loop_2:
    lbzu r9, -1(r4)
    stbu r9, -1(r12)
    bdnz reverse_byte_loop_2
    blr

reverse_byte_setup:
    mtctr r5

reverse_byte_loop_3:
    lbzu r9, -1(r4)
    stbu r9, -1(r12)
    bdnz reverse_byte_loop_3
    blr
}

DECL_SECTION(".init") void __fill_mem(void* dst, int c, size_t n)
{
    int work;
    char* bdst = (char*)dst;
    int* wdst = (int*)dst;
    unsigned int cc = (unsigned char)c;

    bdst--;

    // Optimize when filling more than 32B
    if (n >= 32)
    {
        // How many bytes dest is word-unaligned
        work = ~(u32)(bdst) & 3;

        // Byte-store to hit word alignment
        if (work)
        {
            n -= work;
            do
            {
                *++bdst = cc;
            } while(--work);
        }

        // Build word-sized value
        if (cc)
        {
            cc = (cc << 24) | (cc << 16) | (cc << 8) | cc;
        }

        // 32-byte store
        work = n / 32;
        wdst = (int *)(bdst - 3);
        if (work)
        {
            do
            {
                wdst[1] = cc;
                wdst[2] = cc;
                wdst[3] = cc;
                wdst[4] = cc;
                wdst[5] = cc;
                wdst[6] = cc;
                wdst[7] = cc;
                wdst[8] = cc;
                wdst += 8;
            } while (--work);
        }

        // 4-byte (word) store
        work = (n / 4) & 7;
        if (work)
        {
            do
            {
                *++wdst = cc;
            } while(--work);
        }

        bdst = (char *)wdst + 3;
        n &= 3;
    }

    // Remaining byte-copy, or caller specified < 32B store
    if (n > 0)
    {
        do
        {
            *++bdst = cc;
        } while(--n);
    }
}

DECL_SECTION(".init") void * memset(void * dest, int val, size_t count)
{
    __fill_mem(dest, val, count);
    return dest;
}

DECL_WEAK size_t strlen(const char* s) {
    const u8* p = (u8*)s - 1;
    size_t len = -1;

    do {
        len++;
    } while (*++p);

    return len;
}
