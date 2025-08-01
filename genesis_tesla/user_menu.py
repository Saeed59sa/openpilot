#!/usr/bin/env python3
"""Simple text UI showing data from openpilot."""

import curses
import time

from .openpilot_interface import OpenpilotInterface

UPDATE_DT = 0.05  # 20 Hz


def draw(stdscr, iface: OpenpilotInterface) -> None:
    curses.curs_set(0)
    stdscr.nodelay(True)

    while True:
        data = iface.update()
        stdscr.erase()
        stdscr.addstr(0, 0, f"Set speed: {data.set_speed:.1f}")
        stdscr.addstr(1, 0, f"Lanes: L={data.left_lane_visible} R={data.right_lane_visible}")
        stdscr.addstr(2, 0, f"Lead distance bars: {data.lead_distance_bars}")
        stdscr.addstr(3, 0, f"Visual alert: {data.visual_alert}")
        stdscr.addstr(4, 0, f"Audible alert: {data.audible_alert}")
        stdscr.refresh()
        time.sleep(UPDATE_DT)


def main() -> None:
    iface = OpenpilotInterface()
    curses.wrapper(draw, iface)


if __name__ == "__main__":
    main()
