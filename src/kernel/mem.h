#pragma once

void kinit();

void* kalloc_page();
void kfree_page(void*);

void* kalloc(unsigned long long);
void kfree(void*);
