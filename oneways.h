#include <cstring>

#include <tuple>
#include <unordered_set>

#include <osmium/handler.hpp>
#include <osmium/osm/types.hpp>

#include <boost/functional/hash.hpp>

//
// Parse and accumulate all oneways
//

struct Oneway {
  using NodeId = osmium::unsigned_object_id_type;
  using WayId = osmium::unsigned_object_id_type;

  WayId id;
  NodeId exit;

  // Hashable
  friend std::size_t hash_value(const Oneway &oneway) {
    std::size_t seed{0};
    boost::hash_combine(seed, oneway.id);
    boost::hash_combine(seed, oneway.exit);
    return seed;
  }

  // Comparable
  friend bool operator==(const Oneway &lhs, const Oneway &rhs) {
    return std::tie(lhs.id, lhs.exit) == std::tie(rhs.id, rhs.exit);
  }
};

struct OnewayHandler final : osmium::handler::Handler {
  std::unordered_set<Oneway, boost::hash<Oneway>> oneways;

  void way(const osmium::Way &way) {
    const auto *highway = way.get_value_by_key("highway");
    const auto *oneway = way.get_value_by_key("oneway");

    if (!highway || !oneway)
      return;

    // Oneway values
    const auto isOneway = std::strcmp("yes", oneway) == 0 ||  //
                          std::strcmp("1", oneway) == 0 ||    //
                          std::strcmp("true", oneway) == 0 || //
                          std::strcmp("-1", oneway) == 0;

    // Implied oneways
    const auto *junction = way.get_value_by_key("junction");
    const auto roundabout = junction && //
                            std::strcmp("roundabout", junction) == 0;

    const auto motorway = std::strcmp("motorway", highway) == 0 && //
                          std::strcmp("no", oneway) != 0;

    if (!isOneway && !roundabout && !motorway)
      return;

    // Opposite direction
    const auto reversed = std::strcmp("-1", oneway) == 0;

    Oneway out;
    out.id = way.positive_id();

    const auto &nodes = way.nodes();

    if (reversed)
      out.exit = nodes.front().positive_ref();
    else
      out.exit = nodes.back().positive_ref();

    oneways.insert(std::move(out));
  }
};
