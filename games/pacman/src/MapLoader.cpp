#include "MapLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

static constexpr uint32_t kFlipH    = 0x80000000u;
static constexpr uint32_t kFlipV    = 0x40000000u;
static constexpr uint32_t kFlipD    = 0x20000000u;
static constexpr uint32_t kFlipMask = kFlipH | kFlipV | kFlipD;

static constexpr int kItemsFirstGid = 248;
static constexpr int kDotLocalId    = 8;
static constexpr int kPelletLocalId = 9;

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open TMX: " + path);
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

static int findAttrInt(const std::string& xml, const std::string& attr) {
    size_t pos = xml.find(attr + "=\"");
    if (pos == std::string::npos) throw std::runtime_error("Missing TMX attribute: " + attr);
    pos += attr.size() + 2;
    size_t end = xml.find('"', pos);
    return std::stoi(xml.substr(pos, end - pos));
}

static std::vector<uint32_t> parseLayerData(const std::string& xml, const std::string& layerName) {
    std::string nameAttr = "name=\"" + layerName + "\"";
    size_t layerPos = xml.find(nameAttr);
    if (layerPos == std::string::npos)
        throw std::runtime_error("TMX layer not found: " + layerName);

    size_t dataTag = xml.find("<data", layerPos);
    if (dataTag == std::string::npos)
        throw std::runtime_error("No <data> in layer: " + layerName);

    size_t csvStart = xml.find('>', dataTag) + 1;
    size_t csvEnd   = xml.find("</data>", csvStart);
    if (csvEnd == std::string::npos)
        throw std::runtime_error("Unclosed <data> in layer: " + layerName);

    std::string csv = xml.substr(csvStart, csvEnd - csvStart);

    std::vector<uint32_t> result;
    std::istringstream stream(csv);
    std::string token;
    while (std::getline(stream, token, ',')) {
        token = trim(token);
        if (!token.empty())
            result.push_back(static_cast<uint32_t>(std::stoul(token)));
    }
    return result;
}

MapData loadTmx(const std::string& path) {
    std::string xml = readFile(path);

    MapData data;
    data.cols = findAttrInt(xml, "width");
    data.rows = findAttrInt(xml, "height");

    auto wallRaw = parseLayerData(xml, "Wall");
    data.walls.resize(wallRaw.size());
    for (size_t i = 0; i < wallRaw.size(); ++i) {
        uint32_t raw      = wallRaw[i];
        data.walls[i].gid   = raw & ~kFlipMask;
        data.walls[i].flipH = (raw & kFlipH) != 0;
        data.walls[i].flipV = (raw & kFlipV) != 0;
        data.walls[i].flipD = (raw & kFlipD) != 0;
    }

    auto dotsRaw = parseLayerData(xml, "Dots");
    data.dots.resize(dotsRaw.size());
    for (size_t i = 0; i < dotsRaw.size(); ++i) {
        uint32_t gid = dotsRaw[i] & ~kFlipMask;
        int localId  = static_cast<int>(gid) - kItemsFirstGid;
        if (localId == kDotLocalId)        data.dots[i] = CellType::Dot;
        else if (localId == kPelletLocalId) data.dots[i] = CellType::PowerPellet;
        else                               data.dots[i] = CellType::Empty;
    }

    return data;
}
