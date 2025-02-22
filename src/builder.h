#pragma once
#include <Arduino.h>

#include "canvas.h"
#include "config.hpp"
#include "macro.hpp"
#include "utils/build.h"
#include "utils/color.h"
#include "utils/datatypes.h"
#include "utils/log.h"
#include "utils/misc.h"
#include "utils/pos.h"

class HubBuilder {
   public:
    // ========================== WIDGET ==========================
    void BeginWidgets(int height = 0) {
        if (_isUI()) {
            tab_width = 100;
            _add(F("{'type':'widget_b','height':"));
            *sptr += height;
            _end();
        }
    }
    void EndWidgets() {
        tab_width = 0;
        if (_isUI()) {
            _add(F("{'type':'widget_e'"));
            _end();
        }
    }
    void WidgetSize(int width) {
        tab_width = width;
    }

    // ========================== DUMMY ===========================
    bool Dummy(FSTR name, void* value = nullptr, GHdata_t type = GH_NULL) {
        return _dummy(true, name, value, type);
    }
    bool Dummy(CSREF name, void* value = nullptr, GHdata_t type = GH_NULL) {
        return _dummy(false, name.c_str(), value, type);
    }

    bool _dummy(bool fstr, VSPTR name, void* value, GHdata_t type) {
        if (_isRead()) {
            if (_checkName(name, fstr)) GHtypeToStr(sptr, value, type);
        } else if (bptr->type == GH_BUILD_ACTION) {
            return bptr->parseSet(name, value, type, fstr);
        }
        return 0;
    }

    // ========================== BUTTON ==========================
    bool Button(FSTR name, bool* value = nullptr, FSTR label = nullptr, uint32_t color = GH_DEFAULT, int size = 22) {
        return _button(true, F("button"), name, value, label, color, size);
    }
    bool Button(CSREF name, bool* value = nullptr, CSREF label = "", uint32_t color = GH_DEFAULT, int size = 22) {
        return _button(false, F("button"), name.c_str(), value, label.c_str(), color, size);
    }

    bool ButtonIcon(FSTR name, bool* value = nullptr, FSTR label = nullptr, uint32_t color = GH_DEFAULT, int size = 50) {
        return _button(true, F("button_i"), name, value, label, color, size);
    }
    bool ButtonIcon(CSREF name, bool* value = nullptr, CSREF label = "", uint32_t color = GH_DEFAULT, int size = 50) {
        return _button(false, F("button_i"), name.c_str(), value, label.c_str(), color, size);
    }

    bool _button(bool fstr, FSTR tag, VSPTR name, bool* value, VSPTR label, uint32_t color, int size) {
        if (_isUI()) {
            _begin(tag);
            _name(name, fstr);
            _label(label, fstr);
            _color(color);
            _size(size);
            _tabw();
            _end();
        } else if (bptr->type == GH_BUILD_ACTION) {
            return bptr->parseClick(name, value, fstr);
        }
        return 0;
    }

