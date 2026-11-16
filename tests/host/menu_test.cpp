#include "apps/screen/menu.hpp"
#include <cassert>
#include <cstdio>
using namespace apps::screen;
int main() {
    Button b;
    assert(b.sample(true, 0, Event::Select) == Event::None);
    assert(b.sample(false, 10, Event::Select) == Event::None);
    assert(b.sample(true, 20, Event::Select) == Event::None);
    assert(b.sample(true, 49, Event::Select) == Event::None);
    assert(b.sample(true, 50, Event::Select) == Event::None);
    b.sample(false, 100, Event::Select);
    assert(b.sample(false, 130, Event::Select) == Event::Select);
    b.sample(true, 200, Event::Select); b.sample(true, 230, Event::Select);
    assert(b.sample(true, 929, Event::Select) == Event::None);
    assert(b.sample(true, 930, Event::Select) == Event::Back);
    assert(b.sample(true, 1000, Event::Select) == Event::None);
    b.sample(false, 1100, Event::Select);
    assert(b.sample(false, 1130, Event::Select) == Event::None);
    b = {};
    b.sample(true, 0, Event::Up);
    assert(b.sample(true, 30, Event::Up) == Event::Up);
    assert(b.sample(true, 429, Event::Up) == Event::None);
    assert(b.sample(true, 430, Event::Up) == Event::Up);
    assert(b.sample(true, 579, Event::Up) == Event::None);
    assert(b.sample(true, 580, Event::Up) == Event::Up);
    assert(b.sample(true, 2030, Event::Up) == Event::Up);
    assert(b.sample(true, 2104, Event::Up) == Event::None);
    assert(b.sample(true, 2105, Event::Up) == Event::Up);
    b = {};
    b.sample(true, UINT32_MAX - 20, Event::Down);
    assert(b.sample(true, 10, Event::Down) == Event::Down);

    Menu m;
    m.event(Event::Select, 7, 0); // root
    m.event(Event::Down, 7, 0);
    m.event(Event::Select, 7, 0); // telemetry
    assert(!m.locked() && m.page == Page::Telemetry);
    m.event(Event::Up, 7, 0); assert(m.telemetry == 2);
    m.event(Event::Back, 7, 0);
    m.event(Event::Down, 7, 0); m.event(Event::Select, 7, 0);
    assert(m.locked() && m.page == Page::Settings);
    m.event(Event::Select, 15, 0);
    m.event(Event::Up, 15, 0); assert(m.editId == 0);
    m.event(Event::Down, 15, 0); assert(m.editId == 15);
    assert(m.event(Event::Select, 15, 0) == Effect::None); // no write
    m.event(Event::Select, 15, 0); m.event(Event::Up, 15, 0);
    m.event(Event::Back, 15, 0); assert(m.page == Page::Settings);
    m.event(Event::Select, 15, 0); assert(m.editId == 15); // discard edit
    m.event(Event::Up, 15, 0);
    assert(m.event(Event::Select, 15, 100) == Effect::SaveId);
    assert(m.event(Event::Select, 0, 101) == Effect::None); // no duplicate write
    m.tick(899); assert(m.page == Page::Saved);
    m.tick(900); assert(m.page == Page::Settings);
    m.event(Event::Back, 0, 900); assert(!m.locked());
    m.event(Event::Down, 0, 0); assert(m.selected == 3);
    m.event(Event::Select, 0, 0); m.event(Event::Down, 0, 0); m.event(Event::Select, 0, 0);
    assert(m.page == Page::Reboot && m.selected == 0 && m.locked());
    assert(m.event(Event::Select, 0, 0) == Effect::None && !m.locked());
    m.event(Event::Select, 0, 0); m.event(Event::Down, 0, 0);
    assert(m.event(Event::Select, 0, 0) == Effect::Reboot);
    std::puts("menu/button tests passed");
}
