#pragma once

#define GET_VEC3(vec) Vector3f{vec[0].GetFloat(), vec[1].GetFloat(), vec[2].GetFloat()}
#define GET_VEC2(vec) Vector2f{vec[0].GetFloat(), vec[1].GetFloat()}

#define HUDDATA_NAME                "name"
#define HUDDATA_TYPE                "type"
#define HUDDATA_POSITION            "position"
#define HUDDATA_ORIGIN              "origin"
#define HUDDATA_SIZE                "size"
#define HUDDATA_VISIBILITY          "defaultVisibility"
#define HUDDATA_TEXT_FONTSIZE       "fontsize"
#define HUDDATA_TEXT_TEXT           "text"
#define HUDDATA_TEXT_FONT           "font"
#define HUDDATA_TEXT_COLOR          "colorIndex"
#define HUDDATA_IMG_TEXTURE         "textureIndex"
#define HUDDATA_RECT_COLOR          "colorIndex" 
#define HUDDATA_RECT_OPACITY        "opacity" 
#define HUDDATA_BUTTON_BGCOLOR      "buttonColorIndex"
#define HUDDATA_BUTTON_TEXTCOLOR    "textColorIndex"
#define HUDDATA_LISTB_SPACING       "spacing"
#define HUDDATA_SLIDER_MINVAL       "minValue"
#define HUDDATA_SLIDER_MAXVAL       "maxValue"
#define HUDDATA_SLIDER_LABEL        "label"
#define HUDDATA_SLIDER_FORMAT       "innerLabel"
#define HUDDATA_SLIDER_FORMATVEC    "innerLabelVector"
