.globl vc_ipc_null_check
.section .hook, "ax"
vc_ipc_null_check:
stwu sp, -0x20(sp)
mflr r4
stw r4, 0x24(sp)
stw r31, 0x1c(sp)
mr r31, r1
stw r10, 0x18(r31)
stw r12, 0x14(r31)
lwz r10, 0x1c(r31)
li r12, 0
lwz r3, 0x18(r10)
cmpwi r3, 0
beq nullval
lis r10, 0x8000
cmplw cr7, r3, r10
bgt cr7, loadr3
addis r3, r3, 0x8000
loadr3:
lwzx r3, r3, r29
cmpwi r3, 0
bne ret
nullval:
addi r12, r12, 0x08
ret:
addi r11, r31, 0x20
lwz r4, 4(r11)
mtlr r4
add r4, r4, r12
lwz r31, -0x4(r11)
lwz r10, -0x8(r11)
lwz r12, -0xC(r11)
mr sp, r11
li r4, 1
blr

.globl vc_ipc_null_check2
.section .hook, "ax"
vc_ipc_null_check2:
stwu sp, -0x20(sp)
mflr r4
stw r4, 0x24(sp)
stw r31, 0x1c(sp)
mr r31, r1
stw r10, 0x18(r31)
stw r12, 0x14(r31)
lwz r10, 0x1c(sp)

lwz r3, 0x18(r10)
lis r12, 0x8000
cmplw cr7, r3, r12
bgt cr7, storer0
addis r3, r3, 0x8000
storer0:
stwx r0, r3, r29
lwz r3, 0x18(r10)
add r12, r3, r29

addi r11, r31, 0x20
lwz r4, 4(r11)
mtlr r4
mr r4, r12
lwz r31, -0x4(r11)
lwz r10, -0x8(r11)
lwz r12, -0xC(r11)
mr sp, r11
blr
