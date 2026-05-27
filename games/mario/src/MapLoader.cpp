#include "MapLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

static constexpr uint32_t kFlipMask = 0xE0000000u;

// ---- string helpers --------------------------------------------------------

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return {};
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static std::string dirOf(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    return (slash == std::string::npos) ? "" : path.substr(0, slash + 1);
}

static int attrInt(const std::string& xml, const std::string& attr, size_t searchFrom = 0) {
    size_t pos = xml.find(attr + "=\"", searchFrom);
    if (pos == std::string::npos) throw std::runtime_error("Missing attribute: " + attr);
    pos += attr.size() + 2;
    return std::stoi(xml.substr(pos, xml.find('"', pos) - pos));
}

static std::string attrStr(const std::string& xml, const std::string& attr, size_t searchFrom = 0) {
    size_t pos = xml.find(attr + "=\"", searchFrom);
    if (pos == std::string::npos) return {};
    pos += attr.size() + 2;
    return xml.substr(pos, xml.find('"', pos) - pos);
}

// ---- layer CSV parser ------------------------------------------------------

static TileLayer parseLayer(const std::string& xml, size_t layerTagPos) {
    TileLayer layer;
    layer.name = attrStr(xml, "name", layerTagPos);
    layer.cols = attrInt(xml, "width",  layerTagPos);
    layer.rows = attrInt(xml, "height", layerTagPos);

    size_t dataTag  = xml.find("<data", layerTagPos);
    size_t csvStart = xml.find('>', dataTag) + 1;
    size_t csvEnd   = xml.find("</data>", csvStart);

    std::istringstream stream(xml.substr(csvStart, csvEnd - csvStart));
    std::string token;
    while (std::getline(stream, token, ',')) {
        token = trim(token);
        if (!token.empty()) {
            uint32_t raw = static_cast<uint32_t>(std::stoul(token));
            layer.gids.push_back(raw & ~kFlipMask);
        }
    }
    return layer;
}

// ---- TSX column count ------------------------------------------------------

static int parseTsxColumns(const std::string& tsxPath) {
    std::string xml = readFile(tsxPath);
    return attrInt(xml, "columns");
}

// ---- public API ------------------------------------------------------------

const TileLayer* MarioMap::findLayer(const std::string& name) const {
    for (const auto& l : layers)
        if (l.name == name) return &l;
    return nullptr;
}

const TilesetRef* MarioMap::tilesetForGid(uint32_t gid) const {
    if (gid == 0) return nullptr;
    // Walk backwards to find the tileset with the largest firstGid <= gid
    const TilesetRef* best = nullptr;
    for (const auto& ts : tilesets) {
        if (static_cast<uint32_t>(ts.firstGid) <= gid) {
            if (!best || ts.firstGid > best->firstGid)
                best = &ts;
        }
    }
    return best;
}

// Parse "#rrggbb" or "#aarrggbb" into a Color4 (normalized 0..1).
// Returns black+opaque if the string is missing or malformed.
static Color4 parseHexColor(const std::string& hex) {
    if (hex.empty() || hex[0] != '#') return {};
    std::string h = hex.substr(1);
    if (h.size() == 6) h = "ff" + h; // no alpha → fully opaque
    if (h.size() != 8) return {};
    auto byte = [&](size_t pos) {
        return std::stoul(h.substr(pos, 2), nullptr, 16) / 255.f;
    };
    return { byte(2), byte(4), byte(6), byte(0) }; // AARRGGBB → rgba
}

MarioMap loadMap(const std::string& tmxPath) {
    std::string xml  = readFile(tmxPath);
    std::string dir  = dirOf(tmxPath);

    MarioMap map;
    map.cols            = attrInt(xml, "width");
    map.rows            = attrInt(xml, "height");
    map.tileWidth       = attrInt(xml, "tilewidth");
    map.tileHeight      = attrInt(xml, "tileheight");
    map.backgroundColor = parseHexColor(attrStr(xml, "backgroundcolor"));

    // Parse tileset references
    size_t pos = 0;
    while ((pos = xml.find("<tileset", pos)) != std::string::npos) {
        TilesetRef ts;
        ts.firstGid = attrInt(xml, "firstgid", pos);
        ts.source   = attrStr(xml, "source", pos);
        ts.columns  = parseTsxColumns(dir + ts.source);
        map.tilesets.push_back(ts);
        pos += 8; // skip past "<tileset"
    }

    // Parse tile layers
    pos = 0;
    while ((pos = xml.find("<layer", pos)) != std::string::npos) {
        map.layers.push_back(parseLayer(xml, pos));
        pos += 6;
    }

    return map;
}
