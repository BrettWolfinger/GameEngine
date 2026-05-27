#include "Tilemap.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace Engine::Tilemap {

// ---- string helpers --------------------------------------------------------

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Engine::Tilemap: cannot open file: " + path);
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

static int attrInt(const std::string& xml, const std::string& attr, size_t from = 0) {
    size_t pos = xml.find(attr + "=\"", from);
    if (pos == std::string::npos) throw std::runtime_error("Missing TMX attribute: " + attr);
    pos += attr.size() + 2;
    return std::stoi(xml.substr(pos, xml.find('"', pos) - pos));
}

static std::string attrStr(const std::string& xml, const std::string& attr, size_t from = 0) {
    size_t pos = xml.find(attr + "=\"", from);
    if (pos == std::string::npos) return {};
    pos += attr.size() + 2;
    return xml.substr(pos, xml.find('"', pos) - pos);
}

// ---- hex color parser ------------------------------------------------------

static Color4 parseHexColor(const std::string& hex) {
    if (hex.empty() || hex[0] != '#') return {};
    std::string h = hex.substr(1);
    if (h.size() == 6) h = "ff" + h; // no alpha → fully opaque
    if (h.size() != 8) return {};
    auto byte = [&](size_t pos) {
        return static_cast<float>(std::stoul(h.substr(pos, 2), nullptr, 16)) / 255.f;
    };
    return { byte(2), byte(4), byte(6), byte(0) }; // AARRGGBB → rgba
}

// ---- TSX parser ------------------------------------------------------------

static void parseTsx(const std::string& tsxPath, TilesetRef& ref) {
    std::string xml = readFile(tsxPath);
    ref.columns     = attrInt(xml, "columns");
    ref.tileCount   = attrInt(xml, "tilecount");
    std::string imgSrc = attrStr(xml, "source", xml.find("<image"));
    ref.imagePath   = dirOf(tsxPath) + imgSrc;
}

// ---- tile layer parser -----------------------------------------------------

static TileLayer parseLayer(const std::string& xml, size_t tagPos) {
    TileLayer layer;
    layer.name = attrStr(xml, "name",   tagPos);
    layer.cols = attrInt(xml,  "width",  tagPos);
    layer.rows = attrInt(xml,  "height", tagPos);

    size_t dataTag  = xml.find("<data",  tagPos);
    size_t csvStart = xml.find('>',      dataTag) + 1;
    size_t csvEnd   = xml.find("</data>",csvStart);

    std::istringstream stream(xml.substr(csvStart, csvEnd - csvStart));
    std::string token;
    while (std::getline(stream, token, ',')) {
        token = trim(token);
        if (!token.empty())
            layer.gids.push_back(static_cast<uint32_t>(std::stoul(token)));
    }
    return layer;
}

// ---- Map methods -----------------------------------------------------------

const TileLayer* Map::findLayer(const std::string& name) const {
    for (const auto& l : layers)
        if (l.name == name) return &l;
    return nullptr;
}

const TilesetRef* Map::tilesetForGid(uint32_t gid) const {
    gid = stripFlips(gid);
    if (gid == 0) return nullptr;
    const TilesetRef* best = nullptr;
    for (const auto& ts : tilesets) {
        if (static_cast<uint32_t>(ts.firstGid) <= gid)
            if (!best || ts.firstGid > best->firstGid)
                best = &ts;
    }
    return best;
}

// ---- applyFlips ------------------------------------------------------------

FlippedUV applyFlips(UVRect uv, uint32_t rawGid) {
    static constexpr float kHalfPi = 1.5707963268f;
    float ru0 = uv.u0, rv0 = uv.v0, ru1 = uv.u1, rv1 = uv.v1;
    float angle = 0.f;

    bool fH = hasFlipH(rawGid);
    bool fV = hasFlipV(rawGid);
    bool fD = hasFlipD(rawGid);

    if (!fD) {
        if (fH) std::swap(ru0, ru1);
        if (fV) std::swap(rv0, rv1);
    } else if (fH && !fV) {
        angle = +kHalfPi;                       // 90° CW
    } else if (!fH && fV) {
        angle = -kHalfPi;                       // 90° CCW
    } else if (!fH && !fV) {
        angle = -kHalfPi; std::swap(ru0, ru1);  // main diagonal
    } else {
        angle = +kHalfPi; std::swap(ru0, ru1);  // anti-diagonal
    }

    return { { ru0, rv0, ru1, rv1 }, angle };
}

// ---- loadMap ---------------------------------------------------------------

Map loadMap(const std::string& tmxPath) {
    std::string xml = readFile(tmxPath);
    std::string dir = dirOf(tmxPath);

    Map map;
    map.cols            = attrInt(xml, "width");
    map.rows            = attrInt(xml, "height");
    map.tileWidth       = attrInt(xml, "tilewidth");
    map.tileHeight      = attrInt(xml, "tileheight");
    map.backgroundColor = parseHexColor(attrStr(xml, "backgroundcolor"));

    // Tileset references
    size_t pos = 0;
    while ((pos = xml.find("<tileset", pos)) != std::string::npos) {
        TilesetRef ts;
        ts.firstGid = attrInt(xml, "firstgid", pos);
        ts.source   = attrStr(xml, "source",   pos);
        parseTsx(dir + ts.source, ts);
        map.tilesets.push_back(ts);
        pos += 8;
    }

    // Tile layers
    pos = 0;
    while ((pos = xml.find("<layer", pos)) != std::string::npos) {
        map.layers.push_back(parseLayer(xml, pos));
        pos += 6;
    }

    return map;
}

} // namespace Engine::Tilemap
