#include <cstring>

#include <ios>
#include <locale>
#include <tuple>
#include <unordered_set>

#include <osmium/handler.hpp>
#include <osmium/osm/types.hpp>

#include <boost/functional/hash.hpp>

//
//  Parse and accumulate all Turn Restrictions
//

struct Restriction {
  using RelationId = osmium::unsigned_object_id_type;
  using WayId = osmium::unsigned_object_id_type;
  using NodeId = osmium::unsigned_object_id_type;

  RelationId id;

  WayId from;
  NodeId via;
  WayId to;

  // Hashable
  friend std::size_t hash_value(const Restriction &restriction) {
    std::size_t seed{0};
    boost::hash_combine(seed, restriction.id);
    boost::hash_combine(seed, restriction.from);
    boost::hash_combine(seed, restriction.via);
    boost::hash_combine(seed, restriction.to);
    return seed;
  }

  // Comparable
  friend bool operator==(const Restriction &lhs, const Restriction &rhs) {
    return std::tie(lhs.from, lhs.via, lhs.to) == std::tie(rhs.from, rhs.via, rhs.to);
  }
};

struct RestrictionHandler final : osmium::handler::Handler {
  std::unordered_set<Restriction, boost::hash<Restriction>> restrictions;

  void relation(const osmium::Relation &rel) {
    // We only care for turn restrictions
    const auto *restriction = rel.get_value_by_key("restriction");
    if (!restriction)
      return;

    // We care for no_* restrictions only, not inverted only_* restrictions
    if (std::strncmp("no_", restriction, 3) != 0)
      return;

    // We only care for car-ish turn restrictions
    const constexpr char *exceptions[] = {"motorcar",      //
                                          "motor_vehicle", //
                                          "vehicle"};      //

    const auto *except = rel.get_value_by_key("except");

    if (except)
      for (const auto *exception : exceptions)
        if (std::strcmp(exception, except) == 0)
          return;

    // Parse from, via, to ids into out
    Restriction out;
    out.id = rel.positive_id();

    enum class DoneWith : unsigned {
      None = 0,
      From = 1 << 0,
      Via = 1 << 1,
      To = 1 << 2,
      All = From | Via | To
    } done = DoneWith::None;

    // Parse the no_* turn restriction:  from (way), via (node), to (way)
    for (const auto &member : rel.members()) {
      const auto *role = member.role();

      const auto isFrom = std::strcmp("from", role) == 0;
      const auto isVia = std::strcmp("via", role) == 0;
      const auto isTo = std::strcmp("to", role) == 0;

      if (!isFrom && !isVia && !isTo)
        return;

      const auto type = member.type();

      const auto isWay = type == osmium::item_type::way;
      const auto isNode = type == osmium::item_type::node;

      if ((isFrom || isTo) && !isWay)
        return;
      if (isVia && !isNode)
        return;

      // Here we know: role (type):  from (way), via (node), to (way)
      if (isFrom) {
        out.from = member.positive_ref();
        done = DoneWith((unsigned)done | (unsigned)DoneWith::From);
      }

      if (isVia) {
        out.via = member.positive_ref();
        done = DoneWith((unsigned)done | (unsigned)DoneWith::Via);
      }

      if (isTo) {
        out.to = member.positive_ref();
        done = DoneWith((unsigned)done | (unsigned)DoneWith::To);
      }
    }

    if (done != DoneWith::All)
      return;

    restrictions.insert(std::move(out));
  }
};
