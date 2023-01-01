#include "df/graphic_viewportst.h"

#include "modules/Maps.h"
#include "modules/Screen.h"

#include "Debug.h"
#include "LuaTools.h"
#include "PluginManager.h"

using namespace DFHack;

DFHACK_PLUGIN("pathable");

REQUIRE_GLOBAL(gps);
REQUIRE_GLOBAL(window_x);
REQUIRE_GLOBAL(window_y);
REQUIRE_GLOBAL(window_z);
REQUIRE_GLOBAL(world);

namespace DFHack {
    DBG_DECLARE(pathable, log, DebugCategory::LINFO);
}

DFhackCExport command_result plugin_init(color_ostream &out, std::vector<PluginCommand> &commands) {
    return CR_OK;
}

DFhackCExport command_result plugin_shutdown(color_ostream &out) {
    return CR_OK;
}

static void paintScreen(df::coord target, bool skip_unrevealed = false) {
    DEBUG(log).print("entering paintScreen\n");

    bool use_graphics = Screen::inGraphicsMode();

    auto dimx = use_graphics ? gps->main_viewport->dim_x : gps->dimx;
    auto dimy = use_graphics ? gps->main_viewport->dim_y : gps->dimy;
    for (int y = 0; y < dimy; ++y) {
        for (int x = 0; x < dimx; ++x) {
            df::coord map_pos(*window_x + x, *window_y + y, *window_z);

            if (!Maps::isValidTilePos(map_pos))
                continue;

            // don't overwrite the target tile
            if (!use_graphics && map_pos == target) {
                TRACE(log).print("skipping target tile\n");
                continue;
            }

            if (skip_unrevealed && !Maps::isTileVisible(map_pos)) {
                TRACE(log).print("skipping hidden tile\n");
                continue;
            }

            DEBUG(log).print("scanning map tile at offset %d, %d\n", x, y);
            Screen::Pen cur_tile = Screen::readTile(x, y, true);
            DEBUG(log).print("tile data: ch=%d, fg=%d, bg=%d, bold=%s\n",
                    cur_tile.ch, cur_tile.fg, cur_tile.bg, cur_tile.bold ? "true" : "false");
            DEBUG(log).print("tile data: tile=%d, tile_mode=%d, tile_fg=%d, tile_bg=%d\n",
                    cur_tile.tile, cur_tile.tile_mode, cur_tile.tile_fg, cur_tile.tile_bg);

            if (!cur_tile.valid()) {
                DEBUG(log).print("cannot read tile at offset %d, %d\n", x, y);
                continue;
            }

            bool can_walk = Maps::canWalkBetween(target, map_pos);
            DEBUG(log).print("tile is %swalkable at offset %d, %d\n",
                             can_walk ? "" : "not ", x, y);

            if (use_graphics) {
                if (map_pos == target) {
                    cur_tile.tile = 100711; // highlighted selection
                } else{
                    cur_tile.tile = can_walk ? 779 : 782;
                }
            } else {
                int color = can_walk ? COLOR_GREEN : COLOR_RED;
                if (cur_tile.fg && cur_tile.ch != ' ') {
                    cur_tile.fg = color;
                    cur_tile.bg = 0;
                } else {
                    cur_tile.fg = 0;
                    cur_tile.bg = color;
                }

                cur_tile.bold = false;

                if (cur_tile.tile)
                    cur_tile.tile_mode = Screen::Pen::CharColor;
            }

            Screen::paintTile(cur_tile, x, y, true);
        }
    }
}

DFHACK_PLUGIN_LUA_FUNCTIONS {
    DFHACK_LUA_FUNCTION(paintScreen),
    DFHACK_LUA_END
};
