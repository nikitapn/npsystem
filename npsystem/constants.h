#pragma once

#include <d2d1helper.h>

#define CXA constexpr auto 

namespace constants {
namespace block {
// Graphics
CXA GRIDSZ_L = 16L;
CXA GRIDSZ_F = 16.0f;
// Block parameters
CXA RADIUS_X = 16.f;
CXA RADIUS_Y = 16.f;
CXA BLOCK_WIDTH = 140.f;
CXA PARAM_WIDTH = 170.f;
CXA PARAM_HEIGHT = 28.f;
CXA SLOT_SIZE = 12.f;
CXA SLOT_SPACE = 8.f;
CXA SLOT_TEXT = 60.f;
CXA BETWEEN_SLOTS = 16.0f;
CXA HEAD_HEIGHT = 16.0f;
CXA FOOTER_HEIGHT = 10.0f;

CXA y_pos(const int position) noexcept {
	return HEAD_HEIGHT + 8.0f + 2.0f * BETWEEN_SLOTS * static_cast<float>(position);
}

CXA block_height(const int max_slots) noexcept {
	return y_pos(max_slots - 1) + BETWEEN_SLOTS + 4.0f;
}

CXA SCHEDULE_BLOCK_HEIGHT = y_pos(4) + 60;
CXA SCHEDULE_BLOCK_WIDTH = 400.0f;

} // namespace block

namespace slider {
namespace thing {
CXA offset = 5.0f;
CXA height = 18.0f;
CXA h1 = 9.0f;
CXA width = 6.0f;
} // namespace thing

CXA SCHEDULE_BLOCK_X_OFFSET = 16.0f;
CXA SCHEDULE_BLOCK_Y_OFFSET = block::y_pos(4);

CXA SLIDER_WIDTH = constants::block::SCHEDULE_BLOCK_WIDTH - SCHEDULE_BLOCK_X_OFFSET * 2;
CXA SLIDER_HEIGHT = 45.0f;

CXA SCHEDULE_BLOCK_TIME_CHART_OFFSET = thing::offset + thing::width / 2.0f;
CXA SCHEDULE_BLOCK_TIME_CHART_WIDTH = SLIDER_WIDTH - SCHEDULE_BLOCK_TIME_CHART_OFFSET * 2.0f;
CXA SCHEDULE_BLOCK_TIME_HOUR_INTERVALS  = 4;
CXA SCHEDULE_BLOCK_TIME_MAX_TIME_POINTS  = 24.0f * static_cast<float>(SCHEDULE_BLOCK_TIME_HOUR_INTERVALS) - 1.0f;
CXA SLIDER_STEP = SCHEDULE_BLOCK_TIME_CHART_WIDTH / SCHEDULE_BLOCK_TIME_MAX_TIME_POINTS;

} // namespace slider



namespace treeview {
CXA icon_cx = 19;
CXA icon_cy = 19;
CXA icon_y_offset = 2;
CXA label_cy = icon_cy + icon_y_offset * 2 ;

CXA font_height = 18;

CXA status_icon_cx = 12;
CXA status_icon_cy = 12;

}

} // namespace constants

#undef CXA