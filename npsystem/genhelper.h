#pragma once


// integer and float values quality byte:
// 0x00 - bad quality
// 0x01 - good quality
// >= 0x02  - reserved for future

#define i_at(ix) static_cast<CInputSlot&>(slots.at_i(ix))
#define o_at(ix) static_cast<COutputSlot&>(slots.at_o(ix))

#define ForIn(x) size_t ix = 0; for (auto it = x.begin_in(); it != x.end_in(); it++, ix++)
#define ForInR(x) size_t ix = 0; for (auto it = x.rbegin_in(); it != x.rend_in(); it++, ix++)
#define ForOut(x) size_t ix = 0; for (auto it = x.begin_out(); it != x.end_out(); it++, ix++)
#define ForOutR(x) size_t ix = 0; for (auto it = x.rbegin_out(); it != x.rend_out(); it++, ix++)
#define ins() static_cast<CInputSlot&>(*(*it)->e_slot)
#define ons() static_cast<COutputSlot&>(*(*it)->e_slot)