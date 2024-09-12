#include <aarch64/mmu.h>
#include <common/rc.h>
#include <common/spinlock.h>
#include <driver/memlayout.h>
#include <kernel/mem.h>
// #include <kernel/printk.h>

RefCount kalloc_page_cnt;

void kinit() {
    init_rc(&kalloc_page_cnt);
}

void* kalloc_page() {
    increment_rc(&kalloc_page_cnt);

    return NULL;
}

void kfree_page(void* p) {
    decrement_rc(&kalloc_page_cnt);

    return;
}

void* kalloc(unsigned long long size) {
    return NULL;
}

void kfree(void* ptr) {
    return;
}
