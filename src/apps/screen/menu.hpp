#pragma once
#include <stdint.h>

namespace apps::screen {
enum class Event { None, Up, Down, Select, Back };
struct Button {
    bool raw = false, pressed = false, longSent = false;
    uint32_t changed = 0, since = 0, repeated = 0;
    Event sample(bool value, uint32_t now, Event action) {
        if (raw != value) { raw = value; changed = now; }
        if (pressed != raw && uint32_t(now - changed) >= 30) {
            pressed = raw;
            if (pressed) {
                since = repeated = now; longSent = false;
                if (action != Event::Select) return action;
            } else if (action == Event::Select && !longSent) return action;
        }
        if (!pressed) return Event::None;
        const uint32_t held = now - since;
        if (action == Event::Select) {
            if (held >= 700 && !longSent) { longSent = true; return Event::Back; }
        } else if (held >= 400 && uint32_t(now - repeated) >= (held >= 2000 ? 75u : 150u)) {
            repeated = now; return action;
        }
        return Event::None;
    }
};
enum class Page { Home, Root, Telemetry, Settings, EditId, Saved, System, BootPage, Version, Reboot, Tests, Square, Dribbler, Speed, Kicker, Voltage };
enum class Effect { None, SaveId, Reboot, ToPlay, Kick, StartSquare };
struct Menu {
    Page page = Page::Home;
    int selected = 0, first = 0, telemetry = 0, home = 0;
    uint8_t editId = 0;
    uint8_t speed = 0, voltage = 0, draft = 0;
    bool spinning = false, chip = false;
    uint32_t savedAt = 0;
    bool locked() const { return page == Page::Settings || page == Page::EditId || page == Page::Saved || page == Page::Reboot || page == Page::Tests ||
        page == Page::Square || page == Page::Dribbler || page == Page::Speed || kickerPage(); }
    bool kickerPage() const { return page == Page::Kicker || page == Page::Voltage; }
    void go(Page next, int selection = 0) { page = next; selected = selection; first = 0; }
    void tick(uint32_t now) { if (page == Page::Saved && uint32_t(now - savedAt) >= 800) go(Page::Settings); }
    Effect event(Event e, uint8_t id, uint32_t now) {
        if (e == Event::None) return Effect::None;
        if (e == Event::Back) {
            switch (page) {
            case Page::Home: break;
            case Page::Root: go(Page::Home); break;
            case Page::Telemetry: go(Page::Root, 1); break;
            case Page::Settings: go(Page::Root, 2); break;
            case Page::EditId: case Page::Saved: go(Page::Settings); break;
            case Page::System: go(Page::Root, 3); first = 1; break;
            case Page::BootPage: go(Page::System, 2); break;
            case Page::Version: case Page::Reboot: go(Page::System); break;
            case Page::Tests: go(Page::Root, 4); first = 2; break;
            case Page::Square: go(Page::Tests, 2); break;
            case Page::Dribbler: spinning = false; go(Page::Tests); break;
            case Page::Speed: go(Page::Dribbler); break;
            case Page::Kicker: voltage = 0; go(Page::Tests, 1); break;
            case Page::Voltage: go(Page::Kicker); break;
            }
            return Effect::None;
        }
        if (e == Event::Up || e == Event::Down) {
            int delta = e == Event::Up ? -1 : 1;
            if (page == Page::Home) home = 1 - home;
            else if (page == Page::BootPage) selected = (selected + delta + 5) % 5;
            else if (page == Page::Telemetry) telemetry = (telemetry + delta + 3) % 3;
            else if (page == Page::EditId) editId = (editId + (e == Event::Up ? 1 : 15)) % 16;
            else if (page == Page::Speed || page == Page::Voltage)
                draft = (draft + (e == Event::Up ? 1 : 15)) % 16;
            else {
                int count = 1;
                switch (page) {
                case Page::Root: count = 5; break;
                case Page::System: case Page::Kicker: case Page::Tests: count = 3; break;
                case Page::Dribbler:
                case Page::Reboot: count = 2; break;
                default: break;
                }
                selected += delta;
                if (selected < 0) selected = 0;
                if (selected >= count) selected = count - 1;
                if (selected < first) first = selected;
                if (selected >= first + 3) first = selected - 2;
            }
        } else if (e == Event::Select) {
            switch (page) {
            case Page::Home: go(Page::Root); break;
            case Page::Root:
                if (selected == 0) {
                    go(Page::Home);
                    home = 0;
                    return Effect::ToPlay;
                }
                go(selected == 1 ? Page::Telemetry : selected == 2 ? Page::Settings : selected == 3 ? Page::System : Page::Tests); break;
            case Page::Settings: editId = id; go(Page::EditId); break;
            case Page::EditId:
                if (editId == id) go(Page::Settings);
                else { go(Page::Saved); savedAt = now; return Effect::SaveId; }
                break;
            case Page::System: go(selected == 0 ? Page::Version : selected == 1 ? Page::Reboot : Page::BootPage); break;
            case Page::Reboot:
                if (selected == 1) return Effect::Reboot;
                go(Page::System, 1); break;
            case Page::Tests:
                go(selected == 0 ? Page::Dribbler : selected == 1 ? Page::Kicker : Page::Square);
                break;
            case Page::Square: return Effect::StartSquare;
            case Page::Dribbler:
                if (selected == 0) { draft = speed; go(Page::Speed); }
                else spinning = !spinning;
                break;
            case Page::Speed: speed = draft; go(Page::Dribbler); break;
            case Page::Kicker:
                if (selected == 0) { draft = voltage; go(Page::Voltage); }
                else { chip = selected == 1; return Effect::Kick; }
                break;
            case Page::Voltage: voltage = draft; go(Page::Kicker); break;
            default: break;
            }
        }
        return Effect::None;
    }
};
}