    // ========================== LABEL ==========================
    void Label(FSTR name, const String& value = "", FSTR label = nullptr, uint32_t color = GH_DEFAULT, int size = 40) {
        _label(true, name, value, label, color, size);
    }
    void Label(CSREF name, const String& value = "", CSREF label = "", uint32_t color = GH_DEFAULT, int size = 40) {
        _label(false, name.c_str(), value, label.c_str(), color, size);
    }
    void _label(bool fstr, VSPTR name, const String& value, VSPTR label, uint32_t color, int size) {
        if (_isUI()) {
            _begin(F("label"));
            _name(name, fstr);
            _value();
            _quot();
            *sptr += value;
            _quot();
            _label(label, fstr);
            _color(color);
            _size(size);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) *sptr += value;
        }
    }

    // ========================== TITLE ==========================
    void Title(FSTR label) {
        _title(true, label);
    }
    void Title(CSREF label) {
        _title(false, label.c_str());
    }
    void _title(bool fstr, VSPTR label) {
        if (_isUI()) {
            _begin(F("title"));
            _label(label, fstr);
            _end();
        }
    }

    // ========================== LOG ==========================
    void Log(FSTR name, GHlog* log, FSTR label = nullptr) {
        _log(true, name, log, label);
    }
    void Log(CSREF name, GHlog* log, CSREF label = "") {
        _log(false, name.c_str(), log, label.c_str());
    }
    void _log(bool fstr, VSPTR name, GHlog* log, VSPTR label) {
        if (_isUI()) {
            _begin(F("log"));
            _name(name, fstr);
            _text();
            _quot();
            log->read(sptr);
            _quot();
            _label(label, fstr);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) GH_escapeStr(sptr, log->read().c_str(), false);
        }
    }

    // ========================== DISPLAY ==========================
    void Display(FSTR name, FSTR value = nullptr, FSTR label = nullptr, uint32_t color = GH_DEFAULT, int rows = 2, int size = 40) {
        _display(true, name, value, label, color, rows, size);
    }
    void Display(CSREF name, const String& value = "", CSREF label = "", uint32_t color = GH_DEFAULT, int rows = 2, int size = 40) {
        _display(false, name.c_str(), value.c_str(), label.c_str(), color, rows, size);
    }
    void _display(bool fstr, VSPTR name, VSPTR value, VSPTR label, uint32_t color, int rows, int size) {
        if (_isUI()) {
            _begin(F("display"));
            _name(name, fstr);
            _value();
            _quot();
            GH_escapeStr(sptr, value, fstr);
            _quot();
            _label(label, fstr);
            _color(color);
            _add(F(",'rows':"));
            *sptr += rows;
            _size(size);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) GH_escapeStr(sptr, value, fstr);
        }
    }

    // ========================== HTML ==========================
    void HTML(FSTR name, FSTR value = nullptr, FSTR label = nullptr) {
        _html(true, name, value, label);
    }
    void HTML(CSREF name, const String& value = "", CSREF label = "") {
        _html(false, name.c_str(), value.c_str(), label.c_str());
    }
    void _html(bool fstr, VSPTR name, VSPTR value, VSPTR label) {
        if (_isUI()) {
            _begin(F("html"));
            _name(name, fstr);
            _value();
            _quot();
            GH_escapeStr(sptr, value, fstr);
            _quot();
            _label(label, fstr);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) GH_escapeStr(sptr, value, fstr);
        }
    }

    // =========================== JS ===========================
    void JS(FSTR value = nullptr) {
        _js(true, value);
    }
    void JS(const String& value = "") {
        _js(false, value.c_str());
    }
    void _js(bool fstr, VSPTR value) {
        if (_isUI()) {
            _begin(F("js"));
            _value();
            _quot();
            GH_escapeStr(sptr, value, fstr);
            _quot();
            _end();
        }
    }

    // ========================== INPUT ==========================
    bool Input(FSTR name, void* value = nullptr, GHdata_t type = GH_NULL, FSTR label = nullptr, int maxv = 0, FSTR regex = nullptr, uint32_t color = GH_DEFAULT) {
        return _input(true, F("input"), name, value, type, label, maxv, regex, color);
    }
    bool Input(CSREF name, void* value = nullptr, GHdata_t type = GH_NULL, CSREF label = "", int maxv = 0, CSREF regex = "", uint32_t color = GH_DEFAULT) {
        return _input(false, F("input"), name.c_str(), value, type, label.c_str(), maxv, regex.c_str(), color);
    }

    // ========================== PASS ==========================
    bool Pass(FSTR name, void* value = nullptr, GHdata_t type = GH_NULL, FSTR label = nullptr, int maxv = 0, uint32_t color = GH_DEFAULT) {
        return _input(true, F("pass"), name, value, type, label, maxv, nullptr, color);
    }
    bool Pass(CSREF name, void* value = nullptr, GHdata_t type = GH_NULL, CSREF label = "", int maxv = 0, uint32_t color = GH_DEFAULT) {
        return _input(false, F("pass"), name.c_str(), value, type, label.c_str(), maxv, "", color);
    }

    bool _input(bool fstr, FSTR tag, VSPTR name, void* value, GHdata_t type, VSPTR label, int maxv, VSPTR regex, uint32_t color) {
        if (_isUI()) {
            _begin(tag);
            _name(name, fstr);
            _value();
            _quot();
            GHtypeToStr(sptr, value, type);
            _quot();
            _label(label, fstr);
            if (maxv) _maxv((long)maxv);
            _add(F(",'regex':'"));
            GH_escapeStr(sptr, regex, fstr);
            _quot();
            _color(color);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) GHtypeToStr(sptr, value, type);
        } else if (bptr->type == GH_BUILD_ACTION) {
            return bptr->parseSet(name, value, type, fstr);
        }
        return 0;
    }

    // ========================== SLIDER ==========================
    bool Slider(FSTR name, void* value = nullptr, GHdata_t type = GH_NULL, FSTR label = nullptr, float minv = 0, float maxv = 100, float step = 1, uint32_t color = GH_DEFAULT) {
        return _spinner(true, F("slider"), name, value, type, label, minv, maxv, step, color);
    }
    bool Slider(CSREF name, void* value = nullptr, GHdata_t type = GH_NULL, CSREF label = "", float minv = 0, float maxv = 100, float step = 1, uint32_t color = GH_DEFAULT) {
        return _spinner(false, F("slider"), name.c_str(), value, type, label.c_str(), minv, maxv, step, color);
    }

    // ========================== SPINNER ==========================
    bool Spinner(FSTR name, void* value = nullptr, GHdata_t type = GH_NULL, FSTR label = nullptr, float minv = 0, float maxv = 100, float step = 1, uint32_t color = GH_DEFAULT) {
        return _spinner(true, F("spinner"), name, value, type, label, minv, maxv, step, color);
    }
    bool Spinner(CSREF name, void* value = nullptr, GHdata_t type = GH_NULL, CSREF label = "", float minv = 0, float maxv = 100, float step = 1, uint32_t color = GH_DEFAULT) {
        return _spinner(false, F("spinner"), name.c_str(), value, type, label.c_str(), minv, maxv, step, color);
    }

    bool _spinner(bool fstr, FSTR tag, VSPTR name, void* value, GHdata_t type, VSPTR label, float minv, float maxv, float step, uint32_t color) {
        if (_isUI()) {
            _begin(tag);
            _name(name, fstr);
            _value();
            GHtypeToStr(sptr, value, type);
            _label(label, fstr);
            _minv(minv);
            _maxv(maxv);
            _step(step);
            _color(color);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) GHtypeToStr(sptr, value, type);
        } else if (bptr->type == GH_BUILD_ACTION) {
            return bptr->parseSet(name, value, type, fstr);
        }
        return 0;
    }

    // ========================== GAUGE ===========================
    void Gauge(FSTR name, float value = 0, FSTR text = nullptr, FSTR label = nullptr, float minv = 0, float maxv = 100, float step = 1, uint32_t color = GH_DEFAULT) {
        _gauge(true, name, value, text, label, minv, maxv, step, color);
    }
    void Gauge(CSREF name, float value = 0, CSREF text = "", CSREF label = "", float minv = 0, float maxv = 100, float step = 1, uint32_t color = GH_DEFAULT) {
        _gauge(false, name.c_str(), value, text.c_str(), label.c_str(), minv, maxv, step, color);
    }

    void _gauge(bool fstr, VSPTR name, float value, VSPTR text, VSPTR label, float minv, float maxv, float step, uint32_t color) {
        if (_isUI()) {
            _begin(F("gauge"));
            _name(name, fstr);
            _value();
            *sptr += value;
            _text(text, fstr);
            _label(label, fstr);
            _minv(minv);
            _maxv(maxv);
            _step(step);
            _color(color);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) *sptr += value;
        }
    }

    // ========================== SWITCH ==========================
    bool Switch(FSTR name, bool* value = nullptr, FSTR label = nullptr, uint32_t color = GH_DEFAULT) {
        return _switch(true, F("switch"), name, value, label, color, nullptr);
    }
    bool Switch(CSREF name, bool* value = nullptr, CSREF label = "", uint32_t color = GH_DEFAULT) {
        return _switch(false, F("switch"), name.c_str(), value, label.c_str(), color, nullptr);
    }

    bool SwitchIcon(FSTR name, bool* value = nullptr, FSTR label = nullptr, FSTR text = nullptr, uint32_t color = GH_DEFAULT) {
        return _switch(true, F("switch_i"), name, value, label, color, text);
    }
    bool SwitchIcon(CSREF name, bool* value = nullptr, CSREF label = "", CSREF text = "", uint32_t color = GH_DEFAULT) {
        return _switch(false, F("switch_i"), name.c_str(), value, label.c_str(), color, text.c_str());
    }

    bool SwitchText(FSTR name, bool* value = nullptr, FSTR label = nullptr, FSTR text = nullptr, uint32_t color = GH_DEFAULT) {
        return _switch(true, F("switch_t"), name, value, label, color, text);
    }
    bool SwitchText(CSREF name, bool* value = nullptr, CSREF label = "", CSREF text = "", uint32_t color = GH_DEFAULT) {
        return _switch(false, F("switch_t"), name.c_str(), value, label.c_str(), color, text.c_str());
    }

    bool _switch(bool fstr, FSTR tag, VSPTR name, bool* value, VSPTR label, uint32_t color, VSPTR text) {
        if (_isUI()) {
            _begin(tag);
            _name(name, fstr);
            _value();
            GHtypeToStr(sptr, value, GH_BOOL);
            _label(label, fstr);
            _color(color);
            _text(text, fstr);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) GHtypeToStr(sptr, value, GH_BOOL);
        } else if (bptr->type == GH_BUILD_ACTION) {
            return bptr->parseSet(name, value, GH_BOOL, fstr);
        }
        return 0;
    }

    // ========================== DATETIME ==========================
    bool Date(FSTR name, void* value, FSTR label = nullptr, uint32_t color = GH_DEFAULT) {
        return _date(true, F("date"), name, value, label, color);
    }
    bool Date(CSREF name, void* value, CSREF label = "", uint32_t color = GH_DEFAULT) {
        return _date(false, F("date"), name.c_str(), value, label.c_str(), color);
    }

    bool Time(FSTR name, void* value, FSTR label = nullptr, uint32_t color = GH_DEFAULT) {
        return _date(true, F("time"), name, value, label, color);
    }
    bool Time(CSREF name, void* value, CSREF label = "", uint32_t color = GH_DEFAULT) {
        return _date(false, F("time"), name.c_str(), value, label.c_str(), color);
    }

    bool DateTime(FSTR name, void* value, FSTR label = nullptr, uint32_t color = GH_DEFAULT) {
        return _date(true, F("datetime"), name, value, label, color);
    }
    bool DateTime(CSREF name, void* value, CSREF label = "", uint32_t color = GH_DEFAULT) {
        return _date(false, F("datetime"), name.c_str(), value, label.c_str(), color);
    }

    bool _date(bool fstr, FSTR tag, VSPTR name, void* value, VSPTR label, uint32_t color) {
        if (_isUI()) {
            _begin(tag);
            _name(name, fstr);
            _label(label, fstr);
            _value();
            GHtypeToStr(sptr, value, GH_UINT32);
            _color(color);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) GHtypeToStr(sptr, value, GH_UINT32);
        } else if (bptr->type == GH_BUILD_ACTION) {
            return bptr->parseSet(name, value, GH_UINT32, fstr);
        }
        return 0;
    }

    // ========================== SELECT ==========================
    bool Select(FSTR name, uint8_t* value, FSTR text, FSTR label = nullptr, uint32_t color = GH_DEFAULT) {
        return _select(true, name, value, text, label, color);
    }
    bool Select(CSREF name, uint8_t* value, CSREF text, CSREF label = "", uint32_t color = GH_DEFAULT) {
        return _select(false, name.c_str(), value, text.c_str(), label.c_str(), color);
    }

    bool _select(bool fstr, VSPTR name, uint8_t* value, VSPTR text, VSPTR label, uint32_t color) {
        if (_isUI()) {
            _begin(F("select"));
            _name(name, fstr);
            _value();
            GHtypeToStr(sptr, value, GH_UINT8);
            _text(text, fstr);
            _label(label, fstr);
            _color(color);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) GHtypeToStr(sptr, value, GH_UINT8);
        } else if (bptr->type == GH_BUILD_ACTION) {
            return bptr->parseSet(name, value, GH_UINT8, fstr);
        }
        return 0;
    }

    // ========================== FLAGS ==========================
    bool Flags(FSTR name, GHflags* value = nullptr, FSTR text = nullptr, FSTR label = nullptr, uint32_t color = GH_DEFAULT) {
        return _flags(true, name, value, text, label, color);
    }
    bool Flags(CSREF name, GHflags* value = nullptr, CSREF text = "", CSREF label = "", uint32_t color = GH_DEFAULT) {
        return _flags(false, name.c_str(), value, text.c_str(), label.c_str(), color);
    }

    bool _flags(bool fstr, VSPTR name, GHflags* value, VSPTR text, VSPTR label, uint32_t color) {
        if (_isUI()) {
            _begin(F("flags"));
            _name(name, fstr);
            _value();
            GHtypeToStr(sptr, value, GH_FLAGS);
            _text(text, fstr);
            _label(label, fstr);
            _color(color);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) GHtypeToStr(sptr, value, GH_FLAGS);
        } else if (bptr->type == GH_BUILD_ACTION) {
            return bptr->parseSet(name, value, GH_FLAGS, fstr);
        }
        return 0;
    }

    // ========================== COLOR ==========================
    bool Color(FSTR name, GHcolor* value = nullptr, FSTR label = nullptr) {
        return _color(true, name, value, label);
    }
    bool Color(CSREF name, GHcolor* value = nullptr, CSREF label = "") {
        return _color(false, name.c_str(), value, label.c_str());
    }

    bool _color(bool fstr, VSPTR name, GHcolor* value, VSPTR label) {
        if (_isUI()) {
            _begin(F("color"));
            _name(name, fstr);
            _value();
            GHtypeToStr(sptr, value, GH_COLOR);
            _label(label, fstr);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) GHtypeToStr(sptr, value, GH_COLOR);
        } else if (bptr->type == GH_BUILD_ACTION) {
            return bptr->parseSet(name, value, GH_COLOR, fstr);
        }
        return 0;
    }

    // ========================== LED ==========================
    void LED(FSTR name, bool value = 0, FSTR label = nullptr, FSTR icon = nullptr) {
        _led(true, name, value, label, icon);
    }
    void LED(CSREF name, bool value = 0, CSREF label = "", CSREF icon = "") {
        _led(false, name.c_str(), value, label.c_str(), icon.c_str());
    }

    void _led(bool fstr, VSPTR name, bool value, VSPTR label, VSPTR text) {
        if (_isUI()) {
            _begin(F("led"));
            _name(name, fstr);
            _value();
            *sptr += value;
            _label(label, fstr);
            _text(text, fstr);
            _tabw();
            _end();
        } else if (_isRead()) {
            if (_checkName(name, fstr)) *sptr += value;
        }
    }

    // ========================== SPACE ==========================
    void Space(int height = 0) {
        if (_isUI()) {
            _begin(F("spacer"));
            _add(F(",'height':"));
            *sptr += height;
            _tabw();
            _end();
        }
    }

    // ========================== MENU ==========================
    bool Menu(FSTR text) {
        return _tabs(true, F("menu"), F("_menu"), &menu, text, nullptr);
    }
    bool Menu(CSREF text) {
        return _tabs(false, F("menu"), "_menu", &menu, text.c_str(), nullptr);
    }

    // ========================== TABS ==========================
    bool Tabs(FSTR name, uint8_t* value, FSTR text, FSTR label = nullptr) {
        return _tabs(true, F("tabs"), name, value, text, label);
    }
    bool Tabs(CSREF name, uint8_t* value, CSREF text, CSREF label = "") {
        return _tabs(false, F("tabs"), name.c_str(), value, text.c_str(), label.c_str());
    }

    bool _tabs(bool fstr, FSTR tag, VSPTR name, uint8_t* value, VSPTR text, VSPTR label) {
        if (_isUI()) {
            _begin(tag);
            _name(name, fstr);
            _value();
            *sptr += *value;
            _text(text, fstr);
            _label(label, fstr);
            _tabw();
            _end();
        } else if (bptr->type == GH_BUILD_ACTION) {
            bool act = bptr->parseSet(name, value, GH_UINT8, fstr);
            if (act) refresh();
            return act;
        }
        return 0;
    }

    // ========================= CANVAS =========================
    bool Canvas(FSTR name, int width = 400, int height = 300, GHcanvas* cv = nullptr, GHpos* pos = nullptr, FSTR label = nullptr) {
        return _canvas(true, name, width, height, cv, label, pos, false);
    }
    bool Canvas(CSREF name, int width = 400, int height = 300, GHcanvas* cv = nullptr, GHpos* pos = nullptr, CSREF label = "") {
        return _canvas(false, name.c_str(), width, height, cv, label.c_str(), pos, false);
    }
    bool BeginCanvas(FSTR name, int width = 400, int height = 300, GHcanvas* cv = nullptr, GHpos* pos = nullptr, FSTR label = nullptr) {
        return _canvas(true, name, width, height, cv, label, pos, true);
    }
    bool BeginCanvas(CSREF name, int width = 400, int height = 300, GHcanvas* cv = nullptr, GHpos* pos = nullptr, CSREF label = "") {
        return _canvas(false, name.c_str(), width, height, cv, label.c_str(), pos, true);
    }

    bool _canvas(bool fstr, VSPTR name, int width, int height, GHcanvas* cv, VSPTR label, GHpos* pos, bool begin) {
        if (!_isUI() && cv) cv->extBuffer(nullptr);

        if (_isUI()) {
            _begin(F("canvas"));
            _name(name, fstr);
            _add(F(",'width':"));
            *sptr += width;
            _add(F(",'height':"));
            *sptr += height;
            _label(label, fstr);
            if (pos) _add(F(",'active':1"));
            _value();
            *sptr += '[';
            if (begin && cv) cv->extBuffer(sptr);
            else EndCanvas();
        } else if (bptr->type == GH_BUILD_ACTION) {
            return bptr->parseSet(name, pos, GH_POS, fstr);
        }
        return 0;
    }

    void EndCanvas() {
        if (_isUI()) {
            *sptr += ']';
            _tabw();
            _end();
        }
    }

    // ========================= IMAGE =========================
    void Image(FSTR url, int prd = 0, FSTR label = nullptr) {
        _image(true, url, prd, label);
    }
    void Image(CSREF url, int prd = 0, CSREF label = "") {
        _image(false, url.c_str(), prd, label.c_str());
    }
    void _image(bool fstr, VSPTR url, int prd, VSPTR label) {
        if (_isUI()) {
            _begin(F("image"));
            _value(url, fstr);
            _label(label, fstr);
            _add(F(",'prd':"));
            *sptr += prd;
            _tabw();
            _end();
        }
    }

    // ========================= STREAM =========================
    void Stream() {
        if (_isUI()) {
            _begin(F("stream"));
            _tabw();
            _end();
        }
    }

    // =========================== JOY ===========================
    bool Joystick(FSTR name, GHpos* pos, bool autoc = 1, bool exp = 0, FSTR label = nullptr, uint32_t color = GH_DEFAULT) {
        return _joy(true, name, pos, autoc, exp, label, color);
    }
    bool Joystick(CSREF name, GHpos* pos, bool autoc = 1, bool exp = 0, CSREF label = "", uint32_t color = GH_DEFAULT) {
        return _joy(false, name.c_str(), pos, autoc, exp, label.c_str(), color);
    }

    bool _joy(bool fstr, VSPTR name, GHpos* pos, bool autoc, bool exp, VSPTR label, uint32_t color) {
        if (_isUI()) {
            _begin(F("joy"));
            _name(name, fstr);
            _add(F(",'auto':"));
            *sptr += autoc;
            _add(F(",'exp':"));
            *sptr += exp;
            _label(label, fstr);
            _color(color);
            _tabw();
            _end();
        } else if (bptr->type == GH_BUILD_ACTION) {
            bool act = bptr->parseSet(name, pos, GH_POS, fstr);
            if (act) {
                pos->x -= 255;
                pos->y -= 255;
            }
            return act;
        }
        return 0;
    }

    // ======================= CONFIRM ========================
    bool Confirm(FSTR name, bool* value = nullptr, FSTR label = nullptr) {
        return _confirm(true, name, value, label);
    }
    bool Confirm(CSREF name, bool* value = nullptr, CSREF label = "") {
        return _confirm(false, name.c_str(), value, label.c_str());
    }

    bool _confirm(bool fstr, VSPTR name, bool* value, VSPTR label) {
        if (_isUI()) {
            _begin(F("confirm"));
            _name(name, fstr);
            _label(label, fstr);
            _end();
        } else if (bptr->type == GH_BUILD_ACTION) {
            return bptr->parseSet(name, value, GH_BOOL, fstr);
        }
        return 0;
    }

    // ========================= PROMPT ========================
    bool Prompt(FSTR name, void* value = nullptr, GHdata_t type = GH_NULL, FSTR label = nullptr) {
        return _prompt(true, name, value, type, label);
    }
    bool Prompt(CSREF name, void* value = nullptr, GHdata_t type = GH_NULL, CSREF label = "") {
        return _prompt(false, name.c_str(), value, type, label.c_str());
    }

    bool _prompt(bool fstr, VSPTR name, void* value, GHdata_t type, VSPTR label) {
        if (_isUI()) {
            _begin(F("prompt"));
            _name(name, fstr);
            _value();
            _quot();
            GHtypeToStr(sptr, value, type);
            _quot();
            _label(label, fstr);
            _end();
        } else if (bptr->type == GH_BUILD_ACTION) {
            return bptr->parseSet(name, value, type, fstr);
        }
        return 0;
    }

    uint8_t menu = 0;

    // ======================== PROTECTED ========================
   protected:
    String* sptr = nullptr;
    GHbuild* bptr = nullptr;
    virtual void _afterComponent() = 0;
    virtual void refresh() = 0;
    int tab_width = 0;

    // ========================= PRIVATE =========================
   private:
    bool _checkName(VSPTR name, bool fstr = true) {
        if (fstr ? (!strcmp_P(bptr->action.name, (PGM_P)name)) : (!strcmp(bptr->action.name, (PGM_P)name))) {
            bptr->type = GH_BUILD_NONE;
            return true;
        }
        return false;
    }
    bool _isUI() {
        return (bptr && sptr && (bptr->type == GH_BUILD_UI || bptr->type == GH_BUILD_COUNT));
    }
    bool _isRead() {
        return (bptr && sptr && bptr->type == GH_BUILD_READ);
    }

    // ================
    void _add(VSPTR str, bool fstr = true) {
        if (str) {
            if (fstr) *sptr += (FSTR)str;
            else *sptr += (PGM_P)str;
        }
    }
    void _begin(FSTR type) {
        _add(F("{'type':'"));
        *sptr += type;
        _quot();
    }
    void _end() {
        *sptr += '}';
        _afterComponent();
        *sptr += ',';
    }
    void _quot() {
        *sptr += '\'';
    }
    void _tabw() {
        if (tab_width) {
            _add(F(",'tab_w':"));
            *sptr += tab_width;
        }
    }

    // ================
    void _value() {
        *sptr += F(",'value':");
    }
    void _value(VSPTR value, bool fstr = true) {
        _value();
        _quot();
        _add(value, fstr);
        _quot();
    }
    void _name(VSPTR name, bool fstr = true) {
        _add(F(",'name':'"));
        _add(name, fstr);
        _quot();
    }
    void _label(VSPTR label, bool fstr = true) {
        _add(F(",'label':'"));
        _add(label, fstr);
        _quot();
    }
    void _text() {
        _add(F(",'text':"));
    }
    void _text(VSPTR text, bool fstr = true) {
        _text();
        _quot();
        _add(text, fstr);
        _quot();
    }

    // ================
    void _color(uint32_t& col) {
        if (col == GH_DEFAULT) return;
        _add(F(",'color':"));
        *sptr += col;
    }
    void _size(int& val) {
        _add(F(",'size':"));
        *sptr += val;
    }

    // ================
    void _minv(float val) {
        _add(F(",'min':"));
        *sptr += val;
    }

    void _maxv(float val) {
        _add(F(",'max':"));
        *sptr += val;
    }

    void _step(float val) {
        _add(F(",'step':"));
        if (val < 0.01) *sptr += String(val, 4);
        else *sptr += val;
    }
};